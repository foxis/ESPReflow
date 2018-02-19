#include <EasyOTA.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <max6675.h>
#include <SPI.h>

#include "wificonfig.h"

#define thermoDO 13 // D7
#define thermoCS 12 // D6
#define thermoCLK 14 // D5
#define relay 4 // D2

EasyOTA OTA(ARDUINO_HOSTNAME);
MAX6675 thermocouple;

void setup() {
	Serial.begin(115200);

	OTA.addAP(WIFI_SSID, WIFI_PASSWORD);
	//SPIFFS.begin();
	thermocouple.begin(thermoCLK, thermoCS, thermoDO);

	pinMode(relay, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

}

void loop() {
	unsigned long now = millis();
	static unsigned long last_m = millis();
	static unsigned long last_m1 = last_m;
	static bool on = false;

  OTA.loop(now);

	if (now - last_m > 1000)
	{
		Serial.print("C = ");
		Serial.println(thermocouple.readCelsius());
		Serial.print("F = ");
		Serial.println(thermocouple.readFahrenheit());

		last_m = now;
	}

	if (now - last_m1 > 10000)
	{
		digitalWrite(relay, on);
		digitalWrite(LED_BUILTIN, on);

		on = !on;
		last_m1 = now;
	}
}
