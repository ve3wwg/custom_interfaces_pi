//////////////////////////////////////////////////////////////////////
// hc595.cpp -- 
// Date: Tue Aug 23 01:05:40 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gpio.hpp"

static int hc595_latch	= 16;	// Outputs
static int hc595_sclk	= 5;
static int hc595_sda	= 24;

static unsigned usecs	= 1;
static GPIO gpio;

static void
nap() {
	usleep(usecs);
}

static void
hc595_write(unsigned data) {
	unsigned b, temp = data;

	gpio.write(hc595_sclk,0);	// SCLK = low
	gpio.write(hc595_latch,0);	// LATCH = low
	nap();

	for ( int x=0; x<8; ++x ) {
		b = !!(temp & 0x80);		// b = d7 (msb first)
		temp <<= 1;
		gpio.write(hc595_sda,b);	// SDA = data bit
		nap();				// Wait
		gpio.write(hc595_sclk,1);	// SCLK low -> high
		nap();
		gpio.write(hc595_sclk,0);	// Complete clock pulse
		nap();
	}
	gpio.write(hc595_latch,1);		// Latch data to outputs
	nap();
	printf("Wrote %02X to 74HC595\n",data);
}

int
main(int argc,char **argv) {
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(hc595_sda,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(hc595_latch,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(hc595_sclk,GPIO::Output);
	assert(!gpio.get_error());

	gpio.write(hc595_sda,0);
	gpio.write(hc595_sclk,1);
	gpio.write(hc595_latch,1);

	if ( argc > 1 ) {
		for ( int x=1; x<argc; ++x ) {
			hc595_write(atoi(argv[x]));
		}
	} else	{
		hc595_write(0xD5);
	}
	return 0;
}

// End hc595.cpp

