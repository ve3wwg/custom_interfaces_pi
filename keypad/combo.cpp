//////////////////////////////////////////////////////////////////////
// combo.cpp -- Combination Lock Keypad
// Date: Tue Nov 04 09:17:40 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <unordered_map>

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;			// I2C bus fd
static uint8_t i2c_keypad_addr = 0x20;	// PCF8574 IC1 address
static uint8_t i2c_led_addr = 0x21;	// PCF8574 IC2 address
static uint8_t green_led = 1;		// Green LED is P0
static uint8_t red_led = 2;		// Red LED is P1

static std::unordered_map<uint8_t,std::string> keymap({
	{ 0xEE, "S1" },	{ 0xED, "S2" },	{ 0xEB, "S3" },
	{ 0xE7, "S4" },	{ 0xDE, "S5" },	{ 0xDD, "S6" },
	{ 0xDB, "S7" },	{ 0xD7, "S8" },	{ 0xBE, "S9" },
	{ 0xBD, "S10" }, { 0xBB, "S11" }, { 0xB7, "S12" },
	{ 0x7E, "S13" }, { 0x7D, "S14" }, { 0x7B, "S15" },
	{ 0x77, "S16" }
});

//////////////////////////////////////////////////////////////////////
// Write byte to IC2 address addr
//////////////////////////////////////////////////////////////////////

void
i2c_write(int addr,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	int rc;

	iomsgs[0].addr = unsigned(addr);// Address
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &byte;		// Buffer
	iomsgs[0].len = 1;		// 1 byte

	msgset.msgs = iomsgs;		// The message
	msgset.nmsgs = 1;		// 1 message

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	if ( rc == -1 ) {
		fprintf(stderr,
			"%s: writing to I2C address 0x%02X\n",
			strerror(errno),
			i2c_keypad_addr);
		exit(1);
	}
}

//////////////////////////////////////////////////////////////////////
// Read one byte from I2C address addr
//////////////////////////////////////////////////////////////////////

uint8_t
i2c_read(int addr) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t byte;
	int rc;

	iomsgs[0].addr = unsigned(addr);// I2C Address
	iomsgs[0].flags = I2C_M_RD;	// Read
	iomsgs[0].buf = &byte;		// buffer
	iomsgs[0].len = 1;		// 1 byte

	msgset.msgs = iomsgs;		// The message
	msgset.nmsgs = 1;		// 1 message

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	if ( rc == -1 ) {
		fprintf(stderr,
			"%s: reading I2C address 0x%02X\n",
			strerror(errno),
			i2c_keypad_addr);
		exit(1);
	}
	return byte;			// Return read byte
}

//////////////////////////////////////////////////////////////////////
// Lookup the key name for the given scan code
//////////////////////////////////////////////////////////////////////

const char *
key_lookup(uint8_t scancode) {

	auto it = keymap.find(scancode);
	if ( it != keymap.end() )
		return it->second.c_str();
	return "???";	// Multiple keys pressed
}

//////////////////////////////////////////////////////////////////////
// Return a scan code from the keypad
//////////////////////////////////////////////////////////////////////

uint8_t
get_key_code() {
	uint8_t keypad, row;

	// Wait for key release:
	while ( (i2c_read(i2c_keypad_addr) & 0x0F) != 0x0F )
		usleep(20000);

	// Wait for key press:
	for (;;) {
		// Scan each keypad row:
		for ( row=0x10; row != 0; row <<= 1 ) {
			// Drive the row select
			i2c_write(i2c_keypad_addr,~row);

			// Read the column inputs
			keypad = i2c_read(i2c_keypad_addr);

			if ( (keypad & 0x0F) != 0x0F )
				return keypad;
			usleep(20000);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Main program
//////////////////////////////////////////////////////////////////////

int
main(int argc,char **argv) {
	static uint8_t code[4] = {
		// 9, 8,    6     7:
		0xBE, 0xD7, 0xDD, 0xDB
	};
	uint8_t keypad, kx;
	bool matched;

	// Open the I2C bus
	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"%s: opening %s\n",
			strerror(errno),
			i2c_device);
		exit(1);
	}

	i2c_write(i2c_led_addr,0xFF);	// Both LEDs off

	for (;;) {
		puts("*** LOCKED ***");

		// Wait for entry of all four codes:
		matched = true;
		for ( kx=0; kx<4; ++kx ) {
			keypad = get_key_code();
			printf("code[%u] = %02X (%s)\n",
				kx,keypad,
				key_lookup(keypad));
			if ( keypad != code[kx] )
				matched = false;
			if ( kx == 0 ) {
				// All LEDs off
				i2c_write(i2c_led_addr,0xFF);
			}
		}	
		if ( !matched ) {
			// Failed: Red LED + start over
			i2c_write(i2c_led_addr,~red_led);
			puts("*** FAILED ***");
			continue;
		}
		// Code matched: unlock
		i2c_write(i2c_led_addr,~green_led);

		// Wait for any key code to lock
		// and restart
		puts("*** UNLOCKED ***");
		get_key_code();
		i2c_write(i2c_led_addr,0xFF);
	}

	return 0;
}

// End combo.cpp

