//////////////////////////////////////////////////////////////////////
// mcp_int.cpp -- MCP23017 Demonstration
// Date: Tue Aug 23 08:12:11 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////
//
// 1. Configures MCP23017 GPIOA and B as input ports (with pull-ups)
// 2. Read GPIO A and B and report changes via interrupt INTA
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

#include "gpio.hpp"

#define IODIR			0
#define IPOL			1
#define GPINTEN			2
#define DEFVAL			3
#define INTCON			4
#define IOCON			5
#define GPPU			6
#define INTF			7
#define INTCAP			8
#define GPIO_			9
#define OLAT			10

#define GPIOA			0
#define GPIOB			1

#define MCP_REGISTER(r,g) (((r)<<1)|(g))

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;
static int i2c_addr = 0x20;
static int gpio_inta = 5;	// GPIO for INTA signal

static GPIO gpio;

int
i2c_write_data(int addr,int ab,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t buf[2];
	int rc;

	buf[0] = MCP_REGISTER(GPIO_,ab);
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
i2c_read_data(int addr,int ab,uint8_t reg,uint8_t& byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[2];
	uint8_t txbuf[1];
	int rc;

	txbuf[0] = MCP_REGISTER(reg,ab);

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

	if ( (rc = gpio.get_error()) != 0 ) {
		fprintf(stderr,"%s: starting gpio (sudo?)\n",strerror(rc));
		exit(1);
	}		

	gpio.configure(gpio_inta,GPIO::Input);		// GPIO is input
	assert(!gpio.get_error());
	gpio.configure(gpio_inta,GPIO::Up);		// Enable pull-up resistor
	assert(!gpio.get_error());

	rc = i2c_write_both(i2c_addr,IOCON,0b01000000);	// MIRROR=1,ODR=0,INTPOL=0
	assert(!rc);

	rc = i2c_write_both(i2c_addr,IODIR,0xFF);	// All are inputs
	assert(!rc);

	rc = i2c_write_both(i2c_addr,INTCON,0x00);	// Interrupts compare to self
	assert(!rc);

	rc = i2c_write_both(i2c_addr,GPINTEN,0xFF);	// All interrupts enabled
	assert(!rc);

	rc = i2c_write_both(i2c_addr,IPOL,0x00);	// No inverted input polarity
	assert(!rc);

	rc = i2c_write_both(i2c_addr,GPPU,0xFF);	// Enable all pull-ups
	assert(!rc);

	{
		uint8_t gpioa, gpiob, inta, intf_a = 0, intf_b = 0;

		rc = i2c_read_data(i2c_addr,GPIOA,GPIO_,gpioa);
		assert(!rc);

		rc = i2c_read_data(i2c_addr,GPIOB,GPIO_,gpiob);
		assert(!rc);

		printf("GPIOA 0x%02X INTFA 0x%02X, GPIOB 0x%02X INTFB 0x%02X\n",
			gpioa, intf_a,
			gpiob, intf_b);

		for (;;) {
			inta = gpio.read(gpio_inta);
			if ( inta != 0 ) {
				// No interrupt
				usleep(1);
				continue;
			}

			// Process interrupt

			rc = i2c_read_data(i2c_addr,GPIOA,INTF,intf_a);
			assert(!rc);

			rc = i2c_read_data(i2c_addr,GPIOB,INTF,intf_b);
			assert(!rc);

			rc = i2c_read_data(i2c_addr,GPIOA,INTCAP,gpioa);
			assert(!rc);

			rc = i2c_read_data(i2c_addr,GPIOB,INTCAP,gpiob);
			assert(!rc);

			printf("GPIOA 0x%02X INTFA 0x%02X, GPIOB 0x%02X INTFB 0x%02X\n",
				gpioa, intf_a,
				gpiob, intf_b);
		} 		
	}

	close(i2c_fd);
	return 0;
}

// End mcp_int.cpp
