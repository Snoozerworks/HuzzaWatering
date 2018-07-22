// Do not remove the include below
#include "Pump.h"

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
void Pump::run(unsigned long now, unsigned int tank_vol, bool inhibit) {
	unsigned long elapsed_s;	// Time elapsed since last start of pump [ms]
	unsigned int delivered_vol; // Delivered pump volume this round  [cc]
	unsigned int v_accum;		// Accumulated need to pump [cc].
	unsigned int v_round;		// Pumping volume per round [cc].

	elapsed_s = (now - last_switch_on) / 1000;

	if (isOn()) {
		// Pump is started...

		// Stop pump if it is inhibited, tank is empty or round volume reached.
		if (inhibit || tank_vol == 0 || elapsed_s > runtime) {
			Serial.print("Turn off pin ");
			Serial.print(p_pin, DEC);
			Serial.print(", elapsed [ms]=");
			Serial.print(elapsed_s, DEC);

			// Turn off pump
			digitalWrite(p_pin, LOW);

			// Update pumped volume
			delivered_vol = (elapsed_s * flow_capacity->get()) / 60;
			pumped_vol->set(pumped_vol->get() + delivered_vol);
			pumped_vol->eepromSave();
		}

	} else {
		// Pump is stopped...

		// Don't start the pump if it is inhibited or tank is empty.
		if (inhibit || tank_vol == 0) {
			return;
		}

		v_accum = (elapsed_s * flow_request->get()) / 86400;
		v_round = (round_runtime->get() * flow_capacity->get()) / 60;

		// Start pump if not inhibited, accumulated need exceeds the round
		// volume and tank is not empty.
		if (v_accum > v_round) {
			Serial.print("\nTurn on pin ");
			Serial.print(p_pin, DEC);
			Serial.print(", ontime=");
			Serial.print(runtime, DEC);
			Serial.print(", vol=");
			Serial.print(v_accum, DEC);

			// Turn on pump
			digitalWrite(p_pin, HIGH);

			// Updated time of last pump start.
			last_switch_on = now;

			// Update runtime
			runtime = getPumpTime(v_accum);
		}

	}

}

/***************
 * Private
 ***************/

/**
 * Returns time [s] required for pump to deliver a volume v [cc].
 *
 * @param v Volume in cc.
 */
unsigned int Pump::getPumpTime(unsigned int vol) const {
	return (vol * 60) / flow_capacity->get();
}
