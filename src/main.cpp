#include <EasyOTA.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "wificonfig.h"
#include "ReflowController_v1.h"
#include <ArduinoJson.h>
#include "AsyncJson.h"


EasyOTA OTA(ARDUINO_HOSTNAME);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/event");
ControllerBase * controller = NULL;
ControllerBase * last_controller = NULL;
AsyncWebSocketClient * _client = NULL;

void textThem(String& text) {
	if (_client) {
		_client->text(text);
		return;
	}
  int tryId = 0;
  for (int count = 0; count < ws.count();) {
    if (ws.hasClient(tryId)) {
      ws.client(tryId)->text(text);
      count++;
    }
    tryId++;
  }
}

void textThem(JsonObject &root) {
	String json;
	root.printTo(json);

	textThem(json);
}

void send_reading(float reading, float time, AsyncWebSocketClient * client, bool reset)
{
	Serial.println("Sending readings...");
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();

	JsonObject& data = root.createNestedObject("readings");
	data["temperature"] = reading;
	data["time"] = time;
	data["reset"] = reset;

	textThem(root);
}

void setupController(ControllerBase * c)
{
	ControllerBase * tmp = controller;
	controller = NULL;

	// report messages
	c->onMessage([](const String & msg) {
		Serial.println("Message: " + msg);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["message"] = msg;
		textThem(root);
	});

	c->onHeater([](bool heater) {
		Serial.println("Heater: " + String(heater));
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["heater"] = heater;
		textThem(root);
	});

	// report readings
	c->onMeasure([](const std::vector<float>& readings, unsigned long now){
		send_reading(readings[readings.size() - 1], (now - controller->start_time())/1000.0, NULL, readings.size() == 1);
	});

	// report mode change
	c->onMode([](ControllerBase::MODE_t last, ControllerBase::MODE_t current){
		Serial.println("Change mode: from " + String(last) + " to " + String(current));
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["mode"] = controller->translate_mode(current);

		textThem(root);
	});
	c->onStage([](const String& stage){
		Serial.println("Reflow stage: " + stage);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["stage"] = stage;
		textThem(root);
	});

	last_controller = tmp;
	controller = c;
}

void send_data(AsyncWebSocketClient * client)
{
	Serial.println("Sending all data...");
	std::vector<float>::iterator I = controller->readings().begin();
	std::vector<float>::iterator end = controller->readings().end();
	float seconds = 0;
	while (I != end)
	{
		send_reading(*I, seconds, client, controller->readings().size() == 1);
		seconds += READING_TIME / 1000.0;
		I ++;
	}
}

void save_file(AsyncWebServerRequest *request, const String& fname)
{
	if(request->hasParam("body", true))
	{
		AsyncWebParameter* p = request->getParam("body", true);
		const String &json = p->value();

		Serial.print("Saving file " + fname);

		File f = SPIFFS.open(fname, "w");
	  if (!f) {
			request->send(404);
			return;
		}

		// TODO sanity checks

		f.write((const uint8_t*)json.c_str(), json.length());

		f.close();
		request->send(200);
	}
}

void setup() {
	Serial.begin(115200);

	OTA.addAP(WIFI_SSID, WIFI_PASSWORD);
	SPIFFS.begin();

	setupController(new ReflowController());

	server.addHandler(&ws);
	server.addHandler(&events);
	server.serveStatic("/", SPIFFS, "/web").setDefaultFile("index.html");
	// Heap for general Servertest
	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});
	server.on("/profiles", HTTP_GET, [](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/profiles.json");
		//request->send(SPIFFS, "/profiles.json");
		response->addHeader("Access-Control-Allow-Origin", "null");
		response->addHeader("Access-Control-Allow-Methods", "GET");
		response->addHeader("Content-Type", "application/json");
		request->send(response);
	});
	server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/config.json");
	});
	server.on("/profiles", HTTP_POST, [](AsyncWebServerRequest *request) {
		save_file(request, "/profiles.json");
	});
	server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
		save_file(request, "/config.json");
	});
	server.onNotFound(
			[](AsyncWebServerRequest *request) { request->send(404); });

	ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
		if (type == WS_EVT_DATA) {
			char cmd[128] = "";
			memcpy(cmd, data, min(len, sizeof(cmd) - 1));

			if (strcmp(cmd, "get-data") == 0) {
			} else if (strncmp(cmd, "profile:", 8) == 0) {
				controller->profile(String(cmd + 8));
				client->text("{\"profile\": \"" + controller->profile() + "\"}");
			} else if (strcmp(cmd, "ON") == 0) {
				controller->mode(ControllerBase::ON);
			} else if (strcmp(cmd, "TARGET_OFF") == 0) {
				controller->mode(ControllerBase::TARGET_OFF);
			} else if (strcmp(cmd, "TARGET_PID") == 0) {
				controller->mode(ControllerBase::TARGET_PID);
			} else if (strcmp(cmd, "REFLOW") == 0) {
				controller->mode(ControllerBase::REFLOW);
			} else if (strcmp(cmd, "OFF") == 0) {
				controller->mode(ControllerBase::OFF);
			} else if (strncmp(cmd, "target:", 7) == 0) {
				controller->target(max(0, min(atoi(cmd + 7), MAX_TEMPERATURE)));
				client->text("{\"target\": " + String(controller->target()) + "}");
			}
		} else if (type == WS_EVT_CONNECT) {
			_client = client;
			client->text("{\"message\": \"INFO: Connected!\", \"mode\": \""
				+ String(controller->translate_mode()) + "\""
				+ ", \"target\": "
				+ String(controller->target())
				+ ", \"profile\": \""
				+ controller->profile()
				+ "\"}");
			send_data(client);
			Serial.println("Connected...");
		} else if (type == WS_EVT_DISCONNECT) {
			_client = NULL;
		}
	});

	server.begin();
}

void loop() {
	unsigned long now = millis();

  OTA.loop(now);

	// since this is single core, we don't care about
	// synchronization
	if (controller)
		controller->loop(now);
	if (last_controller) {
		delete last_controller;
		last_controller = NULL;
	}
}
