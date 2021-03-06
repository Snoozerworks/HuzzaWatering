#ifndef Parameter_H_
#define Parameter_H_

#include "consts_and_types.h"

/**
 * Parameters are saved in a common structure type.
 *
 * Each parameter is defined by
 * - uint8	prmid    : Parameter id
 * - uint32	low      : Parameter lower bound
 * - uint32	high     : Parameter higher bound
 * - uint32	val      : Parameter value
 *
 */
class Parameter {
public:
	/*
	 *  True to upload parameter to server
	 */
	bool upload = false;

	/**
	 * Constructor. Value is initialized to the low limit.
	 *
	 * Initialize with parameter id as argument.
	 */
	Parameter(prmid_t id, unsigned long lo, unsigned long hi) :
			prm(id), //
			val(lo), //
			low(lo), //
			high(hi), //
			eeprom_pos(id * sizeof(unsigned long)) {
	}

	void set(unsigned long new_val);

	unsigned long get() const;

	void eepromSave() const;

	void eepromLoad();

private:
	const prmid_t prm; // Index in parameter array.
	unsigned long val; // Parameter value.
	const unsigned long low; // Upper bound.
	const unsigned long high; // Lower bound.
	const unsigned long eeprom_pos; // eeprom offset position

};

#endif
