//////////////////////////////////////////////////////////////////////
// hc165.cpp -- 74HC165 Demonstration
// Date: Tue Aug 16 21:20:08 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gpio.hpp"

static int hc165_pl	= 16;	// Outputs
static int hc165_ce 	= 6;
static int hc165_cp	= 5;
static int hc165_q7	= 23;	// Input

static unsigned usecs	= 1;
static GPIO gpio;

static void
nap() {
	usleep(usecs);
}

static void
strobe(int pin) {
	nap();
	gpio.write(hc165_pl,0);		// Load parallel
	nap();
	gpio.write(hc165_pl,1);		// Loaded
	nap();
}

static unsigned
hc165_read() {
	unsigned byte = 0;

	strobe(hc165_pl);		// Load parallel
	gpio.write(hc165_cp,0);		// CP = low
	gpio.write(hc165_ce,0);		// /CE = low
	nap();

	for ( int x=0; x<8; ++x ) {
		byte <<= 1;
		byte |= !!gpio.read(hc165_q7);	// Read Q7
		gpio.write(hc165_cp,1);		// Shift
		nap();
		gpio.write(hc165_cp,0);		// Complete clock pulse
		nap();
	}
	gpio.write(hc165_ce,1);
	return byte;
}

int
main(int argc,char **argv) {
	unsigned byte = 0;
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(hc165_q7,GPIO::Input);
	assert(!gpio.get_error());
	gpio.configure(hc165_q7,GPIO::Up);
	assert(!gpio.get_error());

	gpio.configure(hc165_pl,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(hc165_ce,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(hc165_cp,GPIO::Output);
	assert(!gpio.get_error());

	gpio.write(hc165_ce,1);
	gpio.write(hc165_cp,1);
	gpio.write(hc165_pl,1);

	byte = hc165_read();

	printf("byte = %02X\n",byte);
	return 0;
}

// End hc165.cpp
