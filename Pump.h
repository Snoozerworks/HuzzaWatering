#ifndef Pump_H_
#define Pump_H_

#include "Arduino.h"
#include "Parameter.h"

class Pump {
private:
	byte p_pin;

public:
	Parameter* flow;		// Pump flow capacity in cc/min
	Parameter* rqst_vol;	// Requested volume in cc per day
	Parameter* accum_vol;	// Accumulated pumped volume in cc
	Parameter* interval;	// Interval between pump activations in seconds
	unsigned long last_activation;

	Pump(byte pin, Parameter* flow_prm, Parameter* rqst_vol_prm,
			Parameter* accum_vol_prm, Parameter* interval_prm);bool isOn();
	unsigned long getPumpedVolume();

	unsigned int getIntervalVolume();

	void run(unsigned long now, unsigned int tank_vol, bool wait);

private:

	unsigned int getPumpTime(unsigned int v);

};

#endif
