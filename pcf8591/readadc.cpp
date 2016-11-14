//////////////////////////////////////////////////////////////////////
// readadc.cpp 
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
static int i2c_ainx = 3;		// Default to AIN3
static int enable_dac = 0;		// -d (enable DAC)

int
i2c_read(int addr,uint8_t control,uint8_t& read_byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[2];
	uint8_t rxbuf[3];
	int rc;

	iomsgs[0].addr = iomsgs[1].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &control;
	iomsgs[0].len = 1;

	iomsgs[1].flags = I2C_M_RD;	// Read
	iomsgs[1].buf = rxbuf;
	iomsgs[1].len = 3;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 2;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	read_byte = rxbuf[2];
	return rc < 0 ? -1 : 0;
}

int
main(int argc,char **argv) {
	int rc = 0, optch;
	uint8_t rbyte;

	while ( (optch = getopt(argc,argv,"a:i:dh")) != -1) {
		switch ( optch ) {
		case 'h' :
			printf(
				"%s [-a address] [-i input] [-h]\n"
				"where:\n"
				"\t-a address\tSpecify I2C address\n"
				"\t-i input\tSpecify AINx (AIN%d is default)\n"
				"\t-d\t\tEnable and leave DAC enabled\n"
				"\t-h\t\tHelp\n",
				argv[0],
				i2c_ainx);
			exit(0);
		case 'a' :
			i2c_addr = strtol(optarg,0,0);
			break;
		case 'i':
			i2c_ainx = strtol(optarg,0,0);
			break;
		case 'd':
			enable_dac = 0x40;
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

	rc = i2c_read(i2c_addr,i2c_ainx|enable_dac,rbyte);
	if ( rc == -1 ) {
		fprintf(stderr,"%s: reading %s device at 0x%02X\n",
			strerror(errno),
			i2c_device,
			i2c_addr);
		exit(1);
	}

	printf("%u\n",rbyte);
	close(i2c_fd);
	return 0;
}

// End readadc.cpp
