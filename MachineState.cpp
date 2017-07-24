// Do not remove the include below
#include "MachineState.h"

#include <ESP8266HTTPClient.h>

/**
 * Constructor
 */
MachineState::MachineState() {
	params[PRM::P1_FLOW] = &p1_flow;
	params[PRM::P2_FLOW] = &p2_flow;
	params[PRM::P3_FLOW] = &p3_flow;

	params[PRM::P1_RQST_VOL] = &p1_rqst_vol;
	params[PRM::P2_RQST_VOL] = &p2_rqst_vol;
	params[PRM::P3_RQST_VOL] = &p3_rqst_vol;

	params[PRM::P1_PUMPED_VOL] = &pumped1;
	params[PRM::P2_PUMPED_VOL] = &pumped2;
	params[PRM::P3_PUMPED_VOL] = &pumped3;

	params[PRM::TANK_VOL] = &tankvol;
	params[PRM::RUN_INTERVAL] = &run_interval;
	params[PRM::REFRESH_RATE] = &refresh;

	params[PRM::ADC1] = &adc1;
	params[PRM::ADC2] = &adc2;
	params[PRM::ADC3] = &adc3;
	params[PRM::ADC4] = &adc4;
}

/**
 * Parse stream for parameter values to get. Returns true on success.
 * Each byte in the stream should correspond to the parameter id to
 * get. An ending parameter value of NONE ends the request for parameters.
 * Method sets the upoad flag of the parameter to send.
 */
bool MachineState::parseParamGetRequest(WiFiClient * const stream) {
	prmid_t prm_id;

	Serial.print("\tPrm=");
	while (stream->available()) {
		// Extract parameter
		prm_id = stream->read();
		Serial.print(prm_id, DEC);
		Serial.print(' ');

		if (prm_id >= PRM::_END) {
			// Unknown command
			reportFault(ERR::PARAMID_GET_ERR, "param " + String(prm_id));
			break;
		}

		// Stop when hitting NONE parameter
		if (prm_id == PRM::NONE) {
			return true;
		}

		// Flag to upload
		params[prm_id]->upload = true;
	}
	return false;
}

/**
 * Parse stream for parameter values to set. Returns true on success.
 * The stream should be repetitions of 5 bytes like PABCD and ending
 * with a byte equal to parameter id NONE.
 * Byte P is the parameter id and bytes A to D the parameter value.
 * Byte a is the most significant byte of the value.
 */
bool MachineState::parseParamSetRequest(WiFiClient * const stream) {
	unsigned int read_length;
	unsigned long val;
	prmid_t prm_id;
	byte msg_buffer[4];

	Serial.print("\t(Prm,Val)=");
	while (stream->available()) {
		// Extract parameter
		prm_id = stream->read();

		if (prm_id >= PRM::_END) {
			// Unknown parameter
			reportFault(ERR::PARAMID_SET_ERR, "param " + String(prm_id));
			break;
		}

		// Stop at the NONE parameter
		if (prm_id == PRM::NONE) {
			return true;
		}

		// Retrive the parameter value
		read_length = stream->readBytes(msg_buffer, sizeof(msg_buffer));
		if (read_length == 0) {
			// Failed to get message id and length.
			reportFault(ERR::RESP_TIMEOUT_ERR, "");
			break;
		}

		if (read_length < sizeof(msg_buffer)) {
			// Failed to get message id and length.
			reportFault(ERR::PARAMVAL_SET_ERR, "param " + String(prm_id));
			break;
		}

		// Pack bytes together to get the value
		val = msg_buffer[0] << 24;
		val += msg_buffer[1] << 16;
		val += msg_buffer[2] << 8;
		val += msg_buffer[3];


		Serial.print('(');
		Serial.print(prm_id, DEC);
		Serial.print(',');
		Serial.print(val, DEC);
		Serial.print(") ");

		params[prm_id]->set(val);

		delay(1);
	}
	return false;
}

/**
 * Upload flagged parameters to server.
 *
 * Parameters are sent as a byte stream like;
 * - first byte is the command CMD::SET
 * - Then in sequences of five bytes b0 to b4. Byte b0 is the parameter
 * id followed by its value b1 (MSB) to b4.
 * - last byte is the command CMD::NONE
 */
