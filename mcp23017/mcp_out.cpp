//////////////////////////////////////////////////////////////////////
// mcp_out.cpp -- MCP23017 Demonstration
// Date: Tue Aug 23 05:17:24 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////
//
// 1. Configures MCP23017 GPIOA and B as output ports
// 2. Output GPIOA as 0x0B
// 3. Output GPIOB as 0xC1

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

static int
i2c_write(int addr,int reg,int ab,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t reg_addr = MCP_REGISTER(reg,ab);
	uint8_t buf[2];
	int rc;
	
	buf[0] = reg_addr;
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
	rc = i2c_write_both(i2c_addr,GPPU,0b11111111);	// Enable all pull-ups
	assert(!rc);
	rc = i2c_write_both(i2c_addr,IPOL,0b00000000);	// No inverted polarity
	assert(!rc);
	rc = i2c_write_both(i2c_addr,IODIR,0b00000000);	// All are outputs
	assert(!rc);

	rc = i2c_write_data(i2c_addr,GPIOA,0x0B);	// GPIOA = 0x0B
	printf("GPIOA = 0x0B\n");
	rc = i2c_write_data(i2c_addr,GPIOB,0xC1);	// GPIOA = 0xC1
	printf("GPIOB = 0xC1\n");

	close(i2c_fd);
	return 0;
}

// End mcp_out.cpp
