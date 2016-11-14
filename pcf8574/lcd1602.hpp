// lcd1602.hpp

#ifndef LCD1602_HPP
#define LCD1602_HPP

#include <stdint.h>
#include <string>

class LCD1602 {
protected:
	struct s_lcd {
		uint8_t		rs : 1;		// P0: 0=Instruction, 1=Data
		uint8_t		rw : 1;		// P1: 0=Write, 1=Read
		uint8_t		e : 1;		// P2: Enable on falling edge
		uint8_t		backlight : 1;	// P3: Enable backlighting
		uint8_t		nibble : 4;	// P7..P4
	};

	struct s_status {
		uint8_t		ac : 7;		// DB6..0: AC6..AC0
		uint8_t		busy : 1;	// DB7: Busy flag
	};

	struct s_nibbles {
		uint8_t		lower : 4;	// Lower nibble
		uint8_t		upper : 4;	// Upper nibble
	};

	union u_lcd {
		s_lcd		lcd;		// Writing
		s_status	status;		// Reading
		s_nibbles	nibbles;
		uint8_t		byte;
	};

	unsigned	i2c_bus : 8;	// I2C Bus
	unsigned	backlight : 1;	// 1=backlight on
	unsigned	i2c_addr : 7;	// I2C Address
	unsigned	cursor : 1;	// 1=Cursor on
	unsigned	blink : 1;	// 1=Blinking cursor
	unsigned	display : 1;	// 1=Display on
	u_lcd		status;		// Last read status
	std::string	busdev;		// Bus device pathname
	int		fd;		// Open file descriptor to I2C driver

	bool i2c_read(uint8_t& byte);
	bool i2c_write(uint8_t byte);
	bool lcd_write4(uint8_t byte,bool instr=true);
	bool lcd_write(uint8_t byte,bool instr=true);

	bool config_display();		// Display/blink/cursor

public:	LCD1602(unsigned i2c_addr=0x27,unsigned bus=1);
	~LCD1602();

	const char *get_busdev() const 	{ return busdev.c_str(); };
	unsigned get_bus() const	{ return i2c_bus; }
	unsigned get_address() const	{ return i2c_addr; }

	bool initialize();		// Initialize the LCD
	bool lcd_ready();

	bool lcd_cmd(uint8_t byte)	{ return lcd_write(byte,true); }
	bool lcd_data(uint8_t byte)	{ return lcd_write(byte,false); }

	bool clear();
	bool home();
	bool moveto(unsigned y,unsigned x);

	bool putstr(const char *str);

	bool set_display(bool on=true);
	bool set_blink(bool on=true);
	bool set_backlight(bool on=true);
	bool set_cursor(bool on=true);

	bool get_display() const	{ return display; };
	bool get_blink() const		{ return blink; };
	bool get_backlight() const	{ return backlight; };
	bool get_cursor() const		{ return cursor; };
};

#endif // LCD1602_HPP

// End lcd1602.hpp