void MachineState::uploadToServer() {
	const unsigned short content_lenght = (PRM::_END - 1) * 5 + 2;
	byte buffer[content_lenght];
	unsigned long val;
	unsigned short byteno;
	HTTPClient http;

	Serial.print("Upload");

	http.setTimeout(WIFI::WIFI_RX_TIMEOUT);
	http.begin(WIFI::upload_url);

	// Prepare data buffer
	byteno = 0;
	buffer[byteno] = CMD::SET;
	for (byte k = 0; k < PRM::_END; k++) {
		if (params[k] && params[k]->upload) {

			readADC(k); 	// Read ADC values (only for related parameters)

			params[k]->upload = false;

			val = params[k]->get();

			buffer[++byteno] = k;
			//Serial.print("\t Upload val");

			buffer[++byteno] = (byte) (val >> 24);
			buffer[++byteno] = (byte) (val >> 16);
			buffer[++byteno] = (byte) (val >> 8);
			buffer[++byteno] = (byte) val;

			//Serial.print("\tHEX=");
			//Serial.print(k, HEX);
			//Serial.print(' ');
			//Serial.print(buffer[byteno - 3], HEX);
			//Serial.print(' ');
			//Serial.print(buffer[byteno - 2], HEX);
			//Serial.print(' ');
			//Serial.print(buffer[byteno - 1], HEX);
			//Serial.print(' ');
			//Serial.print(buffer[byteno], HEX);
			//
			//Serial.print("\tPrm=");
			//Serial.print(k, DEC);
			//Serial.print("\tval=");
			//Serial.print(val, DEC);
		}
	}
	buffer[++byteno] = (byte) PRM::NONE;

	++byteno;
	if (byteno > sizeof(buffer)) {
		Serial.print(" **buffer overrun");
		byteno = sizeof(buffer);
	}

	// No parameters to be sent if not more than 2 bytes
	if (byteno > 2) {
		int http_code = http.POST((uint8_t *) buffer, (size_t) byteno);

		Serial.print(", http code: ");
		Serial.print(http_code, DEC);
		Serial.print(", bytes sent: ");
		Serial.print(byteno, DEC);

		// Check server response
		if (http_code != HTTP_CODE_OK) {
			Serial.print(", **failed");

		} else {
			String response_str;
			WiFiClient client = http.getStream();
			while (client.available()) {
				response_str = client.readStringUntil('\n');
				Serial.println(response_str);
			}

		}

	}

	Serial.println("\nDone upload");
	http.end();

}

/**
 * Download parameters from server
 */
void MachineState::downloadFromServer() {
	cmdid_t cmd_id;

	Serial.print("Download");

	HTTPClient http;
	http.setTimeout(WIFI::WIFI_RX_TIMEOUT);
	http.begin(WIFI::download_url);

	int http_code = http.GET();
	Serial.print(", http code: ");
	Serial.print(http_code, DEC);

	// file found at server
	if (http_code != HTTP_CODE_OK) {
		Serial.print(", **failed");
		http.end();
		return;
	}

	//
	// Start of message data
	//

	WiFiClient * stream = http.getStreamPtr();

	bool loop = true;
	while (stream->available() && loop) {

		// Retrive command
		cmd_id = stream->read();
		Serial.print("\nCmd ");
		Serial.print(cmd_id, DEC);

		// Parse data for command
		if (cmd_id == CMD::SET) {
			if (!parseParamSetRequest(stream)) {
				break;
			}

		} else if (cmd_id == CMD::GET) {
			if (!parseParamGetRequest(stream)) {
				break;
			}

		} else if (cmd_id == CMD::NONE) {
			break;

		} else {
			reportFault(ERR::BAD_COMMAND_ERR, "cmd " + String(cmd_id));
			break;

		}

		delay(1);
	}

	Serial.println("\nDone download");
	stream->stop();
	http.end();
}

/**
 * Call to update state of pumps
 */
void MachineState::run(unsigned long now) {
	unsigned long pumped_vol;
	unsigned int tank_vol_left;

	// Get total volume pumped by all pumps
	pumped_vol = pumped1.get() + pumped2.get() + pumped3.get();

	// Remaining tank volume
	tank_vol_left = tankvol.get() - pumped_vol;

	// Run the pumps.
	p1.run(now, tank_vol_left, p2.isOn() || p3.isOn());
	p2.run(now, tank_vol_left, p3.isOn() || p1.isOn());
	p3.run(now, tank_vol_left, p1.isOn() || p2.isOn());

}

/**
 * Read ADC analogue inputs for parameter pid. If pid is not
 * PRM:ADC1..4 nothing is read.
 *
 * @param pid Parameter id.
 */
void MachineState::readADC(prmid_t pid) {

	switch (pid) {
	case PRM::ADC1:
	case PRM::ADC2:
	case PRM::ADC3:
	case PRM::ADC4:
		digitalWrite(PINS::MPX_EN, LOW); 	// Enable mutiplexer
		break;
	default:
		return; // pid is not a ADC parameter
	}

	// Mutiplexer S0 low for ADC 1 and 3
	switch (pid) {
	case PRM::ADC1:
	case PRM::ADC3:
		digitalWrite(PINS::MPX_S0, LOW);
		break;
	default:
		digitalWrite(PINS::MPX_S0, HIGH);
	}

	// Mutiplexer S1 low for ADC 1 and 2
	switch (pid) {
	case PRM::ADC1:
	case PRM::ADC2:
		digitalWrite(PINS::MPX_S1, LOW);
		break;
	default:
		digitalWrite(PINS::MPX_S1, HIGH);
	}

	delay(10);			// Let analogue value settle (not sure if needed)
	params[pid]->set(analogRead(A0)); // Read and save voltage in parameter
	digitalWrite(PINS::MPX_EN, HIGH); 	// Disable mutiplexer
}

/**
 * For debugging, prints error messages to serial is present.
 */
void MachineState::reportFault(byte err, String err_msg) {
	if (Serial) {
		Serial.println();
		Serial.print("**Err ");
		Serial.print(err, DEC);		// Error code
		Serial.print(" : ");
		Serial.println(err_msg);		// Error message
	}
}
