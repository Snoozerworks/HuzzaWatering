// Do not remove the include below
#include "Pump.h"

/**
 * Constructor
 */
Pump::Pump(byte pin, Parameter* flow_prm, Parameter* rqst_vol_prm,
		Parameter* accum_vol_prm, Parameter* interval_prm) :
		p_pin(pin), flow(flow_prm), rqst_vol(rqst_vol_prm), accum_vol(
				accum_vol_prm), interval(interval_prm), last_activation(-1) {

	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

/**
 * Returns true if pump is running
 */
bool Pump::isOn() {
	return (digitalRead(p_pin) == HIGH);
}

/**
 * Returns the total pumped volume.
 */
unsigned long Pump::getPumpedVolume() {
	return accum_vol->get();
}

/**
 * Returns volume to deliver at each interval
 */
unsigned int Pump::getIntervalVolume() {
	const unsigned long rqst_vol_per_seconds = 24 * 3600;
	// Calculate volume in cc to deliver at each pump interval.
	return (rqst_vol->get() * interval->get()) / rqst_vol_per_seconds;
}

/**
 * Method starts pump at intervals to deliver the requested flow,
 * If wait is true, pump may shut off but not start.
 */
void Pump::run(unsigned long now, unsigned int tank_vol, bool wait) {
	unsigned int v;
	unsigned int pump_time;
	unsigned long t_elapsed;

	// Time elapsed since last time pump turned on
	t_elapsed = now - last_activation;

	// Calculate volume in cc to deliver at each pump interval.
	// Limit it to what remains in tank.
	v = getIntervalVolume();
	if (v > tank_vol) {
		v = tank_vol;
	}

	// Calculate time in ms for the the pump to be on.
	pump_time = getPumpTime(v);

	// Switch off pump when it has delivered the requested volume
	if (isOn() && t_elapsed > pump_time) {
		Serial.print("Turn off pin ");
		Serial.print(p_pin, DEC);
		Serial.print(" @ ");
		Serial.println(now, DEC);

		// Updated pumped volume
		accum_vol->set(accum_vol->get() + v);
		digitalWrite(p_pin, LOW);
		return;
	}

	// If there is nothing to pump, reset last_activation time and return
	if (pump_time == 0) {
		last_activation = now;
		return;
	}

	// Let pump remain switched off if we should wait
	if (wait) {
		return;
	}

	// Switch on pump if elapsed time exceeds the run interval.
	if (t_elapsed / 1000 > interval->get()) {
		Serial.print("Turn on pin ");
		Serial.print(p_pin, DEC);
		Serial.print(" @ ");
		Serial.println(now, DEC);

		// Save time when pump turned on
		last_activation = now;

		// Activate output
		digitalWrite(p_pin, HIGH);
	}

}

/***************
 * Private
 ***************/

/**
 * Returns time in ms to keep pump on to deliver a volume v in cc.
 */
unsigned int Pump::getPumpTime(unsigned int v) {
	// Calculate the duration in ms for the the pump to be on.
	return (v * 60000) / flow->get();
}

