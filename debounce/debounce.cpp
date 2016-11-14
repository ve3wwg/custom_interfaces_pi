//////////////////////////////////////////////////////////////////////
// debounce.cpp -- Debouncing
// Date: Tue Aug 16 21:20:08 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "gpio.hpp"

static int gpio_pin	= 16;	// Input

static GPIO gpio;

int
main(int argc,char **argv) {
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(gpio_pin,GPIO::Input);
	assert(!gpio.get_error());
	gpio.configure(gpio_pin,GPIO::Up);
	assert(!gpio.get_error());

	printf("Now debouncing gpio_pin = %d. ^C to exit.\n",gpio_pin);

	int state = 0, shiftr = 0, m, b;

	for (;;) {
		b = gpio.read(gpio_pin);	// Read bit

		shiftr = (shiftr << 1) | (b & 1);
		m = shiftr & 0b00001111;	// Mask out low order bits

		if ( m == 0b00001111 || m == 0b00000000 ) {
			if ( (shiftr & 1) != state ) {
				state = shiftr & 1;
				printf("State changed: %d\n",state);
			}
		}
		usleep(10000);
	}

	return 0;
}

// End debounce.cpp
