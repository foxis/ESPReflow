#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <PID_v10.h>
#include <SPI.h>
#include <max6675.h>
#include "Config.h"
#include <PID_AutoTune_v0.h>  // https://github.com/t0mpr1c3/Arduino-PID-AutoTune-Library

#define thermoDO 13 // D7
#define thermoCS 12 // D6
#define thermoCLK 14 // D5
#define relay 4 // D2

#define DEFAULT_TARGET 60
#define MAX_ON_TIME 1000 * 60 * 2
#define MAX_TEMPERATURE 400
#define MIN_TEMP_RISE_TIME 1000 * 40
#define MIN_TEMP_RISE 10
#define CONTROL_HYSTERISIS .01
#define DEFAULT_TEMP_RISE_AFTER_OFF 30.0
#define SAFE_TEMPERATURE 50
#define CAL_HEATUP_TEMPERATURE 90
#define DEFAULT_CAL_ITERATIONS 3
#define WATCHDOG_TIMEOUT 30000

#define CB_GETTER(T, name) virtual T name() { return _##name; }
#define CB_SETTER(T, name) virtual T name(T name) { T pa##name = _##name; _##name = name; return pa##name; }

class ControllerBase
{
public:
	typedef float Temperature_t;
	typedef enum {
		UNKNOWN = -100,
		INIT = -2,
		ERROR_OFF = -1,
		OFF = 0,
		ON = 1,
		TARGET_PID = 2,
		CALIBRATE = 3,
		REFLOW = 4,
		CALIBRATE_COOL = 5,
		REFLOW_COOL = 6,
	} MODE_t;

	typedef std::function<void(const char * message)> THandlerFunction_Message;
	typedef std::function<void(MODE_t last, MODE_t current)> THandlerFunction_Mode;
	typedef std::function<void(const char * stage, float target)> THandlerFunction_Stage;
	typedef std::function<void(bool heater)> THandlerFunction_Heater;
	typedef std::function<void(const std::vector<Temperature_t>& readings, unsigned long now)> THandlerFunction_ReadingsReport;

private:
	std::vector<Temperature_t> _readings;
	double _temperature;
	double _target;
	double _CALIBRATE_max_temperature;
	double _target_control;
	double _avg_rate;
	unsigned long _last_heater_on;

	unsigned long _now;

	double _calP, _calD, _calI;

	unsigned long _watchdog;

	PID pidTemperature;
	PID_ATune aTune;

protected:
	Config& config;

public:
	ControllerBase(Config& cfg);

	virtual const char * name() = 0;

	virtual void loop(unsigned long now);

	CB_GETTER(double, target)
	CB_SETTER(double, target)

	CB_GETTER(double, temperature)

	CB_GETTER(std::vector<Temperature_t>&, readings)

	CB_GETTER(MODE_t, mode)
	CB_SETTER(MODE_t, mode)

	CB_GETTER(bool, heater)

	CB_GETTER(bool, locked)
	CB_SETTER(bool, locked)

	CB_GETTER(unsigned long, start_time)

	CB_SETTER(const String&, profile)
	CB_GETTER(const String&, profile)

	CB_SETTER(unsigned long, watchdog)
	CB_GETTER(unsigned long, watchdog)

	CB_SETTER(double, avg_rate)
	CB_GETTER(double, avg_rate)

	CB_GETTER(const String&, stage)

	CB_SETTER(THandlerFunction_Message, onMessage)
	CB_SETTER(THandlerFunction_Mode, onMode)
	CB_SETTER(THandlerFunction_Heater, onHeater)
	CB_SETTER(THandlerFunction_Stage, onStage)
	CB_SETTER(THandlerFunction_ReadingsReport, onReadingsReport)

	PID& setPID(float P, float I, float D);

	PID& setPID(const String& name);

	void resetPID();

	String calibrationString();

	const char * translate_mode(MODE_t mode = UNKNOWN);

	Temperature_t temperature_to_log(float t);

	float log_to_temperature(Temperature_t t);

	float measure_temperature(unsigned long now);
	unsigned long elapsed(unsigned long now);

private:
	MAX6675 thermocouple;
	bool _locked;
	bool _heater;
	bool _last_heater;
	unsigned long last_m;
	unsigned long last_log_m;
	unsigned long _start_time;

	MODE_t _mode;
	MODE_t _last_mode;

	String _profile;
	String _stage;

protected:
	CB_SETTER(const String&, stage)
	CB_SETTER(double, temperature)

	THandlerFunction_Message _onMessage;
	THandlerFunction_Mode _onMode;
	THandlerFunction_Heater _onHeater;
	THandlerFunction_Stage _onStage;
	THandlerFunction_ReadingsReport _onReadingsReport;

	void callMessage(const char * format, ...) ;

	void reportReadings(unsigned long now);

	virtual void handle_mode(unsigned long now);

	virtual void handle_measure(unsigned long now);

	virtual void handle_safety(unsigned long now);

	virtual void handle_pid(unsigned long now);

	virtual void handle_reflow(unsigned long now) = 0;

	virtual void handle_calibration(unsigned long now);
};

#endif
