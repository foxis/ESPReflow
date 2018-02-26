#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <PID_v10.h>
#include <SPI.h>
#include <max6675.h>
#include "Config.h"
#include <PID_AutoTune_v0.h>

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

#define CB_GETTER(T, name) virtual T name() { return _##name; }
#define CB_SETTER(T, name) virtual T name(T name) { T pa##name = _##name; _##name = name; return pa##name; }

class ControllerBase
{
public:
	typedef float Temperature_t;
private:
	std::vector<Temperature_t> _readings;
	double _temperature;
	double _target;
	double _CALIBRATE_max_temperature;
	double _target_control;

	double _calP, _calD, _calI;

	int _calIterations;

	PID pidTemperature;
	Config& config;
	PID_ATune aTune;

public:

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

	typedef std::function<void(const String& message)> THandlerFunction_Message;
	typedef std::function<void(MODE_t last, MODE_t current)> THandlerFunction_Mode;
	typedef std::function<void(const String& stage)> THandlerFunction_Stage;
	typedef std::function<void(bool heater)> THandlerFunction_Heater;
	typedef std::function<void(const std::vector<Temperature_t>& readings, unsigned long now)> THandlerFunction_Measure;

	ControllerBase(Config& cfg) :
		config(cfg),
		pidTemperature(&_temperature, &_target_control, &_target, .5/DEFAULT_TEMP_RISE_AFTER_OFF, 5.0/DEFAULT_TEMP_RISE_AFTER_OFF, 4/DEFAULT_TEMP_RISE_AFTER_OFF, DIRECT),
		aTune(&_temperature, &_target_control)
	{
		//Serial.println("Setup controlller base ");
		if (config.pid.find("default") == config.pid.end())
			Serial.println("no default pid !!");
		else
			setPID(config.pid["default"].P, config.pid["default"].I, config.pid["default"].D);

		_calP = .5/DEFAULT_TEMP_RISE_AFTER_OFF;
		_calD =  5.0/DEFAULT_TEMP_RISE_AFTER_OFF;
		_calI = 4/DEFAULT_TEMP_RISE_AFTER_OFF;

		pidTemperature.SetSampleTime(config.measureInterval * 1000);
    pidTemperature.SetMode(AUTOMATIC);
		pidTemperature.SetOutputLimits(0, 1);
		thermocouple.begin(thermoCLK, thermoCS, thermoDO);

		_mode = _last_mode = INIT;
		_temperature = 0;
		_target = DEFAULT_TARGET;
		_onMessage = NULL;
		_onMode = NULL;
		_onMeasure = NULL;
		_locked = false;

		_heater = _last_heater = false;

		pinMode(relay, OUTPUT);
		pinMode(LED_BUILTIN, OUTPUT);

		digitalWrite(relay, _heater);
		digitalWrite(LED_BUILTIN, !_heater);

		_temperature = thermocouple.readCelsius();
		_readings.push_back(temperature_to_log(_temperature));
	}

	~ControllerBase() {
		_readings.clear();
	}

	virtual String name() = 0;

	virtual void loop(unsigned long now)
	{
		if (_mode >= ON)
		{
			handle_measure(now);
		}

		switch (_mode)
		{
			case INIT:
				callMessage(name() + "Initialized and ready");
				_mode = OFF;
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
				_CALIBRATE_max_temperature = max(_CALIBRATE_max_temperature, _temperature);
				if (_temperature < SAFE_TEMPERATURE) {
					callMessage("INFO: Temperature has reached safe levels (<" + String(SAFE_TEMPERATURE) + "*C). Max temperature: " + String(_CALIBRATE_max_temperature));
					_mode = OFF;
				}
				if (config.pid.find("default") == config.pid.end()) {
					Serial.println("no default pid !!");
					callMessage("WARNING: No default PID found!");
				} else
					setPID(config.pid["default"].P, config.pid["default"].I, config.pid["default"].D);
		}

		handle_mode(now);

		handle_safety(now);

		digitalWrite(relay, _heater);
		digitalWrite(LED_BUILTIN, !_heater);

		if (_onHeater && _heater != _last_heater)
			_onHeater(_heater);
		_last_heater = _heater;
	}

