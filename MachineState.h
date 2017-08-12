#ifndef MachineState_H_
#define MachineState_H_

#include <ESP8266WiFi.h>
#include "consts_and_types.h"
#include "Parameter.h"
#include "Pump.h"

class MachineState {

public:
	//
	// Define parameters with parameter id, min value and max value
	//
	Parameter p1_rqst_vol { PRM::P1_RQST_VOL, 0, 1000000UL }; // Requested flow in cc per day. Max 500 cc
	Parameter p2_rqst_vol { PRM::P2_RQST_VOL, 0, 1000000UL }; // Requested flow in cc per day. Max 500 cc
	Parameter p3_rqst_vol { PRM::P3_RQST_VOL, 0, 1000000UL }; // Requested flow in cc per day. Max 500 cc

	Parameter p1_flow { PRM::P1_FLOW, 1, 1000UL }; // Pump flow capacity cc/min
	Parameter p2_flow { PRM::P2_FLOW, 1, 1000UL }; // Pump flow capacity cc/min
	Parameter p3_flow { PRM::P3_FLOW, 1, 1000UL }; // Pump flow capacity cc/min

	Parameter pumped1 { PRM::P1_PUMPED_VOL, 0, -1UL }; // Pumped volume in cc
	Parameter pumped2 { PRM::P2_PUMPED_VOL, 0, -1UL }; // Pumped volume in cc
	Parameter pumped3 { PRM::P3_PUMPED_VOL, 0, -1UL }; // Pumped volume in cc

	Parameter tanksize { PRM::TANK_SIZE, 0, -1UL }; // Tank volume in cc
	Parameter ontime { PRM::ONTIME, 0, -1UL }; // Pump ontime per round in seconds
	Parameter refresh { PRM::REFRESH_RATE, 0, -1UL }; // Server connection interval in milliseconds

	Parameter adc1 { PRM::ADC1, 0, 1023UL }; // ADC1 value
	Parameter adc2 { PRM::ADC2, 0, 1023UL }; // ADC2 value
	Parameter adc3 { PRM::ADC3, 0, 1023UL }; // ADC3 value
	Parameter adc4 { PRM::ADC4, 0, 1023UL }; // ADC4 value

//	Pump p1 { PINS::PUMP1, &p1_flow, &p1_rqst_vol, &pumped1, &ontime }; // Pump 1
//	Pump p2 { PINS::PUMP2, &p2_flow, &p2_rqst_vol, &pumped2, &ontime }; // Pump 2
//	Pump p3 { PINS::PUMP3, &p3_flow, &p3_rqst_vol, &pumped3, &ontime }; // Pump 3
	Pump *p1;
	Pump *p2;
	Pump *p3;

	MachineState();

	bool parseParamGetRequest(WiFiClient * const stream);

	bool parseParamSetRequest(WiFiClient * const stream);

	void uploadToServer();

	void downloadFromServer();

	void run(unsigned long now);

private:
	Parameter* params[PRM::_END];

	void readADC(prmid_t pid);

	unsigned long tankVolume();

	static void printErrorStream(WiFiClient * const stream);

	static void reportFault(byte err, String err_msg);
};

#endif
