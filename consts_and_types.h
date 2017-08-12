#ifndef HUZZA_WATERING_CONSTS_AND_TYPES_H_
#define HUZZA_WATERING_CONSTS_AND_TYPES_H_

#include "Arduino.h"

/*******************************************
 * Type definitions
 *******************************************/

/**
 * Command id
 */
typedef byte cmdid_t;

/**
 * Paramater id
 */
typedef byte prmid_t;

/**
 * Action id
 */
typedef byte actid_t;

/*******************************************
 * Keep constants in separate namespaces
 *******************************************/

namespace PINS {
// Analogue pin is not specified here since there is only one choice: A0.
const byte SYNC = 13;	// Digital in marked "din"
const byte PUMP1 = 12;	// PWM pump 1
const byte PUMP2 = 14;	// PWM pump 2
const byte PUMP3 = 15;	// PWM pump 3
const byte SERVO = 5; 	// PWM servo 3,3V signal
const byte MPX_EN = 0; 	// Mutiplexor enable
const byte MPX_S0 = 2; 	// Mutiplexor S0
const byte MPX_S1 = 4; 	// Mutiplexor S1
}

namespace PRM {
// Indentifiers for parameters which can be set or get
const prmid_t NONE = 0x00;
const prmid_t P1_FLOW = 0x01;
const prmid_t P2_FLOW = 0x02;
const prmid_t P3_FLOW = 0x03;
const prmid_t P1_RQST_VOL = 0x04;
const prmid_t P2_RQST_VOL = 0x05;
const prmid_t P3_RQST_VOL = 0x06;
const prmid_t P1_PUMPED_VOL = 0x07;
const prmid_t P2_PUMPED_VOL = 0x08;
const prmid_t P3_PUMPED_VOL = 0x09;
const prmid_t TANK_SIZE = 0x0A;
const prmid_t ONTIME = 0x0B;
const prmid_t REFRESH_RATE = 0x0C;
const prmid_t ADC1 = 0x0D;
const prmid_t ADC2 = 0x0E;
const prmid_t ADC3 = 0x0F;
const prmid_t ADC4 = 0x10;
const prmid_t _END = 0x11;
}

namespace WIFI {
// Constants for wifi connection
char const * const ssid = "Gris";
char const* const password = "isterband";
char const * const host = "www.skarmflyg.org";
char const * const download_url =
		"http://www.skarmflyg.org/_temp/HuzzaWatering/download.php";
char const * const upload_url =
		"http://www.skarmflyg.org/_temp/HuzzaWatering/upload.php";
const unsigned int WIFI_RX_TIMEOUT = 5000;	// 10 seconds
const byte http_port = 80;
}

namespace CMD {
// Identifiers for commands which can be interpreted
const cmdid_t NONE = 0x00;
const cmdid_t GET = 0x01;
const cmdid_t SET = 0x02;
const cmdid_t _END = 0x03;
}

namespace ERR {
const byte NOERR = 0x00;
const byte CONN_ERR = 0x01;
const byte RESP_HTTP_ERR = 0x03;
const byte RESP_TIMEOUT_ERR = 0x04;
const byte RESP_MALFORMED_ERR = 0x05;
const byte BAD_COMMAND_ERR = 0x06;
const byte PARAMID_SET_ERR = 0x07;
const byte PARAMVAL_SET_ERR = 0x08;
const byte PARAMID_GET_ERR = 0x09;
const byte NULLPTR_ERR = 0x0A;
const byte _END = 0x0B;

const actid_t NONE = 0x00;
const actid_t UPLOAD = 0x01;
const actid_t DOWNLOAD = 0x02;
}

#endif /* HUZZA_WATERING_CONSTS_AND_TYPES_H_ */