	virtual void handle_mode(unsigned long now) {
		if (_last_mode <= OFF && _mode > OFF)
		{
			_readings.clear();
			_start_time = now;
			_temperature = thermocouple.readCelsius();
			pidTemperature.Reset();
			_readings.push_back(temperature_to_log(_temperature));
			last_m = now;
			last_log_m = now;

			if (_mode == CALIBRATE) {
				_target_control = .5; 		// initial output
				_temperature = _target;		// target temperature
				aTune.Cancel();						// just in case
				aTune.SetNoiseBand(1);		// noise band +-1*C
				aTune.SetOutputStep(.5);	// change output +-.5 around initial output
				aTune.SetControlType(1); 	// PID
				aTune.SetLookbackSec(config.measureInterval / 10);	// this one I don't know what it really does, but we need to register every reading
				aTune.Runtime(now);				// initialize autotuner here, as later we give it actual readings
			}

		} else if (_mode <= OFF && _last_mode > OFF)
		{
			_temperature = thermocouple.readCelsius();
			_readings.push_back(temperature_to_log(_temperature));
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
			if (_mode != CALIBRATE && _mode != CALIBRATE_COOL)
				pidTemperature.Compute(now * 1000);
			Serial.println("Tt: " + String(_target) + "           T: " + String(_temperature) + "        Ctrl: " + String(_target_control));
		}

		if (now - last_log_m > config.reportInterval) {
			_readings.push_back(temperature_to_log(_temperature));
			last_log_m = now;
			if (_onMeasure)
				_onMeasure(_readings, now);
		}
	}

	virtual void handle_safety(unsigned long now) {
		if (!_heater)
			return;

		if (now - _start_time > MAX_ON_TIME && _temperature < SAFE_TEMPERATURE)
		{
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Heater time limit exceeded");
		}

		if (_temperature > MAX_TEMPERATURE)
		{
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Temperature limit exceeded");
		}

		if (now - _start_time > MIN_TEMP_RISE_TIME && _temperature - _readings[0] < MIN_TEMP_RISE && _mode != CALIBRATE) {
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Temperature did not rise for " + String((int)(MIN_TEMP_RISE_TIME / 1000)) + " seconds");
		}
	}

	virtual void handle_pid(unsigned long now) {
		_heater = now - last_m < config.measureInterval * _target_control && _target_control > CONTROL_HYSTERISIS ||
						now - last_m >= config.measureInterval * _target_control && _target_control > 1.0-CONTROL_HYSTERISIS;
	}

	virtual void handle_reflow(unsigned long now) = 0;

	virtual void handle_calibration(unsigned long now) {

		if (aTune.Runtime(now))
		{
			_heater = false;
			_mode = OFF;
			_calP = aTune.GetKp();
			_calI = aTune.GetKi();
			_calD = aTune.GetKd();
			callMessage("INFO: Calibration data available! [" + String(_calP) + ", " + String(_calI) + ", " + String(_calD) + "]");
		} else
			handle_pid(now);
	}

	CB_GETTER(double, target)
	CB_SETTER(double, target)

	CB_GETTER(double, temperature)
	CB_SETTER(double, temperature)

	CB_GETTER(std::vector<float>&, readings)

	CB_GETTER(MODE_t, mode)
	CB_SETTER(MODE_t, mode)

	CB_GETTER(bool, locked)
	CB_SETTER(bool, locked)

	CB_GETTER(unsigned long, start_time)

	CB_SETTER(const String&, profile)
	CB_GETTER(const String&, profile)

	CB_GETTER(const String&, stage)

	CB_SETTER(THandlerFunction_Message, onMessage)
	CB_SETTER(THandlerFunction_Mode, onMode)
	CB_SETTER(THandlerFunction_Heater, onHeater)
	CB_SETTER(THandlerFunction_Stage, onStage)
	CB_SETTER(THandlerFunction_Measure, onMeasure)

	PID& setPID(float P, float I, float D) {
		pidTemperature.SetTunings(P, I, D);
		return pidTemperature;
	}

	String calibrationString() {
		return String("[" + String(_calP) + ", " + String(_calI) + ", " + String(_calD) + "]");
	}

	const char * translate_mode(MODE_t mode = UNKNOWN)
	{
		MODE_t m = mode = UNKNOWN ? _mode : mode;
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
	void callMessage(const String& message) {
		if (_onMessage)
			_onMessage(message);
		Serial.println("Message: " + message);
	}

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

	THandlerFunction_Message _onMessage;
	THandlerFunction_Mode _onMode;
	THandlerFunction_Heater _onHeater;
	THandlerFunction_Stage _onStage;
	THandlerFunction_Measure _onMeasure;
};

#endif
