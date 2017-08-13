// Do not remove the include below
#include "Parameter.h"

#include <EEPROM.h>

/**
 * Set and save to EEPROM parameter value if not outside bounds.
 *
 * @param int new_val
 */
void Parameter::set(unsigned long new_val) {
	val = max(min(new_val, high), low);
}

/**
 * Get parameter value
 */
unsigned long Parameter::get() const {
	return val;
}

/**
 * Save value to EEPROM. Call EEPROM.begin() first!
 */
void Parameter::eepromSave() const {
	EEPROM.write(eeprom_pos + 0, val >> 24);
	EEPROM.write(eeprom_pos + 1, val >> 16);
	EEPROM.write(eeprom_pos + 2, val >> 8);
	EEPROM.write(eeprom_pos + 3, val);
	EEPROM.commit();	// Commit writes
}

/**
 * Load value from EEPROM. Call EEPROM.begin() first!
 */
void Parameter::eepromLoad() {
	val = 0;
	val = EEPROM.read(eeprom_pos + 0) << 24;
	val |= EEPROM.read(eeprom_pos + 1) << 16;
	val |= EEPROM.read(eeprom_pos + 2) << 8;
	val |= EEPROM.read(eeprom_pos + 3);
	set(val);
}
