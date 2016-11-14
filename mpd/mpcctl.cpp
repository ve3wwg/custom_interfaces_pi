//////////////////////////////////////////////////////////////////////
// mpcctl.cpp -- mpc control program
// Date: Mon Oct 10 12:20:11 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <vector>
#include <thread>
#include <sstream>
#include <mutex>

#include "flywheel.hpp"
#include "lcd1602.hpp"

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;
static uint8_t adc_i2c_addr = 0x48;		// I2C Addr for pcf8591 ADC
static uint8_t adc_i2c_ainx = 3;		// Default to AIN3

static volatile time_t changed = 0;		// Time of last display update
static volatile int current_pos = -1;		// Last song playing status

static std::mutex mutex;			// Thread lock
static volatile bool disp_changed = false;	// True if display changed
static std::string disp_title;			// Displayed title

//////////////////////////////////////////////////////////////////////
// Truncate artist - song to just song
// to fit 16x2 LCD
//////////////////////////////////////////////////////////////////////

static const char *
song_title(const char *info) {
	const char *cp = strchr(info,'-');	// Find hyphen
	
	if ( !cp )				// Not found?
		return info;			// If not, return as is

	for ( ++cp; *cp == ' '; ++cp )		// Skip blanks
		;
	return cp;				// 1st char of song
}

//////////////////////////////////////////////////////////////////////
// Read the volume control
//////////////////////////////////////////////////////////////////////

static int
read_volume() {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[2];
	uint8_t rxbuf[3];
	int rc;

	iomsgs[0].addr = iomsgs[1].addr = adc_i2c_addr;
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &adc_i2c_ainx;	// AIN3
	iomsgs[0].len = 1;

	iomsgs[1].flags = I2C_M_RD;	// Read
	iomsgs[1].buf = rxbuf;
	iomsgs[1].len = 3;		// 1st byte is last reading

	msgset.msgs = iomsgs;
	msgset.nmsgs = 2;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	if ( rc == -1 ) {
		fprintf(stderr,"%s: reading PCF8591 ADC at 0x%02X\n",
			strerror(errno),
			unsigned(iomsgs[1].addr));
		return 0;
	}

	return int(rxbuf[2]) * 100 / 255; // Return percentage
}

//////////////////////////////////////////////////////////////////////
// Queue up a message to be displayed
//////////////////////////////////////////////////////////////////////

static void
put_msg(const char *msg) {

	mutex.lock();				// Lock mutex
	if ( strcmp(msg,disp_title.c_str()) != 0 ) {
		disp_title = msg;		// Save new message
		disp_changed = true;		// Mark it as changed
	}
	changed = time(0);			// Hold off status update
	mutex.unlock();
}

//////////////////////////////////////////////////////////////////////
// Fetch status info from mpc command:
// ARGUMENTS:
//	pfile	Opened popen() for reading
// RETURNS:
//	title	Title playing
//	pos	1-based song 
//	of	# of titles/stations
//////////////////////////////////////////////////////////////////////

static void
fetch_status(FILE *pfile,std::string& title,int& pos,int& of ) {
	char buf[2048];

	if ( fgets(buf,sizeof buf,pfile) != nullptr ) {
		// Check if stopped:
		if ( !strncmp(buf,"volume:",7) ) {
			title = "Stopped";
			pclose(pfile);
			pos = -1;
			return;
		}

		// Chop off newline
		char *cp = strchr(buf,'\n');
		if ( cp != nullptr )
			*cp = 0;
		title = buf;	// Return title
	}

	// Next line has selection #
	if ( fgets(buf,sizeof buf,pfile) != nullptr ) {
		// [playing] #17/23   2:50/4:33 (62%)
		char *cp = strchr(buf,'#');
		pos = atoi(cp+1);	// 1-based
		if ( (cp = strchr(cp,'/')) != nullptr )
			of = atoi(cp+1); // # selections
		else	of = -1;

		if ( pos > 0 ) // Zero based in pgm
			--pos;
	}

	// Read remaining lines, if any
	while ( fgets(buf,sizeof buf,pfile) != nullptr )
		;
	pclose(pfile);		// Pipe close!

	current_pos = pos;	// Indicate current pos
}

//////////////////////////////////////////////////////////////////////
// Read MPC status
//////////////////////////////////////////////////////////////////////

static bool
get_mpc_status(std::string& title,int& pos,int& of) {
	FILE *pfile = popen("mpc status 2>/dev/null","r");

	if ( !pfile )
		return false;
	fetch_status(pfile,title,pos,of);
	return true;
}

//////////////////////////////////////////////////////////////////////
// Load playlist from MPC
//////////////////////////////////////////////////////////////////////

static bool
load(std::vector<std::string>& playlist) {
	FILE *pfile = popen("mpc playlist","r");
	char buf[2048], *cp;
	int pos = 0;
	
	if ( !pfile )
		return false;	// Unable to get playlist

	playlist.clear();
	
	while ( fgets(buf,sizeof buf,pfile) != nullptr ) {
		if ( (cp = strrchr(buf,'\n')) != nullptr )
			*cp = 0;

		std::stringstream ss;
		ss << ++pos << ": " << buf;
		playlist.push_back(ss.str());
	}

	pclose(pfile);
	return true;
}

//////////////////////////////////////////////////////////////////////
// Play title/station # pos
// Returns title, pos, and of
//////////////////////////////////////////////////////////////////////

