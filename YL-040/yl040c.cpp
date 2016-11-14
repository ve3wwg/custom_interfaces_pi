//////////////////////////////////////////////////////////////////////
// yl040c.cpp -- Reading rotational switch debounced
//		 and error recovery
// Date: Tue Oct 11 11:54:35 2016  (C) Warren W. Gay VE3WWG 
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
#include "rotary.hpp"

/*
 * Main test program:
 */
int
main(int argc,char **argv) {
	GPIO gpio;		// GPIO access object
	int rc, counter = 0;

	// Check initialization of GPIO class:
	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	puts("Monitoring rotary control:");

	// Instantiate our rotary switch:
	RotarySwitch rsw(gpio,20,21);

	// Loop reading rotary switch for changes:
	for (;;) {
		rc = rsw.read();
		if ( rc != 0 ) {
			// Position changed:
			counter += rc;
			printf("Step %+d, Count %d\n",rc,counter);
		} else	{
			// No position change
			usleep(1);
		}
	}

	return 0;
}

// End yl040c.cpp
