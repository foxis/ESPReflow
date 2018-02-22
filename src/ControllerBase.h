#ifndef CONTROLLER_BASE_H
#define CONTROLLER_BASE_H

#include <PID_v1.h>
#include <SPI.h>
#include <max6675.h>

#define thermoDO 13 // D7
#define thermoCS 12 // D6
#define thermoCLK 14 // D5
#define relay 4 // D2

#define DEFAULT_TARGET 100
#define MAX_ON_TIME 1000 * 60 * 15
#define MAX_TEMPERATURE 400
#define MIN_TEMP_RISE_TIME 1000 * 40
#define MIN_TEMP_RISE 10
#define READING_TIME 1000
#define TEMPERATURE_LOGGING_TIME 10 * 1000
#define CONTROL_HYSTERISIS .1
#define DEFAULT_TEMP_RISE_AFTER_OFF 30.0
#define SAFE_TEMPERATURE 50

#define CB_GETTER(T, name) virtual T name() { return _##name; }
#define CB_SETTER(T, name) virtual T name(T name) { T pa##name = _##name; _##name = name; return pa##name; }

class ControllerBase
{
	std::vector<float> _readings;
	double _temperature;
	double _target;
	double _target_off_max_temperature;
	double _target_control;

	PID pidTemperature;

public:
	typedef enum {
		UNKNOWN = -100,
		INIT = -2,
		ERROR_OFF = -1,
		OFF = 0,
		ON = 1,
		TARGET_PID = 2,
		TARGET_OFF = 3,
		REFLOW = 4,
		TARGET_OFF_COOL = 5,
		REFLOW_COOL = 6,
	} MODE_t;

	typedef std::function<void(const String& message)> THandlerFunction_Message;
	typedef std::function<void(MODE_t last, MODE_t current)> THandlerFunction_Mode;
	typedef std::function<void(const String& stage)> THandlerFunction_Stage;
	typedef std::function<void(bool heater)> THandlerFunction_Heater;
	typedef std::function<void(const std::vector<float>& readings, unsigned long now)> THandlerFunction_Measure;

	ControllerBase() : pidTemperature(&_temperature, &_target_control, &_target, .5/DEFAULT_TEMP_RISE_AFTER_OFF, 5.0/DEFAULT_TEMP_RISE_AFTER_OFF, 4/DEFAULT_TEMP_RISE_AFTER_OFF, DIRECT)
	{
		pidTemperature.SetSampleTime(READING_TIME * 1000);
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
		_readings.push_back(_temperature);
	}

	~ControllerBase() {
		_readings.clear();
	}

	virtual String name() = 0;

	virtual void loop(unsigned long now)
	{
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
			case TARGET_OFF:
				if (_temperature > _target)
				{
					callMessage("INFO: Temperature has reached the target. Turning off the heater");
					_mode = TARGET_OFF_COOL;
					_heater = false;
					_target_off_max_temperature = _temperature;
				} else
					_heater = true;
				break;
			case TARGET_OFF_COOL:
			case REFLOW_COOL:
				_heater = false;
				_target_off_max_temperature = max(_target_off_max_temperature, _temperature);
				if (_temperature < SAFE_TEMPERATURE) {
					callMessage("INFO: Temperature has reached safe levels (<" + String(SAFE_TEMPERATURE) + "*C). Max temperature: " + String(_target_off_max_temperature));
					handle_calibration();
					_mode = OFF;
				}
		}

		if (_last_mode <= OFF && _mode > OFF)
		{
			_readings.clear();
			_start_time = now;
			_temperature = thermocouple.readCelsius();
			pidTemperature.Reset();
			_readings.push_back(_temperature);
			last_m = now;
			last_log_m = now;
		} else if (_mode <= OFF && _last_mode > OFF)
		{
			_temperature = thermocouple.readCelsius();
			_readings.push_back(_temperature);
		}
		if (_onMode && _last_mode != _mode) {
			_onMode(_last_mode, _mode);
		}
		_last_mode = _mode;

		if (_mode >= ON)
		{
			if (now - last_m > READING_TIME)
			{
				_temperature = thermocouple.readCelsius();
				last_m = now;
				pidTemperature.Compute(now * 1000);
				Serial.println("Tt: " + String(_target) + "           T: " + String(_temperature) + "        Ctrl: " + String(_target_control));
			}

			if (now - last_log_m > TEMPERATURE_LOGGING_TIME) {
				_readings.push_back(_temperature);
				last_log_m = now;
				if (_onMeasure)
					_onMeasure(_readings, now);
			}
		}

		if (_heater && now - _start_time > MAX_ON_TIME && _temperature < SAFE_TEMPERATURE)
		{
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Heater time limit exceeded");
		}
		else if (_heater && _temperature > MAX_TEMPERATURE)
		{
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Temperature limit exceeded");
		} else if (_heater && now - _start_time > MIN_TEMP_RISE_TIME && _temperature - _readings[0] < MIN_TEMP_RISE) {
			_mode = ERROR_OFF;
			_heater = false;
			callMessage("ERROR: Temperature did not rise for " + String((int)(MIN_TEMP_RISE_TIME / 1000)) + " seconds");
		}

		digitalWrite(relay, _heater);
		digitalWrite(LED_BUILTIN, !_heater);

		if (_onHeater && _heater != _last_heater)
			_onHeater(_heater);
		_last_heater = _heater;
	}

	virtual void handle_pid(unsigned long now) {
		_heater = _heater ? _target_control > .5 - CONTROL_HYSTERISIS : _target_control > .5 + CONTROL_HYSTERISIS;
	}

	virtual void handle_reflow(unsigned long now) = 0;

	virtual void handle_calibration() {
		// TODO: calculate calibration parameters
		callMessage("INFO: Calibration data available!");
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

	const String& calibrationString() {
		return String("[0, 0, 0]"); // TODO
	}

	const char * translate_mode(MODE_t mode = UNKNOWN)
	{
		MODE_t m = mode = UNKNOWN ? _mode : mode;
		switch (m)
		{
			case INIT: return "Init"; break;
			case ON: return "ON"; break;
			case OFF: return "OFF"; break;
			case TARGET_OFF: return  "Reach Target"; break;
			case TARGET_PID: return  "Keep Target"; break;
			case TARGET_OFF_COOL: return  "Cooldown"; break;
			case ERROR_OFF: return  "Error"; break;
			case REFLOW: return  "Reflow"; break;
			case REFLOW_COOL: return  "Cooldown"; break;
		}
	}

private:
	void callMessage(const String& message) {
		if (_onMessage)
			_onMessage(message);
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
