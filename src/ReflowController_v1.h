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

#ifndef REFLOWCONTROLLER_v1_H
#define REFLOWCONTROLLER_v1_H

#include <ArduinoJson.h>
#include "ControllerBase.h"

class ReflowController : public ControllerBase
{
	unsigned long _stage_start;
	double _start_temperature;
	Config::stages_iterator current_stage;
	Config::profiles_iterator current_profile;

public:
	ReflowController(Config& cfg) : ControllerBase(cfg)
	{
		_stage_start = 0;
		current_profile = config.profiles.end();
	}

	virtual void handle_reflow(unsigned long now) {
		if (current_profile == config.profiles.end()) {
			callMessage("ERROR: No Profile in reflow mode!");
			mode(ERROR_OFF);
			return;
		} else if (current_stage == current_profile->second.stages.end())
			return;

		float direction = current_stage->target >= _start_temperature ? 1 : -1;
		if (direction * (temperature() - current_stage->target) > 0 && _stage_start == 0) {
			_stage_start = now;
			if (current_stage->rate <= 0)
				target(current_stage->target);
			resetPID();
			callMessage("INFO: Stage reached, waiting for %f seconds...", current_stage->stay);
		} else if (_stage_start != 0 && now - _stage_start > current_stage->stay * 1000) {
			stage(++current_stage);
		}

		handle_pid(now);
	}

	virtual const char * name() { return "Reflow Controller v1.0"; }

	virtual void handle_measure(unsigned long now) {
		ControllerBase::handle_measure(now);
		if (ControllerBase::mode() == REFLOW) {
			handle_target(avg_rate());
		}
	}

	virtual void interpolate_target(float direction) {
		float next_temperature = target() + direction * current_stage->rate * config.measureInterval / 1000.0;
		float target_cap = current_stage->target;
		float T = direction > 0 ? min(next_temperature, target_cap) : max(next_temperature, target_cap);
		//callMessage("DEBUG: target: d=%f     t=%f    T=%f", direction, next_temperature, T);
		target(T);
		//resetPID();
	}
	virtual void handle_target(float current_rate) {
		if (current_profile != config.profiles.end()										// profile set
				&& current_stage != current_profile->second.stages.end()		// stage set
				&& current_stage->rate > 0																	// rate set
				&& abs(current_rate) <= current_stage->rate) {							// current average rate is within limits

			float direction = target() <= current_stage->target ? 1 : -1;

			if (direction * (temperature() - current_stage->target) < 0						// haven't reached stage target yet
					&& (direction * (temperature() - target()) > 0 || abs(current_rate) < current_stage->rate)) {										// reached interpolated target
				interpolate_target(direction);
			} /* else if (current_profile != config.profiles.end()
					&& current_stage != current_profile->second.stages.end()
					&& current_stage->rate > 0
					&& abs(current_rate) > current_stage->rate) {
					TODO: override _current_control
			} */
		}
	}

	// TODO:
	// * sanity checks
	// * profile check
	// * load profile on reflow start
	virtual MODE_t mode(MODE_t m) {
		if (m != REFLOW) {
			return ControllerBase::mode(m);
		}

		if (current_profile != config.profiles.end()) {
			stage(current_profile->second.stages.begin());
			return ControllerBase::mode(m);
		}
		return ControllerBase::mode(OFF);
	}

	virtual const String& profile(const String& name) {
		Config::profiles_iterator p = config.profiles.find(name);
		if (p != config.profiles.end()) {
			if (p->second.stages.begin() == p->second.stages.end()) {
				callMessage("ERROR: Profile '%s' has no stages!", name.c_str());
				return ControllerBase::profile();
			}
			mode(OFF);
			current_profile = p;
			current_stage = p->second.stages.begin();
			stage(current_stage);
			callMessage("INFO: Profile set to '%s'", current_profile->second.name.c_str());
			return ControllerBase::profile(name);
		} else {
			mode(ERROR_OFF);
			callMessage("ERROR: No such profile '%s' found!", name.c_str());
			return ControllerBase::profile();
		}
	}

	virtual const String& profile() {
		if (current_profile != config.profiles.end())
			return current_profile->second.name;
		else
			return ControllerBase::profile();
	}

	virtual const String& stage() {
		if (current_profile != config.profiles.end()) {
			return current_stage->name;
		} else
			return ControllerBase::stage();
	}

	virtual const String& stage(Config::stages_iterator stage) {
		Config::stages_iterator last_stage = current_stage;
		current_stage = stage;

		if (current_stage != last_stage)
			callMessage("INFO: Stage '%s' finished.", last_stage->name.c_str());
		if (stage != current_profile->second.stages.end()) {
			setPID(stage->pid);
			_stage_start = 0;
			_start_temperature = measure_temperature(millis());
			if (stage->rate > 0) {
				if (last_stage == current_stage) {
					target(_start_temperature);
				} else
					target(last_stage->target);
			} else {
				target(stage->target);
			}
			if (_onStage)
				_onStage(stage->name.c_str(), (float)stage->target);
			return ControllerBase::stage(stage->name);
		} else {
			mode(REFLOW_COOL);
			target(20);
			if (_onStage)
				_onStage("DONE", target());
			return ControllerBase::stage();
		}
	}

};

#endif
