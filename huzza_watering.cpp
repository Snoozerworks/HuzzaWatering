// Do not remove the include below
#include "huzza_watering.h"

#include "consts_and_types.h"
#include "MachineState.h"

MachineState M;
bool manual_refresh;
unsigned long time_last_refresh = -1;

void onSyncPinInterrupt() {
	// Set flag to refresh parameters from server
	manual_refresh = true;
}

void setup() {
	// Setup gpio pins
	pinMode(A0, INPUT);

	pinMode(PINS::SYNC, INPUT);
	pinMode(PINS::MPX_EN, OUTPUT);
	pinMode(PINS::MPX_S0, OUTPUT);
	pinMode(PINS::MPX_S1, OUTPUT);
	pinMode(PINS::SERVO, OUTPUT);

	digitalWrite(PINS::MPX_EN, HIGH); 	// Disable mutiplexer
	digitalWrite(PINS::SERVO, LOW);		// Servo signal to 0 volt

	manual_refresh = false;

	Serial.begin(115200);
	delay(100);
	Serial.setDebugOutput(true); 	// On ESP8266, debug with serial

	// Set some default values
	//	M.tankvol.set(0);			// Initiate to empty tank.
	//	M.run_interval.set(3600); 	// Interval in seconds between pump activations
	//	M.refresh.set(10000);		// 10 seconds interval for server connections.

	// Connecting to a WiFi network
	Serial.println();
	Serial.println("Connecting ");
	WiFi.begin(WIFI::ssid, WIFI::password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(10000);
		Serial.print(".");
	}
	delay(1000);
	Serial.println();
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	// Use pin PINS::SYNC as input to synchronize with server directly
	attachInterrupt(PINS::SYNC, onSyncPinInterrupt, FALLING);
}

void loop() {
	unsigned long now = millis();

	if ((now - time_last_refresh > M.refresh.get()) || manual_refresh) {
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
