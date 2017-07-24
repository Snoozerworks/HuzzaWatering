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
	Parameter p1_rqst_vol { PRM::P1_RQST_VOL, 0, -1UL, 0 }; // Pump flow in cc per day. Max 500 cc
	Parameter p2_rqst_vol { PRM::P2_RQST_VOL, 0, -1UL, 0 }; // Pump flow in cc per day. Max 500 cc
	Parameter p3_rqst_vol { PRM::P3_RQST_VOL, 0, -1UL, 0 }; // Pump flow in cc per day. Max 500 cc

	Parameter p1_flow { PRM::P1_FLOW, 0, 1000, 1000 }; // Pump flow capacity cc/min
	Parameter p2_flow { PRM::P2_FLOW, 0, 1000, 1000 }; // Pump flow capacity cc/min
	Parameter p3_flow { PRM::P3_FLOW, 0, 1000, 1000 }; // Pump flow capacity cc/min

	Parameter pumped1 { PRM::P1_PUMPED_VOL, 0, -1UL, 0 }; // Pumped volume in cc
	Parameter pumped2 { PRM::P2_PUMPED_VOL, 0, -1UL, 0 }; // Pumped volume in cc
	Parameter pumped3 { PRM::P3_PUMPED_VOL, 0, -1UL, 0 }; // Pumped volume in cc

	Parameter tankvol { PRM::TANK_VOL, 0, -1UL, 0 }; // Tank volume in cm^3
	Parameter run_interval { PRM::RUN_INTERVAL, 0, -1UL, 3600 }; // Pump run interval in seconds
	Parameter refresh { PRM::REFRESH_RATE, 0, -1UL, 10000 }; // Server connection interval in milliseconds

	Parameter adc1 { PRM::ADC1, 0, 1023, 0 }; // ADC1 value
	Parameter adc2 { PRM::ADC2, 0, 1023, 0 }; // ADC2 value
	Parameter adc3 { PRM::ADC3, 0, 1023, 0 }; // ADC3 value
	Parameter adc4 { PRM::ADC4, 0, 1023, 0 }; // ADC4 value

	Pump p1 { PINS::PUMP1, &p1_flow, &p1_rqst_vol, &pumped1, &run_interval };
	Pump p2 { PINS::PUMP2, &p2_flow, &p2_rqst_vol, &pumped2, &run_interval };
	Pump p3 { PINS::PUMP3, &p3_flow, &p3_rqst_vol, &pumped3, &run_interval };

	MachineState();

	bool parseParamGetRequest(WiFiClient * const stream);

	bool parseParamSetRequest(WiFiClient * const stream);

	void uploadToServer();

	void downloadFromServer();

	void run(unsigned long now);

private:
	Parameter* params[PRM::_END];

	void readADC(prmid_t pid);

	void reportFault(byte err, String err_msg);
};

#endif
