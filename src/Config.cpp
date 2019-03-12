/**
 *  Copyright (C) 2018  foxis (Andrius Mikonis <andrius.mikonis@gmail.com>)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "Config.h"

Config::Stage::Stage(const char * n, const char * p, float t, float r, float s) :
 name(n), pid(p), target(t), rate(r), stay(s) {
}

Config::Profile::Profile(JsonObject& json)
{
	char str[255] = "";
	stages.reserve(5);

	name = json["name"].as<char*>();
	JsonArray& jo = json["stages"];
	JsonArray::iterator I = jo.begin();
	while (I != jo.end())
	{
		const char * stage_name = I->as<char*>();
		JsonObject &stage = json[stage_name];
		Stage s(
			stage_name,
			stage["pid"].as<char*>(),
			stage["target"],
			stage["rate"],
			stage["stay"]
		);
		stages.push_back(s);
		sprintf(str, "Profile stage: %s, t=%f, r=%f, s=%f", s.name.c_str(), s.target, s.rate, s.stay);
		Serial.println(str);
		++I;
	}
}

Config::Config(const String& cfg, const String& profiles) :
	cfgName(cfg),
	profilesName(profiles) {
}

bool Config::load_config() {
	return load_json(cfgName, 1024, [](JsonObject& json, Config* self){
		char str[255] = "";
		self->networks.empty();
		self->hostname = json["hostname"].as<char*>();
		self->user = json["user"].as<char*>();
		self->password = json["password"].as<char*>();
		self->otaPassword = json["otaPassword"].as<char*>();
		self->measureInterval = json["measureInterval"];
		self->reportInterval = json["reportInterval"];

		sprintf(str, "Config hostname: %s", self->hostname.c_str());
		Serial.println(str);
		sprintf(str, "Config OTA password: %s", self->otaPassword.c_str());
		Serial.println(str);
		sprintf(str, "Config user: %s @ %s", self->user.c_str(), self->password.c_str());
		Serial.println(str);
		sprintf(str, "Config measure/report intervals: %f @ %f", self->measureInterval, self->reportInterval);
		Serial.println(str);

		JsonObject& jo = json["networks"];
		JsonObject::iterator I = jo.begin();
		while (I != jo.end())
		{
			self->networks.insert(std::pair<String, String>(I->key, I->value.as<char*>()));

			sprintf(str, "Config network: %s @ %s", I->key, I->value.as<char*>());
			Serial.println(str);
			++I;
		}
		return true;
	});
}

bool Config::load_profiles() {
	return load_json(profilesName, 10240, [](JsonObject& json, Config* self){
		char str[255] = "";
		self->pid.clear();
		JsonObject::iterator I;
		JsonObject& pid = json["PID"];
		I = pid.begin();
		while (I != pid.end())
		{
			PID_t p = {
				I->value[0],
				I->value[1],
				I->value[2],
			};
			self->pid.insert(std::pair<String, PID_t>(I->key, p));
			sprintf(str, "Profiles PID: %s [%f, %f, %f]", I->key, p.P, p.I, p.D);
			Serial.println(str);
			++I;
		}

		self->profiles.clear();
		JsonObject& profiles = json["profiles"];
		I = profiles.begin();
		while (I != profiles.end())
		{
			sprintf(str, "Profile %s: %s", I->key, I->value["name"].as<char*>());
			Serial.println(str);

			Profile p((JsonObject&)I->value);
			self->profiles.insert(std::pair<String, Profile>(I->key, p));
			++I;
		}
		self->tuner_id = json["tuner"]["id"];
		self->tuner_init_output = json["tuner"]["init_output"];
		self->tuner_noise_band = json["tuner"]["noise_band"];
		self->tuner_output_step = json["tuner"]["output_step"];

		return true;
	});
}

bool Config::load_json(const String& name, size_t max_size, THandlerFunction_parse parser) {
	Serial.println("Loading config " + name + "; Heap: " + String(ESP.getFreeHeap()));
	File configFile = SPIFFS.open(name, "r");
	if (!configFile) {
		Serial.println("Could not open config file");
		return false;
	}

	size_t size = configFile.size();
	if (size > max_size) {
		configFile.close();
		Serial.println("config file size is too large: " + String(size));
		return false;
	}

	DynamicJsonBuffer jsonBuffer;
	JsonObject &json = jsonBuffer.parseObject(configFile);
	configFile.close();

	if (!json.success()) {
		Serial.println("Failed parsing config file");
		return false;
	}

	bool parsed = false;
	if (parser)
	 	parsed = parser(json, this);

	Serial.println("Loading config " + name + " DONE; Heap: " + String(ESP.getFreeHeap()));
	return parsed;
}

bool Config::setup_OTA() {
	Serial.println("OTA setup");

	OTA = new EasyOTA(hostname);

	std::map<String, String>::iterator I = networks.begin();
	while (I != networks.end()) {
		OTA->addAP(I->first, I->second);
		Serial.println("Add network: " + I->first);
		I++;
	}

	OTA->onConnect([](const String& ssid, EasyOTA::STATE state) {
		S_printf("Connected %s, state: %s", ssid.c_str(), state == EasyOTA::EOS_STA ? "Station" : "Access Point");
	});

	OTA->onMessage([](const String& msg, int line) {
		S_printf("OTA message: %s", msg.c_str());
	});
}

bool Config::save_config(AsyncWebServerRequest *request, uint8_t * data, size_t len, size_t index, size_t total) {
	return save_file(request, cfgName, data, len, index, total);
}
bool Config::save_profiles(AsyncWebServerRequest *request, uint8_t * data, size_t len, size_t index, size_t total) {
	return save_file(request, profilesName, data, len, index, total);
}

bool Config::save_file(AsyncWebServerRequest *request, const String& fname, uint8_t * data, size_t len, size_t index, size_t total)
{
	Serial.println("Saving config " + fname +" len/index: " + String(len) + "/" +  String(index));

	File f = SPIFFS.open(fname, index != 0 ? "a" : "w");
  if (!f) {
		request->send(404, "application/json", "{\"msg\": \"ERROR: couldn't " + fname + " file for writing!\"}");
		return false;
	}

	// TODO sanity checks

	f.write(data, len);

	if (f.size() >= total)
	{
		request->send(200, "application/json", "{\"msg\": \"INFO: " + fname + " saved!\"}");
		Serial.println("Saving config... DONE");
	}

	f.close();
	return true;
}

void S_printf(const char * format, ...) {
	char buffer[512];
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, sizeof(buffer), format, args);
	Serial.println(buffer);
	va_end (args);
}
