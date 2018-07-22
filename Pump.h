#ifndef Pump_H_
#define Pump_H_

#include "Arduino.h"
#include "Parameter.h"

class Pump {
private:
	const uint8_t p_pin;

public:
	Parameter const * const flow_capacity;		// Pump flow capacity in cc/min
	Parameter const * const flow_request;	// Requested volume in cc per day
	Parameter * pumped_vol;	// Pumped volume in cc
	Parameter const * const ontime;		// Pump ontime per round in seconds
	unsigned long onsince;	// Last time of pump activation.

	Pump(byte const pin, //
			Parameter const * const flow_capacity_prm, //
			Parameter const * const flow_request_prm, //
			Parameter * const accum_vol_prm, //
			Parameter const * const interval_prm);

	bool isOn() const;

	unsigned long getPumpedVolume() const;

	unsigned int getIntervalVolume();

	void run(unsigned long now, unsigned long tank_vol, bool inhibit);

private:

	unsigned int getPumpTime(unsigned int v) const;

};

#endif
