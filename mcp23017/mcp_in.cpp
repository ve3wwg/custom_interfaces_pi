//////////////////////////////////////////////////////////////////////
// mcp_in.cpp -- MCP23017 Demonstration
// Date: Tue Aug 23 05:17:24 2016   (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////
//
// 1. Configures MCP23017 GPIOA and B as input ports (with pull-ups)
// 2. Read GPIO A and B and report changes
// 3. Until the program is killed

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

#define IODIR			0
#define IPOL			1
#define GPINTEN			2
#define DEFVAL			3
#define INTCON			4
#define IOCON			5
#define GPPU			6
#define INTF			7
#define INTCAP			8
#define GPIO			9
#define OLAT			10

#define GPIOA			0
#define GPIOB			1

#define MCP_REGISTER(r,g) (((r)<<1)|(g))

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;
static int i2c_addr = 0x20;

int
i2c_write_data(int addr,int ab,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t buf[2];
	int rc;

	buf[0] = MCP_REGISTER(GPIO,ab);
	buf[1] = byte;

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = buf;
	iomsgs[0].len = 2;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

int
i2c_read_data(int addr,int ab,uint8_t& byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[2];
	uint8_t txbuf[1];
	int rc;

	txbuf[0] = MCP_REGISTER(GPIO,ab);

	iomsgs[0].addr = iomsgs[1].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = txbuf;
	iomsgs[0].len = 1;

	iomsgs[1].flags = I2C_M_RD;	// Read
	iomsgs[1].buf = &byte;		// Pass back data byte
	iomsgs[1].len = 1;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 2;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

static int
i2c_write(int addr,int reg,int ab,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t buf[2];
	int rc;
	
	buf[0] = MCP_REGISTER(reg,ab);
	buf[1] = byte;

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = 0;
	iomsgs[0].buf = buf;
	iomsgs[0].len = 2;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;
	
	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

static int
i2c_write_both(int addr,int reg,uint8_t value) {

	if ( i2c_write(addr,reg,GPIOA,value) || i2c_write(addr,reg,GPIOB,value) )
		return -1;
	return 0;
}

int
main(int argc,char **argv) {
	int rc;

	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"%s: opening %s\n",
			strerror(errno),
			i2c_device);
		exit(1);
	}

	rc = i2c_write_both(i2c_addr,IOCON,0b01000100);	// MIRROR=1,ODR=1
	assert(!rc);

	rc = i2c_write_both(i2c_addr,GPINTEN,0x00);	// No interrupts enabled
	assert(!rc);

	rc = i2c_write_both(i2c_addr,DEFVAL,0x00);	// Clear default value
	assert(!rc);

	rc = i2c_write_both(i2c_addr,OLAT,0x00);	// OLATx=0
	assert(!rc);

	rc = i2c_write_both(i2c_addr,IPOL,0b00000000);	// No inverted polarity
	assert(!rc);

	rc = i2c_write_both(i2c_addr,GPPU,0b11111111);	// Enable all pull-ups
	assert(!rc);

	rc = i2c_write_both(i2c_addr,IODIR,0b11111111);	// All are inputs
	assert(!rc);

	{
		uint8_t gpioa, gpiob, temp;

		rc = i2c_read_data(i2c_addr,GPIOA,gpioa);
		assert(!rc);

		rc = i2c_read_data(i2c_addr,GPIOB,gpiob);
		assert(!rc);

		for (;;) {
			printf("GPIOA = 0x%02X, GPIOB = 0x%02X\n",gpioa,gpiob);

			for (;;) {
				rc = i2c_read_data(i2c_addr,GPIOA,temp);
				assert(!rc);
				if ( temp != gpioa ) {
					gpioa = temp;
					break;
				}

				rc = i2c_read_data(i2c_addr,GPIOB,temp);
				if ( temp != gpiob ) {
					gpiob = temp;
					break;
				}
				usleep(1);
			}
		} 		
	}

	close(i2c_fd);
	return 0;
}

// End mcp_in.cpp
