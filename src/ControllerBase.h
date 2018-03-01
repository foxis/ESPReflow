#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <PID_v10.h>
#include <SPI.h>
#include <max6675.h>
#include "Config.h"
#include <PID_AutoTune_v0.h>  // https://github.com/tom131313/Arduino-PID-AutoTune-Library

#define thermoDO 13 // D7
#define thermoCS 12 // D6
#define thermoCLK 14 // D5
#define relay 4 // D2

#define DEFAULT_TARGET 100
#define MAX_ON_TIME 1000 * 60 * 15
#define MAX_TEMPERATURE 400
#define MIN_TEMP_RISE_TIME 1000 * 40
#define MIN_TEMP_RISE 10
#define CONTROL_HYSTERISIS .1
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

	unsigned long _now;

	double _calP, _calD, _calI;

	unsigned long _watchdog;

	PID pidTemperature;
	PID_ATune aTune;

protected:
	Config& config;

public:
	ControllerBase(Config& cfg) :
		config(cfg),
		pidTemperature(&_temperature, &_target_control, &_target, .5/DEFAULT_TEMP_RISE_AFTER_OFF, 5.0/DEFAULT_TEMP_RISE_AFTER_OFF, 4/DEFAULT_TEMP_RISE_AFTER_OFF, DIRECT),
		aTune(&_temperature, &_target_control, &_target, &_now, DIRECT)
	{
		_readings.reserve(15 * 60);

		_calP = .5/DEFAULT_TEMP_RISE_AFTER_OFF;
		_calD =  5.0/DEFAULT_TEMP_RISE_AFTER_OFF;
		_calI = 4/DEFAULT_TEMP_RISE_AFTER_OFF;

		pidTemperature.SetSampleTime(config.measureInterval * 1000);
    pidTemperature.SetMode(AUTOMATIC);
		pidTemperature.SetOutputLimits(-1, 1);
		thermocouple.begin(thermoCLK, thermoCS, thermoDO);

		_mode = _last_mode = INIT;
		_temperature = 0;
		_target = DEFAULT_TARGET;
		_onMessage = NULL;
		_onMode = NULL;
		_onReadingsReport = NULL;
		_locked = false;
		_watchdog = 0;

		_heater = _last_heater = false;

		pinMode(relay, OUTPUT);
		pinMode(LED_BUILTIN, OUTPUT);

		digitalWrite(relay, _heater);
		digitalWrite(LED_BUILTIN, !_heater);

		setPID("default");

		_temperature = thermocouple.readCelsius();
		_readings.push_back(temperature_to_log(_temperature));
	}

	virtual const char * name() = 0;

	virtual void loop(unsigned long now)
	{
		if (_mode >= ON)
		{
			handle_measure(now);
		}

		switch (_mode)
		{
			case INIT:
				callMessage("%s Initialized and ready", name());
				mode(OFF);
				break;
			case ON:
				//callMessage("WARNING: Heater is on until turned off");
				_heater = true;
				break;
			case ERROR_OFF:
			case OFF:
				//callMessage("WARNING: Heater is on until turned off");
				_heater = false;
				break;
			case TARGET_PID:
				handle_pid(now);
				break;
			case REFLOW:
				handle_reflow(now);
				break;
			case CALIBRATE:
				handle_calibration(now);
				break;
			case CALIBRATE_COOL:
			case REFLOW_COOL:
				_heater = false;
				if (_temperature < SAFE_TEMPERATURE) {
					callMessage("INFO: Temperature has reached safe levels (<%.2f*C). Max temperature: %.2f", (float)SAFE_TEMPERATURE, (float)_CALIBRATE_max_temperature);
					mode(OFF);
				}
		}

		handle_mode(now);

		handle_safety(now);

		digitalWrite(relay, _heater);
		digitalWrite(LED_BUILTIN, !_heater);

		if (_onHeater && _heater != _last_heater)
			_onHeater(_heater);
		_last_heater = _heater;
	}

	CB_GETTER(double, target)
	CB_SETTER(double, target)

	CB_GETTER(double, temperature)

	CB_GETTER(std::vector<Temperature_t>&, readings)

	CB_GETTER(MODE_t, mode)
	CB_SETTER(MODE_t, mode)

	CB_GETTER(bool, locked)
	CB_SETTER(bool, locked)

	CB_GETTER(unsigned long, start_time)

	CB_SETTER(const String&, profile)
	CB_GETTER(const String&, profile)

	CB_SETTER(unsigned long, watchdog)
	CB_GETTER(unsigned long, watchdog)

	CB_GETTER(const String&, stage)

	CB_SETTER(THandlerFunction_Message, onMessage)
	CB_SETTER(THandlerFunction_Mode, onMode)
	CB_SETTER(THandlerFunction_Heater, onHeater)
	CB_SETTER(THandlerFunction_Stage, onStage)
	CB_SETTER(THandlerFunction_ReadingsReport, onReadingsReport)

	PID& setPID(float P, float I, float D) {
		pidTemperature.Reset();
		pidTemperature.SetTunings(P, I, D);
		return pidTemperature;
	}

	PID& setPID(const String& name) {
		if (config.pid.find(name) == config.pid.end()) {
			callMessage("WARNING: No PID named '%s' found!!", name.c_str());
			return pidTemperature;
		} else {
			callMessage("INFO: Setting PID to '%s'.", name.c_str());
			return setPID(config.pid[name].P, config.pid[name].I, config.pid[name].D);
		}
	}

	String calibrationString() {
		char str[64] = "";
		sprintf(str, "[%f, %f, %f]", _calP, _calI, _calD);
		return str;
	}

	const char * translate_mode(MODE_t mode = UNKNOWN)
	{
		MODE_t m = mode == UNKNOWN ? _mode : mode;
		switch (m)
		{
			case INIT: return "Init"; break;
			case ON: return "ON"; break;
			case OFF: return "OFF"; break;
			case CALIBRATE: return  "Calibrating 1"; break;
			case TARGET_PID: return  "Keep Target"; break;
			case CALIBRATE_COOL: return  "Cooldown"; break;
			case ERROR_OFF: return  "Error"; break;
			case REFLOW: return  "Reflow"; break;
			case REFLOW_COOL: return  "Cooldown"; break;
		}
	}

	Temperature_t temperature_to_log(float t) {
		return t; // TODO: convert to 16bit fixed point
	}

	float log_to_temperature(Temperature_t t) {
		return t;
	}

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

	void callMessage(const char * format, ...) {
		char buffer[512];
	  va_list args;
	  va_start (args, format);
	  vsnprintf (buffer, sizeof(buffer), format, args);
		if (_onMessage)
			_onMessage(buffer);
		Serial.println(buffer);
	  va_end (args);
	}

	void reportReadings(unsigned long now)
	{
		if (_onReadingsReport)
			_onReadingsReport(_readings, now);
	}

	virtual void handle_mode(unsigned long now) {
		if (_last_mode <= OFF && _mode > OFF)
		{
			_start_time = now;
			_temperature = thermocouple.readCelsius();
			pidTemperature.Reset();
			_readings.clear();
			_readings.push_back(temperature_to_log(_temperature));
			reportReadings(now - _start_time);
			last_m = now;
			last_log_m = now;

			if (_mode == CALIBRATE) {
				_target_control = .5; 		// initial output
				//_temperature = _target;		// target temperature
				aTune.Cancel();						// just in case
				aTune.SetNoiseBand(1);		// noise band +-1*C
				aTune.SetOutputStep(.5);	// change output +-.5 around initial output
				aTune.SetControlType(PID_CONTROL); 	// PID
				aTune.SetSampleTime(config.measureInterval);	// this one I don't know what it really does, but we need to register every reading
				aTune.SetLookbackTime(config.measureInterval * 10);	// this one I don't know what it really does, but we need to register every reading
				_now = now;
				aTune.Runtime();				// initialize autotuner here, as later we give it actual readings
			} else if (_mode == TARGET_PID) {
				setPID("default");
				pidTemperature.Reset();
			}

		} else if (_mode <= OFF && _last_mode > OFF)
		{
			_temperature = thermocouple.readCelsius();
			_readings.push_back(temperature_to_log(_temperature));
			if (_last_mode == REFLOW || _last_mode == REFLOW_COOL)
				setPID("default");
			reportReadings(now - _start_time);
		}
		if (_onMode && _last_mode != _mode) {
			_onMode(_last_mode, _mode);
		}
		_last_mode = _mode;
	}

	virtual void handle_measure(unsigned long now) {
		if (now - last_m > config.measureInterval)
		{
			_temperature = thermocouple.readCelsius();
			last_m = now;
			if (_mode != CALIBRATE) {
				pidTemperature.Compute(now * 1000);
				_target_control = max(_target_control, 0.0);
			}

			callMessage("DEBUG: PID: <code>e=%f     i=%f     d=%f       Tt=%f       T=%f     C=%f</code>",
					pidTemperature._e, pidTemperature._i, pidTemperature._d, (float)_target, (float)_temperature, (float)_target_control);
		}

		if (now - last_log_m > config.reportInterval) {
			_readings.push_back(temperature_to_log(_temperature));
			last_log_m = now;
			reportReadings(now - _start_time);
		}
	}

	virtual void handle_safety(unsigned long now) {
		if (!_heater)
			return;

		if (now - _start_time > MAX_ON_TIME && _temperature < SAFE_TEMPERATURE)
		{
			mode(ERROR_OFF);
			_heater = false;
			callMessage("ERROR: Heater time limit exceeded");
		}

		if (_temperature > MAX_TEMPERATURE)
		{
			mode(ERROR_OFF);
			_heater = false;
			callMessage("ERROR: Temperature limit exceeded");
		}

		if (isnan(_temperature)) {
			mode(ERROR_OFF);
			_heater = false;
			callMessage("ERROR: Error reading temperature. Check the probe!");
		}

		if (now - _watchdog > WATCHDOG_TIMEOUT) {
			mode(ERROR_OFF);
			_heater = false;
			callMessage("ERROR: Watchdog timeout. Check connectivity!");
		}
return;
		if (now - _start_time > MIN_TEMP_RISE_TIME && _temperature - _readings[0] < MIN_TEMP_RISE && _temperature < SAFE_TEMPERATURE) {
			mode(ERROR_OFF);
			_heater = false;
			callMessage("ERROR: Temperature did not rise for %i seconds!",  (int)(MIN_TEMP_RISE_TIME / 1000));
		}
	}

	virtual void handle_pid(unsigned long now) {
		_heater = now - last_m < config.measureInterval * _target_control && _target_control > CONTROL_HYSTERISIS ||
						now - last_m >= config.measureInterval * _target_control && _target_control > 1.0-CONTROL_HYSTERISIS;
	}

	virtual void handle_reflow(unsigned long now) = 0;

	virtual void handle_calibration(unsigned long now) {
		_now = now;
		switch (aTune.Runtime()) {
			case 1:
				_heater = false;
				mode(CALIBRATE_COOL);
				_calP = aTune.GetKp();
				_calI = aTune.GetKi();
				_calD = aTune.GetKd();
				callMessage("INFO: Calibration data available! PID = [%f, %f, %f]", _calP, _calI, _calD);
				break;
			case 3:
				callMessage("INFO: Calibration - peak detected");
				break;
			case 2:
				//callMessage("INFO: Calibration - peak skipped");
				break;
			default:
				//callMessage("INFO: Calibration - Weird return value");
				break;
		}

		handle_pid(now);

		_CALIBRATE_max_temperature = max(_CALIBRATE_max_temperature, _temperature);
	}
};

#endif
