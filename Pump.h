#ifndef Pump_H_
#define Pump_H_

#include "Arduino.h"
#include "Parameter.h"

class Pump {
private:
	const uint8_t p_pin;

public:
	Parameter const * const flow_capacity;	// Pump flow capacity [cc/min]
	Parameter const * const flow_request;	// Requested volume [cc/day]
	Parameter * pumped_vol;					// Pumped volume [cc]
	Parameter const * const round_runtime;	// Pump runtime per round [s].
	unsigned long last_switch_on;			// Time of last pump start [ms].
	unsigned long runtime;					// Pump run time [s].

	/**
	 * Constructor
	 */
	Pump(const byte pin, //
			const Parameter* const flow_capacity_prm, //
			const Parameter* const flow_request_prm, //
			Parameter* const accum_vol_prm, //
			const Parameter* const ontime_prm) :
			p_pin(pin), flow_capacity(flow_capacity_prm), flow_request(
					flow_request_prm), pumped_vol(accum_vol_prm), round_runtime(
					ontime_prm), last_switch_on(-1UL), runtime(0) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}

	bool isOn() const;

	unsigned long getPumpedVolume() const;

	void run(unsigned long now, unsigned int tank_vol, bool inhibit);

private:

	unsigned int getPumpTime(unsigned int vol) const;

};

#endif
