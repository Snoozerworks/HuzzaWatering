#ifndef Parameter_H_
#define Parameter_H_

#include "consts_and_types.h"

/**
 * Parameters are saved in a common structure type.
 *
 * Each parameter is defined by
 * - uint8	prmid    : Parameter id
 * - uint32	val      : Parameter value
 * - uint32	low      : Parameter lower bound
 * - uint32	high     : Parameter higher bound
 *
 */
class Parameter {
public:
	/*
	 *  True to upload parameter to server
	 */
	bool upload = false;

	/**
	 * Constructor
	 *
	 * Initialize with parameter id as argument.
	 */
	Parameter(prmid_t id, unsigned long lo, unsigned long hi,
			unsigned long init_val) :
			prm(id), //
			low(lo), //
			high(hi), //
			eeprom_pos(id * sizeof(init_val)), //
			val(init_val) {
	}

	void set(unsigned long new_val);

	unsigned long get();

	void eepromSave();

	void eepromLoad();

private:
	prmid_t prm; // Index in parameter array.
	const unsigned long low; // Upper bound.
	const unsigned long high; // Lower bound.
	const unsigned long eeprom_pos; // eeprom offset position
	unsigned long val; // Parameter value.

};

#endif