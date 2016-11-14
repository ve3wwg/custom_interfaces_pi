// switch.hpp : Debounced Switch class, Warren W. Gay ve3wwg

#ifndef SWITCH_HPP
#define SWITCH_HPP

#include <stdlib.h>
#include "gpio.hpp"

/*
 * Class for a single switch
 */
class Switch {
	GPIO&		gpio;	// GPIO Access
	int		gport;	// GPIO Port #
	unsigned	mask;	// Debounce mask
	unsigned	shiftr;	// Debouncing shift register
	unsigned	state;	// Current debounced state

public:	Switch(GPIO& gpio,int port,unsigned mask=0xF);
	unsigned read();	// Read debounced
};

#endif // SWITCH_HPP

// End switch.hpp
