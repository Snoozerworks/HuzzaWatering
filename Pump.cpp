// Do not remove the include below
#include "Pump.h"

/**
 * Constructor
 */
Pump::Pump(byte const pin, //
		Parameter const * const flow_prm, //
		Parameter const * const rqst_vol_prm, //
		Parameter * const accum_vol_prm, //
		Parameter const * const ontime_prm) :
		p_pin(pin), flow(flow_prm), rqst_vol(rqst_vol_prm), pumped_vol(
				accum_vol_prm), ontime(ontime_prm), onsince(-1UL) {

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
	unsigned long vol;		 // Pump volume per round [cc]
	unsigned long actvol;   // Pump volume per this round  [cc]
	unsigned long interval;	// Time between two rounds [seconds]
	unsigned long elapsed;	// Time elapsed since start of last round [ms]

	elapsed = now - onsince;

	// Get pump ontime
	_ontime = ontime->get();

	// Get pump volume
	vol = (_ontime * flow->get() + 30) / 60;	// cc per one period. Term +30 to round instead of truncate.
	actvol = min(vol, tank_vol); 			// Limit volume to what's left in tank.
	actvol = min(actvol, rqst_vol->get());	// Limit volume to what's requested.

	if (actvol == 0) {
		// Zero volume to pump
		onsince = now;
		interval = -1UL;
		elapsed = 0;
		_ontime = 0;
	} else {
		// Flow and requested volume is > 0
		interval = (vol * 86400) / rqst_vol->get(); // Pump round interval.
		_ontime = (actvol * 60) / flow->get();      // Ontime per pump round. 
	}

	// Switch off pump after _ontime seconds
	if (isOn() && elapsed >= _ontime * 1000) {
		Serial.print("Turn off pin ");
		Serial.print(p_pin, DEC);
		Serial.print(", elapsed=");
		Serial.print(elapsed, DEC);

		// Turn off pump
		digitalWrite(p_pin, LOW);

		// Updated pumped volume
		pumped_vol->set(pumped_vol->get() + actvol);

		Serial.print("\nSave pumped...");
		pumped_vol->eepromSave();
		return;
	}

	// Delay activaion of pump if we should wait
	if (inhibit) {
		//Serial.print("w");
		return;
	}

	// Switch on pump interv_time seconds after last activation.
	if (elapsed >= interval * 1000) {
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

