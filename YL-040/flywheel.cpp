//////////////////////////////////////////////////////////////////////
// flywheel.cpp -- Flywheeling control implementation
// Date: Tue Oct 11 11:48:48 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include "flywheel.hpp"

static inline void
get_time(timespec& tv) {
	int rc = clock_gettime(CLOCK_MONOTONIC,&tv);
	assert(!rc);
}

static inline double
as_double(const timespec& tv) {
	return double(tv.tv_sec) + double(tv.tv_nsec) / 1000000000.0;
}

static inline double
timediff(const timespec& t0,const timespec& t1) {
	double d0 = as_double(t0), d1 = as_double(t1);

	return d1 - d0;
}

Flywheel::Flywheel(GPIO& agpio,int aclk,int adt,unsigned amask)
: RotarySwitch(agpio,aclk,adt,amask) {

	get_time(t0);
	lastr = 0;
};

int
Flywheel::read() {
	static const double speed = 15.0; // Lower values are faster changing
	timespec t1;			// Time now
	double diff, rate;		// Difference, rate
	int r, m = 1;			// Return r, m multiple

	r = RotarySwitch::read();	// Get reading (debounced)
	get_time(t1);			// Now
	diff = timediff(t0,t1);		// Diff in seconds

	if ( r != 0 ) {			// Got a click event
		lastr = r;		// Save the event type
		ival = ( ival * 0.75 + diff * 1.25 ) / 2.0 * 1.10;
		if ( ival < 1.0 && ival > 0.00001 ) {
			rate = 1.0 / ival;
		} else	rate = 0.0;
		t0 = t1;
		if ( speed > 0.0 )
			m = rate / speed;
	} else if ( diff > ival && ival >= 0.000001 ) {
		rate = 1.0 / ival;	// Compute a rate
		if ( rate > 15.0 ) {
			if ( speed > 0.0 )
				m = rate / speed;
			ival *= 1.2;	// Increase interval
			t0 = t1;
			r = lastr;	// Return last r
		}
	}

	return m > 0 ? r * m : r;
}

// End flywheel.cpp
