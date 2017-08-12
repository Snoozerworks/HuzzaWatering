// Do not remove the include below
#include "Parameter.h"

#include <EEPROM.h>

const unsigned int EEPROM_SIZE = 1024;

/**
 * Set and save to EEPROM parameter value if not outside bounds.
 *
 * @param int new_val
 */
void Parameter::set(unsigned long new_val) {
	val = _max(_min(new_val, high), low);
	//	Serial.print("Set prm ");
	//	Serial.print(prm);
	//	Serial.print(" at ");
	//	Serial.print(eeprom_pos);
	//	Serial.print(" len ");
	//	Serial.print(sizeof(val));
	//	Serial.print(" val ");
	//	Serial.println(val);
}

/**
 * Get parameter value
 */
unsigned long Parameter::get() const {
	return val;
}

/**
 * Save value to EEPROM.
 */
void Parameter::eepromSave() const {
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.write(eeprom_pos + 0, val >> 24);
	EEPROM.write(eeprom_pos + 1, val >> 16);
	EEPROM.write(eeprom_pos + 2, val >> 8);
	EEPROM.write(eeprom_pos + 3, val);
	EEPROM.commit();
}

/**
 * Load value from EEPROM.
 */
void Parameter::eepromLoad() {
	val = 0;
	val = EEPROM.read(eeprom_pos + 0) << 24;
	val |= EEPROM.read(eeprom_pos + 1) << 16;
	val |= EEPROM.read(eeprom_pos + 2) << 8;
	val |= EEPROM.read(eeprom_pos + 3);
	set(val);
}
