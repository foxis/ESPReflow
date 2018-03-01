#ifndef REFLOWCONTROLLER_v1_H
#define REFLOWCONTROLLER_v1_H

#include <ArduinoJson.h>
#include "ControllerBase.h"

class ReflowController : public ControllerBase
{
	unsigned long _stage_start;
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

		if (temperature() - target() > -2 && _stage_start == 0) {
			_stage_start = now;
			callMessage("INFO: Stage reached, waiting for " + String(current_stage->stay) + " seconds...");
		} else if (_stage_start != 0 && now - _stage_start > current_stage->stay * 1000) {
			stage(++current_stage);
		}

		handle_pid(now);
	}

	virtual String name() { return "Reflow Controller v1.0"; }

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
				callMessage("ERROR: Profile '" + name + "' has no stages!");
				return ControllerBase::profile();
			}
			mode(OFF);
			current_profile = p;
			current_stage = p->second.stages.begin();
			callMessage("INFO: Profile set to " + current_profile->second.name);
			return ControllerBase::profile(name);
		} else {
			mode(ERROR_OFF);
			callMessage("ERROR: No such profile '" + name + "' found!");
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
		if (current_stage != stage)
			callMessage("INFO: Stage " + current_stage->name + " finished.");
		current_stage = stage;
		if (stage != current_profile->second.stages.end()) {
			setPID(stage->pid);
			target(stage->target);
			_stage_start = 0;
			if (_onStage)
				_onStage(stage->name, (float)target());
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
