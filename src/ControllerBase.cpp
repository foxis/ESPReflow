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

#include "ControllerBase.h"

ControllerBase::ControllerBase(Config& cfg) :
	config(cfg),
	pidTemperature(&_temperature, &_target_control, &_target, .5/DEFAULT_TEMP_RISE_AFTER_OFF, 5.0/DEFAULT_TEMP_RISE_AFTER_OFF, 4/DEFAULT_TEMP_RISE_AFTER_OFF, DIRECT),
	aTune(&_temperature, &_target_control, &_target, &_now, DIRECT),
	thermocouple(thermoCLK, thermoCS, thermoDO)
{
	_readings.reserve(15 * 60);

	_calP = .5/DEFAULT_TEMP_RISE_AFTER_OFF;
	_calD =  5.0/DEFAULT_TEMP_RISE_AFTER_OFF;
	_calI = 4/DEFAULT_TEMP_RISE_AFTER_OFF;

	pidTemperature.SetSampleTime(config.measureInterval * 1000);
  pidTemperature.SetMode(AUTOMATIC);
	pidTemperature.SetOutputLimits(0, 1);
	thermocouple.begin();
	Wire.begin(SDA, SCL);
	pca9536.begin(Wire);
	pca9536.pinMode(RELAY, OUTPUT);
	pca9536.pinMode(LED_RED, OUTPUT);
	pca9536.pinMode(LED_GREEN, OUTPUT);
	pca9536.pinMode(LED_BLUE, OUTPUT);

	pca9536.write(RELAY, LOW);
	pca9536.write(LED_RED, LOW);
	pca9536.write(LED_GREEN, LOW);
	pca9536.write(LED_BLUE, LOW);
	pca9536.write(LED_RED, HIGH);
	delay(100);
	pca9536.write(LED_GREEN, HIGH);
	delay(100);
	pca9536.write(LED_BLUE, HIGH);
	delay(100);
	pca9536.write(LED_RED, LOW);
	pca9536.write(LED_GREEN, LOW);
	pca9536.write(LED_BLUE, LOW);

	_mode = _last_mode = INIT;
	_temperature = 0;
	_target = DEFAULT_TARGET;
	_onMessage = NULL;
	_onMode = NULL;
	_onReadingsReport = NULL;
	_locked = false;
	_watchdog = 0;
	_last_heater_on = 0;

	_heater = _last_heater = false;

	pinMode(BUZZER_A, OUTPUT);
	pinMode(BUZZER_B, OUTPUT);

	tone(BUZZER_A, 440, 100);

	setPID("default");

	thermocouple.read();
	_temperature = thermocouple.getTemperature();

	S_printf("Current temperature: %f\n", _temperature);
	_readings.push_back(temperature_to_log(_temperature));
}


void ControllerBase::loop(unsigned long now)
{
	if (_last_mode == _mode && _mode >= ON)
	{
		if (now - last_m > config.measureInterval) {
			handle_measure(now);
		}
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

	pca9536.write(RELAY, _heater);
	pca9536.write(LED_RED, _heater);

	if (_onHeater && _heater != _last_heater)
		_onHeater(_heater);
	_last_heater = _heater;
}

PID& ControllerBase::setPID(float P, float I, float D) {
	resetPID();
	pidTemperature.SetTunings(P, I, D);
	return pidTemperature;
}

PID& ControllerBase::setPID(const String& name) {
	if (config.pid.find(name) == config.pid.end()) {
		callMessage("WARNING: No PID named '%s' found!!", name.c_str());
		return pidTemperature;
	} else {
		callMessage("INFO: Setting PID to '%s'.", name.c_str());
		return setPID(config.pid[name].P, config.pid[name].I, config.pid[name].D);
	}
}

void ControllerBase::resetPID() {
	pidTemperature.Reset();
}

String ControllerBase::calibrationString() {
	char str[64] = "";
	sprintf(str, "[%f, %f, %f]", _calP, _calI, _calD);
	return str;
}

const char * ControllerBase::translate_mode(MODE_t mode)
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

ControllerBase::Temperature_t ControllerBase::temperature_to_log(float t) {
	return t; // TODO: convert to 16bit fixed point
}

float ControllerBase::log_to_temperature(ControllerBase::Temperature_t t) {
	return isnan(t) ? 0.0 : t;
}

float ControllerBase::measure_temperature(unsigned long now) {
	if (now - last_m > config.measureInterval * 1.1) {
		last_m = now;
		thermocouple.read();
		return temperature(thermocouple.getTemperature());
	} else
		return temperature();
}
unsigned long ControllerBase::elapsed(unsigned long now) {
	return now - _start_time;
}

void ControllerBase::callMessage(const char * format, ...) {
	char buffer[512];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, sizeof(buffer), format, args);
	if (_onMessage)
		_onMessage(buffer);
	Serial.println(buffer);
  va_end (args);
}

