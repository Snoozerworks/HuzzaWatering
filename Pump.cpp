// Do not remove the include below
#include "Pump.h"

/**
 * Constructor
 */
Pump::Pump(byte const pin, //
		Parameter const * const flow_prm, //
		Parameter const * const rqst_vol_prm, //
		Parameter * const accum_vol_prm, //
		Parameter const * const interval_prm) :
		p_pin(pin), flow(flow_prm), rqst_vol(rqst_vol_prm), pumped_vol(
				accum_vol_prm), ontime(interval_prm), onsince(-1UL) {

	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

/**
 * Returns true if pump is running
 */
bool Pump::isOn() const {
	return (digitalRead(p_pin) == HIGH);
}

/**
 * Returns the total pumped volume.
 */
unsigned long Pump::getPumpedVolume() const {
	return pumped_vol->get();
}

/**

 * Method starts pump at intervals to deliver the requested flow.
 * If inhibit is true, pump may shut off but not start.
 *
 * tbd Returns true when pump is switched off, i.e.
 */
void Pump::run(unsigned long now, unsigned long tank_vol, bool inhibit) {
	unsigned long _ontime;	// Pump ontime per round [s]
	unsigned long vol;		// Pump volume per round [cc]
	unsigned long interval;	// Time between two rounds [seconds]
	unsigned long elapsed;	// Time elapsed since start of last round [ms]

	elapsed = now - onsince;

	// Get pump ontime, volume and interval.
	_ontime = ontime->get();
	vol = (_ontime * flow->get() + 30) / 60;	// +30 to round.
	interval = (vol * 86400) / rqst_vol->get();

	// Limit ontime and interv_vol to what remains in tank.
	if (vol > tank_vol) {
		_ontime = (tank_vol * 60) / flow->get();
		vol = tank_vol;
	}

//	Serial.print("1 ");
	delay(100);

	// Switch off pump after _ontime seconds
	if (isOn() && elapsed >= _ontime * 1000) {
		Serial.print("Turn off pin ");
		Serial.print(p_pin, DEC);
		Serial.print(", elapsed=");
		Serial.print(elapsed, DEC);

		// Turn off pump
		digitalWrite(p_pin, LOW);

		// Updated pumped volume
		unsigned long new_vol = pumped_vol->get() + vol;
		pumped_vol->set(new_vol);

		return;
	}

//	Serial.print("2 ");

//	delay(5);
//	Serial.print(". ");
//	// If there is nothing to pump, reset onsince and return
	if (_ontime == 0) {
		Serial.print(". ");
		onsince = now;
		Serial.print(". ");
		return;
	}
//	Serial.print(". ");
//
//	Serial.print("3");
//	delay(200);
//
	// Delay activaion of pump if we should wait
	if (inhibit) {
		Serial.print("w");
		return;
	}
//	Serial.print(". ");
//
	delay(5);
//	Serial.print(" 4-");
	delay(5);

//	// Switch on pump interv_time seconds after last activation.
	if (!inhibit && elapsed >= interval * 1000) {
		Serial.print("\nTurn on pin ");
		Serial.print(p_pin, DEC);
		Serial.print(", ontime=");
		Serial.print(_ontime, DEC);
		Serial.print(", vol=");
		Serial.print(vol, DEC);
		Serial.print(", interval=");
		Serial.print(interval, DEC);

		// Turn on pump
		digitalWrite(p_pin, HIGH);

		// Save time when pump turned on
		onsince = now;
	}

}

/***************
 * Private
 ***************/

/**
 * Returns time in ms to keep pump on to deliver a volume v in cc.
 */
unsigned int Pump::getPumpTime(unsigned int v) const {
	// Calculate the duration in ms for the the pump to be on.
	return (v * 60000) / flow->get();
}

