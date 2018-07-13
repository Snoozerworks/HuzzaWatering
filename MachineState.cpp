// Do not remove the include below
#include "MachineState.h"

#include <ESP8266HTTPClient.h>

/**
 * Constructor
 */
MachineState::MachineState() {
	params[PRM::P1_RQST_VOL] = &p1_rqst_vol;
	params[PRM::P2_RQST_VOL] = &p2_rqst_vol;
	params[PRM::P3_RQST_VOL] = &p3_rqst_vol;

	params[PRM::P1_FLOW] = &p1_flow;
	params[PRM::P2_FLOW] = &p2_flow;
	params[PRM::P3_FLOW] = &p3_flow;

	params[PRM::P1_PUMPED_VOL] = &pumped1;
	params[PRM::P2_PUMPED_VOL] = &pumped2;
	params[PRM::P3_PUMPED_VOL] = &pumped3;

	params[PRM::TANK_SIZE] = &tanksize;
	params[PRM::ONTIME] = &ontime;
	params[PRM::REFRESH_RATE] = &refresh;

	params[PRM::ADC1] = &adc1;
	params[PRM::ADC2] = &adc2;
	params[PRM::ADC3] = &adc3;
	params[PRM::ADC4] = &adc4;

	params[PRM::LAST_ERR] = &last_err;
}

/**
 * Parse stream for parameter values to get. Returns true on success.
 * Each byte in the stream should correspond to the parameter id to
 * get. An ending parameter value of NONE ends the request for parameters.
 * Method sets the upload flag of parameters to send.
 */