static bool
mpc_play(std::string& title,int& pos,int& of) {
	std::stringstream cmd;

	if ( pos >= 0 )
		cmd << "mpc play " << (pos+1) << " 2>/dev/null";
	else	cmd << "mpc stop 2>/dev/null";

	FILE *pfile = popen(cmd.str().c_str(),"r");
	if ( pfile ) {
		fetch_status(pfile,title,pos,of);
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// Put message onto LCD's two 16 char lines
//////////////////////////////////////////////////////////////////////

static void
display(LCD1602& lcd,const char *msg) {
	char line1[17], line2[17];

	puts(msg);

	strncpy(line1,msg,16)[16] = 0;
	lcd.clear();
	lcd.set_cursor(false);
	lcd.putstr(line1);

	if ( strlen(msg) > 16 ) {
		strncpy(line2,msg+16,16)[16] = 0;
		lcd.moveto(2,0);
		lcd.putstr(line2);
	}
}

//////////////////////////////////////////////////////////////////////
// THREAD: Report on current MPC status
//////////////////////////////////////////////////////////////////////

static void
update_status() {
	time_t now;
	std::string title;
	int cpos = 0, epos = -1;

	for (;;) {
		now = time(nullptr);

		// While status info is stale
		while ( now - changed > 1 ) {
			changed = now;

			if ( get_mpc_status(title,cpos,epos) ) {
				if ( cpos >= 0 ) {
					std::stringstream ss;

					ss << (cpos+1) << ':' << song_title(title.c_str());
					put_msg(ss.str().c_str());
				} else	{
					put_msg(title.c_str());
				}
			}
		}
		usleep(10000);
	}
}

//////////////////////////////////////////////////////////////////////
// THREAD: Update the LCD whenever the display
// information changes.
//////////////////////////////////////////////////////////////////////

static void
lcd_thread(LCD1602 *plcd) {
	LCD1602& lcd = *plcd;		// Ref to LCD class
	std::string local_title;
	bool chgf;

	for (;;) {
		mutex.lock();
		if ( disp_changed ) {
			disp_changed = false;
			chgf = local_title != disp_title;
			local_title = disp_title;
		} else	{
			chgf = false;
		}
		mutex.unlock();		

		if ( chgf ) // Did info change?
			display(lcd,local_title.c_str());
		else	usleep(50000);
	}
}

//////////////////////////////////////////////////////////////////////
// THREAD: Monitor for rotations of the rotary control
//////////////////////////////////////////////////////////////////////

static void
rotary_control(GPIO *gpio) {
	Flywheel rsw(*gpio,20,21);	// Flywheeling control
	std::string title;		// Current playing title
	time_t last_update, last_sel;
	std::vector<std::string> playlist;
	int pos = 0, of = -1, last_play = -1;
	int rc;

	// Initialize:
	load(playlist);
	get_mpc_status(title,pos,of);
	if ( pos >= 0 )
		last_play = pos;
	last_update = last_sel = time(0);

	for (;;) {
		rc = rsw.read();	// Read rotary switch
		if ( rc != 0 ) {	// Movement?
			// Position changed:

			if ( current_pos > -1 ) {
				pos = current_pos;	// Now playing a different tune
				current_pos = -1;
			}

			if ( playlist.size() < 1 ) {
				put_msg("(empty playlist)");
			} else	{
				pos = (pos + rc);
				if ( pos < 0 )
					pos = -1;

				if ( pos >= int(playlist.size()) )
					pos = playlist.size() - 1;

				if ( pos >= 0 ) {
					std::stringstream ss;
					ss << (pos+1) << '>' << song_title(playlist[pos].c_str());
					std::string text = ss.str();

					put_msg(text.c_str());
				} else	{
					put_msg("(Stop)");
				}
				last_sel = time(0);
			}
		} else	{
			// No position change

			time_t now = time(0);
			if ( now - last_update > 60 ) {
				// Everything 30 seconds, update the
				// playlist in case it has been changed
				// externally
				load(playlist);
				last_update = now;
			} else if ( now - last_sel > 1 ) {
				// Start playing new selection
				if ( last_play != pos ) {
					mpc_play(title,pos,of);
					last_play = pos;
				}
				last_sel = time(0);
			}

			usleep(200); // Don't eat the CPU
		}
	}
}

//////////////////////////////////////////////////////////////////////
// THREAD: Volume control thread
//////////////////////////////////////////////////////////////////////

static void
vol_control() {
	int pct, last_pct = -1;

	for (;;) {
		pct = read_volume();
		if ( pct != last_pct ) {
			std::stringstream ss;

			ss << "mpc volume " << pct << " 2>/dev/null 0</dev/null";
			FILE *pfile = popen(ss.str().c_str(),"r");
			char buf[2048];

			while ( fgets(buf,sizeof buf,pfile) )
				;	// Read and discard output, if any
			pclose(pfile);
			last_pct = pct;
		} else	{
			usleep(50000);	// usec
		}
	}
}

//////////////////////////////////////////////////////////////////////
// MAIN PROGRAM:
//////////////////////////////////////////////////////////////////////

int
main(int argc,char **argv) {
	GPIO gpio;		// GPIO access object
	LCD1602 lcd(0x27);	// LCD class
	int rc;

	// Check initialization of GPIO class:
	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	// Initialize LCD
	if ( !lcd.initialize() ) {
		fprintf(stderr,"%s: Initializing LCD1602 at I2C bus %s, address 0x%02X\n",
			strerror(errno),
			lcd.get_busdev(),
			lcd.get_address());
			exit(1);
	}

	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"Warning, %s: opening %s\n",
			strerror(errno),
			i2c_device);
	}

	lcd.clear();
	puts("MPC with Custom Controls");
	lcd.set_cursor(false);

	std::thread thread1(rotary_control,&gpio);
	std::thread thread2(update_status);
	std::thread thread3(lcd_thread,&lcd);
	std::thread thread4(vol_control);

	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();

	return 0;
}

// End mpcctl.cpp
