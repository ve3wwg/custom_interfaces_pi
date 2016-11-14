// flywheel.hpp : Rotary Switch with flywheel, Warren W. Gay ve3wwg

#ifndef FLYWHEEL_HPP
#define FLYWHEEL_HPP

#include <time.h>
#include "rotary.hpp"

// #define FLY_TN	4

/*
 * Class for a (pair) Rotary switch:
 */
class Flywheel : public RotarySwitch {
	timespec	t0;	// Time of last motion
	double		ival;	// Last interval time
	int		lastr;	// Last returned value r

public:	Flywheel(GPIO& gpio,int clk,int dt,unsigned mask=0xF);
	int read();		// Returns +1, 0 or -1
};

#endif // FLYWHEEL_HPP

// End flywheel.hpp
