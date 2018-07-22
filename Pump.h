#ifndef Pump_H_
#define Pump_H_

#include "Arduino.h"
#include "Parameter.h"

class Pump {
private:
	const uint8_t p_pin;

public:
	Parameter const * const flow_capacity;	// Pump flow capacity in cc/min
	Parameter const * const flow_request;	// Requested volume in cc per day
	Parameter * pumped_vol;					// Pumped volume in cc
	Parameter const * const round_runtime_max;		// Max pump ontime per round in seconds
	unsigned long last_switch_on;			// Last time of pump activation.

	/**
	 * Constructor
	 */
	Pump(const byte pin, //
			const Parameter* const flow_capacity_prm, //
			const Parameter* const flow_request_prm, //
			Parameter* const accum_vol_prm, //
			const Parameter* const ontime_prm) :
			p_pin(pin), flow_capacity(flow_capacity_prm), flow_request(
					flow_request_prm), pumped_vol(accum_vol_prm), round_runtime_max(
					ontime_prm), last_switch_on(-1UL) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}

	bool isOn() const;

	unsigned long getPumpedVolume() const;

	unsigned int getIntervalVolume();

	void run(unsigned long now, unsigned long tank_vol, bool inhibit);

private:

	unsigned int getPumpTimeMs(unsigned int v) const;

};

#endif
