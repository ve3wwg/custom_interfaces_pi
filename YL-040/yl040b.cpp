//////////////////////////////////////////////////////////////////////
// yl040b.cpp -- Reading rotational switch debounced
// Date: Tue Oct 11 11:54:01 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>

#include "switch.hpp"

int
main(int argc,char **argv) {
	unsigned uv, lastv = ~0u;
	GPIO gpio;
	int rc;

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	Switch CLK(gpio,20), DT(gpio,21);

	puts("Monitoring rotary control:");

	for (;;) {
		uv = (CLK.read() << 1) | DT.read();
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

// End yl040b.cpp
