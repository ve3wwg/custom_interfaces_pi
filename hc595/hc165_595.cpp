//////////////////////////////////////////////////////////////////////
// hc165_595.cpp -- 74HC165/595 Demonstration
// Date: Tue Aug 23 02:39:58 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gpio.hpp"

static int gpio_qh      = 23;	// Input
static int gpio_latch	= 16;	// Outputs
static int gpio_sclk	= 5;
static int gpio_sda	= 24;

static unsigned usecs	= 1;
static GPIO gpio;

static void
nap() {
	usleep(usecs);
}

static unsigned
io(unsigned data) {
	unsigned b, ib, temp = data;
	unsigned idata = 0u;

	gpio.write(gpio_sclk,0);		// SCLK = low
	nap();

	for ( int x=0; x<8; ++x ) {
		ib = gpio.read(gpio_qh);	// Read 74HC165 QH output
		idata = (idata << 1) | ib;	// Collect input bits

		b = !!(temp & 0x80);		// Set b to output data bit
		temp <<= 1;

		gpio.write(gpio_sda,b);		// write data bit
		nap();				// Wait
		gpio.write(gpio_sclk,1);	// SCLK low -> high (shift data)
		nap();
		gpio.write(gpio_sclk,0);	// Complete clock pulse
		nap();
	}
	gpio.write(gpio_latch,0);		// Set new inputs
	nap();
	gpio.write(gpio_latch,1);		// Latch data to outputs
	nap();
	printf("Read %02X, Wrote %02X\n",idata,data);

	return idata;
}

int
main(int argc,char **argv) {
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(gpio_qh,GPIO::Input);
	assert(!gpio.get_error());
	gpio.configure(gpio_qh,GPIO::Up);
	assert(!gpio.get_error());

	gpio.configure(gpio_sda,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(gpio_latch,GPIO::Output);
	assert(!gpio.get_error());

	gpio.configure(gpio_sclk,GPIO::Output);
	assert(!gpio.get_error());

	gpio.write(gpio_sda,0);
	gpio.write(gpio_sclk,1);

	gpio.write(gpio_latch,0);	// LATCH = low
	nap();
	gpio.write(gpio_latch,1);	// LATCH = high (clock input data)
	nap();

	for ( int x=0; x<258; ++x )
		io(x & 0xFF);
	return 0;
}

// End hc165_595.cpp
