//////////////////////////////////////////////////////////////////////
// writedac.cpp 
// Date: Tue Aug 23 16:31:12 2016  (C) Warren W. Gay VE3WWG 
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

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;
static int i2c_addr = 0x48;

int
i2c_write(int addr,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t txbuf[2];
	int rc;

	txbuf[0] = 0b01000000;		// Enable DAC
	txbuf[1] = byte;		// DAC value to write

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = txbuf;
	iomsgs[0].len = 2;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

int
main(int argc,char **argv) {
	int rc = 0, optch;

	while ( (optch = getopt(argc,argv,"a:h")) != -1) {
		switch ( optch ) {
		case 'h' :
			printf(
				"%s [-a address] [-h] values to write...\n"
				"where:\n"
				"\t-a address\tSpecify I2C address\n"
				"\t-h\t\tHelp\n"
				"\n"
				"Prefix values with '0x' for hexadeimal etc.\n",
				argv[0]);
			exit(0);
		case 'a' :
			i2c_addr = strtol(optarg,0,0);
			break;
		default :
			fprintf(stderr,"Unsupported option: -%c\n",optch);
			rc = 1;
		}
	}
	
	if ( rc )
		exit(1);

	if ( argc <= 1 ) {
		fprintf(stderr,"No values provided to write to DAC.\n");
		exit(1);
	}

	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"%s: opening %s\n",
			strerror(errno),
			i2c_device);
		exit(1);
	}

	for ( int x=1; x<argc; ++x ) {
		uint8_t value = strtol(argv[x],0,0);
		rc = i2c_write(i2c_addr,value);
		if ( rc == -1 ) {
			fprintf(stderr,"%s: writing %s device at 0x%02X\n",
				strerror(errno),
				i2c_device,
				i2c_addr);
			exit(1);
		}
	}

	close(i2c_fd);
	return 0;
}

// End writedac.cpp
