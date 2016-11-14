//////////////////////////////////////////////////////////////////////
// switch.cpp -- Debounced Switch
// Date: Thu Aug 25 01:22:12 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "switch.hpp"

/*
 * Single switch class, with debounce:
 */
Switch::Switch(GPIO& agpio,int aport,unsigned amask) 
: gpio(agpio), gport(aport), mask(amask) {

	shiftr = 0;
	state = 0;

	// Configure GPIO port as input
	gpio.configure(gport,GPIO::Input);
	assert(!gpio.get_error());

	// Configure GPIO port with pull-up
	gpio.configure(gport,GPIO::Up);
	assert(!gpio.get_error());
}

/*
 * Read debounced switch state:
 */
unsigned
Switch::read() {
	unsigned b = gpio.read(gport);
	unsigned s;

	shiftr = (shiftr << 1) | (b & 1);
	s = shiftr & mask;

	if ( s != mask && s != 0x00 )
		return state;	// Bouncing: return state
	if ( s == state )
		return state;	// No change
	state = shiftr & 1;	// Set new state
	return state;
}

// End switch.cpp
