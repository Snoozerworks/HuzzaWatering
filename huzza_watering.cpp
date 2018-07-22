// Do not remove the include below
#include "huzza_watering.h"

#include <EEPROM.h>
#include "consts_and_types.h"
#include "MachineState.h"

MachineState M;
volatile bool manual_refresh;
unsigned long time_last_refresh;
unsigned long now;

void onSyncPinInterrupt() {
	// Set flag to refresh parameters from server
	manual_refresh = true;
}

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true); 	// On ESP8266, debug with serial
	EEPROM.begin(EEPROM_SIZE);
	delay(50);

	// Init values
	time_last_refresh = -1UL;
	manual_refresh = false;
	M.refresh.set(10000);

	// All parameters are initialized to its lower limit. Some should be
	// read back parameter from EEPROM though.
	M.pumped1.eepromLoad();
	M.pumped2.eepromLoad();
	M.pumped3.eepromLoad();

	// Setup gpio pins
	pinMode(A0, INPUT);
	pinMode(PINS::SYNC, INPUT);
	pinMode(PINS::MPX_EN, OUTPUT);
	pinMode(PINS::MPX_S0, OUTPUT);
	pinMode(PINS::MPX_S1, OUTPUT);
	pinMode(PINS::SERVO, OUTPUT);
	// Disable mutiplexer
	digitalWrite(PINS::MPX_EN, HIGH);
	// Servo signal to 0 volt
	digitalWrite(PINS::SERVO, LOW);

	// Connecting to a WiFi network
	Serial.print("\nConnecting ");
	WiFi.begin(WIFI::ssid, WIFI::password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(5000);
		Serial.print(".");
	}
	delay(100);
	Serial.print("\nWiFi connected. IP: ");
	Serial.println(WiFi.localIP());

	// Use pin PINS::SYNC as input to synchronize with server directly
	attachInterrupt(digitalPinToInterrupt(PINS::SYNC), onSyncPinInterrupt,
	FALLING);
}

void loop() {
	bool auto_refresh;
	now = millis();
	auto_refresh = now - time_last_refresh > M.refresh.get();

	if (auto_refresh || manual_refresh) {
		// Get parameters from server at intervals set by the refresh rate
		// or when input PINS:SYNC switch off.
		Serial.println("\nRefresh");

		manual_refresh = false;
		time_last_refresh = now;

		M.downloadFromServer();
		yield(); // Let the ESP8266 do its thing too
		M.uploadToServer();
	}

	yield(); // Let the ESP8266 do its thing too
	M.run(now);
}