bool MachineState::parseParamGetRequest(WiFiClient * const stream) {
	prmid_t prm_id;

	if (stream == nullptr) {
		reportFault(ERR::NULLPTR_ERR, "");
		return false;
	}

	Serial.print("\tPrm=");
	while (stream->available()) {
		// Extract parameter
		prm_id = stream->read();
		Serial.print(prm_id, DEC);
		Serial.print(' ');

		if (prm_id >= PRM::_END) {
			// Unknown command
			reportFault(ERR::PARAMID_GET_ERR, "Prm " + String(prm_id));
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
	int stream_data;
	prmid_t prm_id;
	byte msg_buffer[4];

	if (stream == nullptr) {
		reportFault(ERR::NULLPTR_ERR, "");
		return false;
	}

	Serial.print("\t(Prm,Val)=");
	while (stream->available()) {
		// Extract parameter
		stream_data = stream->read();

		if (stream_data < 0) {
			reportFault(99, "Nodata? ");
			continue;
		}

		prm_id = (prmid_t) stream_data;

		if (prm_id >= PRM::_END) {
			// Unknown parameter
			reportFault(ERR::PARAMID_SET_ERR, "Prm " + String(prm_id));
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

		(params[prm_id])->set(val);

		Serial.print("(");
		Serial.print(prm_id, DEC);
		Serial.print(',');
		Serial.print(val, DEC);
		Serial.print(",");
		Serial.print((params[prm_id])->get(), DEC);
		Serial.print(") ");

		delay(1);
	}
	return false;
}

/**
 * Upload flagged parameters to server.
 *
 * Parameters are sent as a byte stream like;
 * - 3 bytes; ESP8266 chip id.
 * - 1 byte; Command CMD::SET
 * - 5xN bytes; In sequence b0 to b4. Byte b0 is the parameter id followed by
 *   its value b1 (MSB) to b4.
 * - 1 byte; Command CMD::NONE
 */
void MachineState::uploadToServer() {
	const unsigned short content_lenght = (PRM::_END - 1) * 5 + 5;
	byte buffer[content_lenght];
	bool resetLastErr = false;
	unsigned long val;
	unsigned short byteno;
	HTTPClient http;

	Serial.print("\nUpload");

	http.setTimeout(WIFI::WIFI_RX_TIMEOUT);
	http.begin(WIFI::upload_url);

	byteno = 0;

	// Add chip id to data buffer
	val = ESP.getChipId();
	buffer[byteno] = (byte) (val >> 16);
	buffer[++byteno] = (byte) (val >> 8);
	buffer[++byteno] = (byte) val;

	// Add command to data buffer
	buffer[++byteno] = CMD::SET;

	// Add parameters to data buffer
	for (byte k = 0; k < PRM::_END; k++) {
		if (params[k] && params[k]->upload) {

			if (k == PRM::LAST_ERR) {
				// Last error code should be reset if successfully uploaded.
				resetLastErr = true;
			}

			readADC(k); 	// Read ADC values (only for related parameters)

			params[k]->upload = false;

			val = params[k]->get();

			buffer[++byteno] = k;
			buffer[++byteno] = (byte) (val >> 24);
			buffer[++byteno] = (byte) (val >> 16);
			buffer[++byteno] = (byte) (val >> 8);
			buffer[++byteno] = (byte) val;
		}
	}
	buffer[++byteno] = (byte) PRM::NONE;

	++byteno;
	if (byteno > sizeof(buffer)) {
		reportFault(ERR::BUFFER_OVERRUN, "");
		//Serial.print("\n**buffer overrun");
		byteno = sizeof(buffer);
	}

	// Send parameters if there are at least 5 bytes in the buffer.
	if (byteno > 4) {
		int http_code = http.POST((uint8_t *) buffer, (size_t) byteno);

		Serial.print("\nHttp code: ");
		Serial.print(http_code, DEC);
		Serial.print(", bytes sent: ");
		Serial.print(byteno, DEC);

		// Check server response
		if (http_code != HTTP_CODE_OK) {
			Serial.print(", **failed");

		} else {
			// Reset last error if uploadad
			if (resetLastErr) {
				reportFault(ERR::NOERR, "");
			}

			// Print response
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
	int stream_data;
	int http_code;
	cmdid_t cmd_id;

	Serial.print("\nDownload ");

	HTTPClient http;
	http.setTimeout(WIFI::WIFI_RX_TIMEOUT);

	// Include chip id in url query
	String url = String(WIFI::download_url);
	url += "?cid=";
	url += String(ESP.getChipId(), HEX);
	http.begin(url);

	http_code = http.GET();
	Serial.print("\nHttp code: ");
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
	if (stream == nullptr) {
		reportFault(ERR::NULLPTR_ERR, "");
		return;
	}

	while (stream->available()) {

		// Retrive command
		stream_data = stream->read();
		if (stream_data < 0) {
			reportFault(ERR::EMPTY_INSTREAM, "Nodata? ");
			continue;
		}
		cmd_id = (cmdid_t) stream_data;
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
			printErrorStream(stream);
			break;

		}
		delay(2);
		yield();
	}

	stream->stop();
	http.end();
	Serial.println("\nDone download");
}

/**
 * Call to update state of pumps
 */
void MachineState::run(unsigned long now) {
	// Run the pumps.
	yield(); // Let the ESP8266 do its thing too
	p1.run(now, tankVolume(), p2.isOn() || p3.isOn());
	yield(); // Let the ESP8266 do its thing too
	p2.run(now, tankVolume(), p3.isOn() || p1.isOn());
	yield(); // Let the ESP8266 do its thing too
	p3.run(now, tankVolume(), p1.isOn() || p2.isOn());
}

/**
 * Read ADC analogue inputs for parameter pid. If pid is not
 * PRM:ADC1..4 nothing is read.
 *
 * @param pid Parameter id.
 */
void MachineState::readADC(prmid_t pid) {
	unsigned int val;
	byte k;

	// Select mutiplexer input
	switch (pid) {
	case PRM::ADC1:
		digitalWrite(PINS::MPX_S0, LOW);
		digitalWrite(PINS::MPX_S1, LOW);
		break;
	case PRM::ADC2:
		digitalWrite(PINS::MPX_S0, HIGH);
		digitalWrite(PINS::MPX_S1, LOW);
		break;
	case PRM::ADC3:
		digitalWrite(PINS::MPX_S0, LOW);
		digitalWrite(PINS::MPX_S1, HIGH);
		break;
	case PRM::ADC4:
		digitalWrite(PINS::MPX_S0, HIGH);
		digitalWrite(PINS::MPX_S1, HIGH);
		break;
	default:
		return; // pid is not a ADC parameter
	}

	// Enable mutiplexer
	digitalWrite(PINS::MPX_EN, LOW);
	// Let analogue value settle (not sure if needed)
	delay(10);
	// Read and save voltage in parameter. Average of 20 values.
	val = 0;
	k = -1;
	while (++k < 20) {
		val += analogRead(A0);
	}
	val = val / 20;

	params[pid]->set(val);
	Serial.print("\nADC ");
	Serial.print(pid, DEC);
	Serial.print("=");
	Serial.print(params[pid]->get(), DEC);

	// Disable mutiplexer
	digitalWrite(PINS::MPX_EN, HIGH);
}

/**
 * Returns volume left in tank .
 */
unsigned long MachineState::tankVolume() {
	unsigned long pumped = pumped1.get() + pumped2.get() + pumped3.get();
	unsigned long tsize = tanksize.get();
	return (pumped < tsize) ? tsize - pumped : 0;
}

/**
 * Serial print up to 150 bytes from stream for debugging. Flush the stream when
 * done.
 */
void MachineState::printErrorStream(WiFiClient * const stream) {
	int stream_data;
	byte max_count = 150;

	if (stream == nullptr) {
		reportFault(ERR::NULLPTR_ERR, "");
		return;
	}

	Serial.print("\nBad data=");
	while (stream->available() && --max_count > 0) {
		// Extract parameter
		stream_data = stream->read();
		if (stream_data < 0) {
			Serial.print(stream_data);
			break;
		}
		Serial.print("x");
		Serial.print((byte) stream_data, HEX);
		Serial.print(' ');
		yield();
	}

	stream->flush();
}

/**
 * For debugging, prints error messages to serial and updates the last_err
 * parameter.
 *
 */
void MachineState::reportFault(byte err, String err_msg) {
	last_err.set(err);
	if (Serial) {
		Serial.print("\n**Err ");
		Serial.print(err, HEX);		// Error code
		Serial.print(" : ");
		Serial.println(err_msg);		// Error message
	}
}
