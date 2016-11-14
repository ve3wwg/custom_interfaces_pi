//////////////////////////////////////////////////////////////////////
// lcd1602.cpp -- LCD 16x2
// Date: Wed Aug 24 04:08:54 2016  (C) Warren W. Gay VE3WWG 
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

#include "lcd1602.hpp"
#include <sstream>


LCD1602::LCD1602(unsigned i2c_addr,unsigned bus) {

	this->i2c_bus = bus;
	this->i2c_addr = i2c_addr;
	this->backlight = 1;
	status.byte = 0x00;
	fd = -1;
	cursor = 0;
	blink = 0;
	display = 1;
}

LCD1602::~LCD1602() {
	if ( fd >= 0 ) {
		::close(fd);
		fd = -1;
	}
}

bool
LCD1602::i2c_write(uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	int rc;

	iomsgs[0].addr = unsigned(i2c_addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &byte;
	iomsgs[0].len = 1;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ::ioctl(fd,I2C_RDWR,&msgset);
	return rc >= 0;
}

bool
LCD1602::i2c_read(uint8_t& byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	int rc;

	iomsgs[0].addr = unsigned(i2c_addr);
	iomsgs[0].flags = I2C_M_RD;	// Read
	iomsgs[0].buf = &byte;		// Into byte
	iomsgs[0].len = 1;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ::ioctl(fd,I2C_RDWR,&msgset);
	return rc >= 0;
}

bool
LCD1602::lcd_write4(uint8_t byte,bool instr) {
	u_lcd u;
	
	u.byte = 0;
	u.lcd.rs = instr ? 0 : 1;
	u.lcd.rw = 0;
	u.lcd.e = 1;
	u.lcd.backlight = backlight;
	u.lcd.nibble = byte >> 4;
	if ( !i2c_write(u.byte) )
		return false;

	u.lcd.e = 0;
	return i2c_write(u.byte);
}

bool
LCD1602::lcd_ready() {
	u_lcd u;
	uint8_t b1, b2;

	u.byte = 0;
	u.lcd.rs = 0;		// Status
	u.lcd.rw = 1;		// Set Read mode
	u.lcd.e = 0;		// Remains low

	u.lcd.backlight = backlight;
	u.lcd.nibble = 0x0F;	// Allow DB7..DB4 to be inputs

	if ( !i2c_write(u.byte) )
		return false;

	for (;;) {
		u.lcd.e = 1;	// Allow e=1
		if ( !i2c_write(u.byte) )
			return false;

		usleep(1);
		if ( !i2c_read(b1) )
			return false;

		status.byte = b1 & 0xF0;

		u.lcd.e = 0;
		if ( !i2c_write(u.byte) )
			return false;

		u.lcd.e = 1;
		if ( !i2c_write(u.byte) )
			return false;

		if ( !i2c_read(b2) )
			return false;

		status.byte |= b2 >> 4;

		u.lcd.e = 0;
		if ( !i2c_write(u.byte) )
			return false;

		if ( !status.status.busy )
			return true;
	}
}

bool
LCD1602::lcd_write(uint8_t byte,bool instr) {

	if ( !lcd_ready() )
		return false;
	if ( !lcd_write4(byte,instr) || !lcd_write4(byte << 4,instr) )
		return false;
	return true;
}

bool
LCD1602::initialize() {

	if ( fd < 0 ) {
		// Open the I2C Bus:
		std::stringstream ss;

		ss << "/dev/i2c-" << i2c_bus;
		busdev = ss.str();

		fd = ::open(busdev.c_str(),O_RDWR);
		if ( fd == -1 )
			return false;
	}

	usleep(20000);		// 20 ms
	lcd_write4(0x30);	// 8-bit mode initialize
	usleep(10000);		// 10 ms
	lcd_write4(0x30);
	usleep(1000);		// 1 ms
	lcd_write4(0x30);
	usleep(1000);		// 1 ms
	lcd_write4(0x20);	// 4-bit mode initialize
	usleep(1000);		// 1 ms

	if ( !clear() )
		return false;
	if ( !lcd_cmd(0x02) )
		return false;
	if ( !lcd_cmd(0x08) )	// Display on
		return false;
	if ( !lcd_cmd(0x2C) )
		return false;

	return true;
}

bool
LCD1602::config_display() {
	uint8_t cmd = 0x08;

	cmd |= display ? 0x04 : 0;
	cmd |= cursor ? 0x02 : 0;
	cmd |= cursor && blink ? 0x01 : 0;

	return lcd_cmd(cmd);	
}

bool
LCD1602::set_blink(bool on) {
	blink = on;
	return config_display();
}

bool
LCD1602::set_backlight(bool on) {
	backlight = on;
	return config_display();
}

bool
LCD1602::set_cursor(bool on) {
	cursor = on;
	return config_display();
}

bool
LCD1602::set_display(bool on) {
	display = on;
	return config_display();
}

bool
LCD1602::putstr(const char *str) {

	while ( *str ) {
		if ( !lcd_data(*str++) )
			return false;
	}
	return true;
}

bool
LCD1602::clear() {
	bool bf = lcd_cmd(0x01);
	config_display();
	return bf;
}

bool
LCD1602::home() {
	return lcd_cmd(0x02);
}

bool
LCD1602::moveto(unsigned y,unsigned x) {
	unsigned ac = (y ? 0x40 : 0) + (x % 0x28);
	uint8_t cmd;

	cmd = 0x80 | ac;
	return lcd_cmd(cmd);
}

// End lcd1602.cpp
