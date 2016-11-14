// rotary.hpp : Rotary Switch Class, Warren W. Gay ve3wwg

#ifndef ROTARY_HPP
#define ROTARY_HPP

#include "switch.hpp"

/*
 * Class for a (pair) rotary switch:
 */
class RotarySwitch {
	Switch		clk;	// CLK pin
	Switch		dt;	// DT pin
	unsigned	index;	// Index into states[]
protected:
	unsigned read_pair();	// Read (CLK << 1) | DT

public:	RotarySwitch(GPIO& gpio,int clk,int dt,unsigned mask=0xF);
	int read();		// Returns +1, 0 or -1
};

#endif // ROTARY_HPP

// End rotary.hpp
