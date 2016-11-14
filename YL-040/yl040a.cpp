//////////////////////////////////////////////////////////////////////
// yl040a.cpp -- Simple reading of rotational switch changes
// Date: Thu Aug 25 01:22:12 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>

#include "gpio.hpp"

static GPIO gpio;

static int gpio_CLK = 20;
static int gpio_DT  = 21;

static unsigned
read_rotary() {

	return (gpio.read(gpio_CLK) << 1) | gpio.read(gpio_DT);
}

int
main(int argc,char **argv) {
	unsigned uv, lastv = ~0u;
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(gpio_CLK,GPIO::Input);
	assert(!gpio.get_error());

	gpio.configure(gpio_CLK,GPIO::Up);
	assert(!gpio.get_error());

	gpio.configure(gpio_DT,GPIO::Input);
	assert(!gpio.get_error());

	gpio.configure(gpio_DT,GPIO::Up);
	assert(!gpio.get_error());

	for (;;) {
		uv = read_rotary();
		if ( uv != lastv ) {
			printf("%u%u\n",
				(uv >> 1) & 1,
				uv & 1);
			lastv = uv;
		} else	{
			usleep(1);
		}
	}

	return 0;
}

// End yl040a.cpp