void ControllerBase::reportReadings(unsigned long now)
{
	if (_onReadingsReport)
		_onReadingsReport(_readings, now);
}

void ControllerBase::handle_mode(unsigned long now) {
	if (_last_mode <= OFF && _mode > OFF)
	{
		_start_time = now;
		thermocouple.read();
		_temperature = thermocouple.getTemperature();
		pidTemperature.Reset();
		_readings.clear();
		_readings.push_back(temperature_to_log(_temperature));
		reportReadings(now - _start_time);
		last_m = now;
		last_log_m = now;
		_avg_rate = 0;

		if (_mode == CALIBRATE) {
			_target_control = config.tuner_init_output; 		// initial output
			//_temperature = _target;		// target temperature
			aTune.Cancel();						// just in case
			aTune.SetNoiseBand(config.tuner_noise_band);		// noise band +-1*C
			aTune.SetOutputStep(config.tuner_output_step);	// change output +-.5 around initial output
			aTune.SetControlType(config.tuner_id);
			aTune.SetLookbackSec(config.measureInterval * 100);
			aTune.SetSampleTime(config.measureInterval);

			_now = now;
			aTune.Runtime();				// initialize autotuner here, as later we give it actual readings
		} else if (_mode == TARGET_PID) {
			setPID("default");
			pidTemperature.Reset();
		}

	} else if (_mode <= OFF && _last_mode > OFF)
	{
		thermocouple.read();
		_temperature = thermocouple.getTemperature();
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

void ControllerBase::handle_measure(unsigned long now) {
	double last_temperature = _temperature;
	thermocouple.read();
	_temperature = thermocouple.getTemperature();
	double rate = 1000.0 * (_temperature - last_temperature) / (double)config.measureInterval;
	_avg_rate = _avg_rate * .9 + rate * .1;

	last_m = now;
	if (_mode != CALIBRATE) {
		pidTemperature.Compute(now * 1000);
		_target_control = max(_target_control, 0.0);
	}

	callMessage("DEBUG: PID: <code>e=%f     i=%f     d=%f       Tt=%f       T=%f     C=%f     rate=%f</code>",
			pidTemperature._e, pidTemperature._i, pidTemperature._d, (float)_target, (float)_temperature, (float)_target_control, (float)_avg_rate);

	if (now - last_log_m > config.reportInterval) {
		_readings.push_back(temperature_to_log(_temperature));
		last_log_m = now;
		reportReadings(now - _start_time);
	}
}

void ControllerBase::handle_safety(unsigned long now) {
	if (!_heater) {
		_last_heater_on = now;
		return;
	}

	double factor = _mode == CALIBRATE ? 6 : 1;

	if (now - _last_heater_on > MAX_ON_TIME * factor && _temperature > SAFE_TEMPERATURE)
	{
		mode(ERROR_OFF);
		_heater = false;
		callMessage("ERROR: Heater time limit exceeded (%i seconds)", (int)(MAX_ON_TIME / 1000));
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

void ControllerBase::handle_pid(unsigned long now) {
	_heater = now - last_m < config.measureInterval * _target_control && _target_control > CONTROL_HYSTERISIS ||
					now - last_m >= config.measureInterval * _target_control && _target_control > 1.0-CONTROL_HYSTERISIS;
}

void ControllerBase::handle_calibration(unsigned long now) {
	_now = now;
	if (aTune.Runtime()) {
			_heater = false;
			mode(CALIBRATE_COOL);
			_calP = aTune.GetKp();
			_calI = aTune.GetKi();
			_calD = aTune.GetKd();
			callMessage("INFO: Calibration data available! PID = [%f, %f, %f]", _calP, _calI, _calD);
	}

	handle_pid(now);

	_CALIBRATE_max_temperature = max(_CALIBRATE_max_temperature, _temperature);
}
