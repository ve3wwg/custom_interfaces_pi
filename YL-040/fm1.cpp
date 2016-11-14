//////////////////////////////////////////////////////////////////////
// fm1.cpp -- FM Dial without Flywheel effect
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
#include "rotary.hpp"

/*
 * Main test program:
 */
int
main(int argc,char **argv) {
	GPIO gpio;		// GPIO access object
	int rc, counter = 1000;

	// Check initialization of GPIO class:
	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	puts("FM Dial with NO Flywheel Effect.");
	printf("%5.1lf MHz\n",double(counter)/10.0);

	// Instantiate our rotary switch:
	RotarySwitch rsw(gpio,20,21);

	// Loop reading rotary switch for changes:
	for (;;) {
		rc = rsw.read();
		if ( rc != 0 ) {
			// Position changed:
			counter += rc;
			if ( counter < 881 )
				counter = 881;
			else if ( counter > 1079 )
				counter = 1079;
			printf("%5.1lf MHz\n",double(counter)/10.0);
		} else	{
			// No position change
			usleep(1);
		}
	}

	return 0;
}

// End fm1.cpp
