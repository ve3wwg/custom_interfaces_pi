//////////////////////////////////////////////////////////////////////
// i2cio.cpp 
// Date: Wed Aug 24 01:26:33 2016  (C) Warren W. Gay VE3WWG 
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
static int i2c_addr = 0x27;
static int i2c_inputs = 0;

int
i2c_write(int addr,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	int rc;

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &byte;
	iomsgs[0].len = 1;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

int
i2c_read(int addr,uint8_t& byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	int rc;

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = I2C_M_RD;	// Read
	iomsgs[0].buf = &byte;		// Into byte
	iomsgs[0].len = 1;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

int
main(int argc,char **argv) {
	uint8_t v;
	int rc = 0, optch;

	while ( (optch = getopt(argc,argv,"a:i:h")) != -1) {
		switch ( optch ) {
		case 'h' :
			printf(
				"%s [-a address] [-i pins] [-h] [out_value1...]\n"
				"where:\n"
				"\t-a address\tSpecify I2C address\n"
				"\t-i 0xHex\tSpecify input port pins (0=none=default)\n"
				"\t-h\t\tHelp\n",
				argv[0]);
			exit(0);
		case 'a' :   
			i2c_addr = strtol(optarg,0,0);
			break;
		case 'i' :
			i2c_inputs = strtol(optarg,0,0);
			break;
		default :
			fprintf(stderr,"Unsupported option: -%c\n",optch);
			rc = 1;
		}
	}

	if ( rc )
		exit(1);

	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"%s: opening %s\n",
			strerror(errno),
			i2c_device);
		exit(1);
	}

	if ( i2c_inputs != 0 ) {
		// Configure inputs
		rc = i2c_read(i2c_addr,v);
		if ( rc == -1 ) {
			fprintf(stderr,"%s: Reading from I2C device 0x%02X on %s\n",
				strerror(errno),
				i2c_addr,
				i2c_device);
			exit(2);
		}
		v |= i2c_inputs;	// Set a 1-bit for each input

		rc = i2c_write(i2c_addr,v);
		if ( rc == -1 ) {
			fprintf(stderr,"%s: Writing to I2C device 0x%02X on %s\n",
				strerror(errno),
				i2c_addr,
				i2c_device);
			exit(2);
		}
	}

	if ( optind < argc ) {
		// Write all output values
		for ( int x=optind; x<argc; ++x ) {
			uint8_t v = strtoul(argv[x],0,0);
	
			rc = i2c_write(i2c_addr,v);
			if ( rc == -1 ) {
				fprintf(stderr,"%s: Writing to I2C device 0x%02X on %s\n",
					strerror(errno),
					i2c_addr,
					i2c_device);
				exit(2);
			} else	{
				printf("I2C Wrote: 0x%02X\n",v);
			}

			if ( i2c_inputs ) {
				rc = i2c_read(i2c_addr,v);
				if ( rc == -1 ) {
					fprintf(stderr,"%s: Reading from I2C device 0x%02X on %s\n",
						strerror(errno),
						i2c_addr,
						i2c_device);
					exit(2);
				}

				printf("I2C Read: 0x%02X (Inputs: %02X)\n",v,i2c_inputs);
			}
		}
	} else if ( i2c_inputs ) {
		rc = i2c_read(i2c_addr,v);
		if ( rc == -1 ) {
			fprintf(stderr,"%s: Reading from I2C device 0x%02X on %s\n",
				strerror(errno),
				i2c_addr,
				i2c_device);
			exit(2);
		}

		printf("I2C Read: 0x%02X (Inputs: %02X)\n",v,i2c_inputs);
	}

	close(i2c_fd);
	return 0;
}

// End i2cio.cpp
