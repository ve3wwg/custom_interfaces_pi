//////////////////////////////////////////////////////////////////////
// lcd.cpp -- LCD Test
// Date: Wed Aug 24 08:30:48 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "lcd1602.hpp"

int
main(int argc,char **argv) {
	LCD1602 lcd(0x27);

	if ( !lcd.initialize() ) {
		fprintf(stderr,"%s: Initializing LCD1602 at I2C bus %s, address 0x%02X\n",
			strerror(errno),
			lcd.get_busdev(),
			lcd.get_address());
		exit(1);
	}

	lcd.clear();
	lcd.putstr(" Raspberry Pi 3");
	lcd.moveto(1,4);
	lcd.putstr("LCD1602 ");

	return 0;
}

// End lcd.cpp
