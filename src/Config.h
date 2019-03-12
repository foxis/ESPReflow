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

#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <EasyOTA.h>
#include <map>
#include "wificonfig.h"

class Config {
public:
	typedef struct {
		float P, I, D;
	} PID_t;

	class Stage {
	public:
		Stage(const char * n, const char * p, float t, float r, float s);

		String name;
		String pid;
		float target;
		float rate;
		float stay;
	};
	typedef std::vector<Stage>::iterator stages_iterator;

	class Profile {
	public:
		Profile(JsonObject& json);

		stages_iterator begin() {return stages.begin();}
		stages_iterator end() {return stages.end();}

		std::vector<Stage> stages;
		String name;
	};

	typedef std::map<String, Profile>::iterator profiles_iterator;

public:
	String cfgName;
	String profilesName;
	std::map<String, String> networks;

	std::map<String, PID_t> pid;
	std::map<String, Profile> profiles;

public:
	String hostname;
	String user;
	String password;
	String otaPassword;
	float measureInterval;
	float reportInterval;
	int tuner_id;
	double tuner_init_output;
	double tuner_noise_band;
	double tuner_output_step;

	EasyOTA *OTA;

	typedef std::function<bool(JsonObject& json, Config * self)> THandlerFunction_parse;

public:
	Config(const String& cfg, const String& profiles);

	bool load_config();

	bool load_profiles();

	bool load_json(const String& name, size_t max_size, THandlerFunction_parse parser);

	bool setup_OTA();

	bool save_config(AsyncWebServerRequest *request, uint8_t * data, size_t len, size_t index, size_t total);
	bool save_profiles(AsyncWebServerRequest *request, uint8_t * data, size_t len, size_t index, size_t total);

	bool save_file(AsyncWebServerRequest *request, const String& fname, uint8_t * data, size_t len, size_t index, size_t total);
};

void S_printf(const char * format, ...);

#endif
