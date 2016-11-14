//////////////////////////////////////////////////////////////////////
// rotary.cpp -- Rotary Switch Class
// Date: Thu Aug 25 01:22:12 2016 (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include "rotary.hpp"

/*
 * State array for CLK & DT switch settings:
 */
static unsigned states[4] = 
	{ 0b00, 0b10, 0b11, 0b01 };


/*
 * Constructor: clk and dt GPIO #'s to use:
 */
RotarySwitch::RotarySwitch(GPIO& agpio,int aclk,int adt,unsigned amask)
: clk(agpio,aclk,amask), dt(agpio,adt,amask) {

	// Initialize according to current sw state
	unsigned pair = read_pair();

	for ( index=0; index<4; ++index ) {
		if ( pair == states[index] )
			return;
	}
	index = 0; // Faulty switch?
};


/*
 * Protected: Read switch pair
 */
unsigned
RotarySwitch::read_pair() {
	return (clk.read() << 1) | dt.read();
}


/*
 * Read rotary switch:
 *
 * RETURNS:
 *	+1	Rotated clockwise
 *	 0	No change
 *	-1	Rotated counter-clockwise
 */
int
RotarySwitch::read() {
	unsigned pair = read_pair();
	unsigned cw = (index + 1) % 4;
	unsigned ccw = (index + 3) % 4;

	if ( pair != states[index] ) {
		// State has changed
		if ( pair == states[cw] ) {
			index = cw;
			return (pair == 0b11 || pair == 0b00) ? +1 : 0;
		} else if ( pair == states[ccw] ) {
			index = ccw;
			return (pair == 0b11 || pair == 0b00) ? -1 : 0;
		}
	}

	return 0;	// No change
}

// End rotary.cpp
