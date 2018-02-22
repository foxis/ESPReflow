#ifndef REFLOWCONTROLLER_v1_H
#define REFLOWCONTROLLER_v1_H

#include <ArduinoJson.h>
#include "ControllerBase.h"

class ReflowController : public ControllerBase
{
	//JsonDynamicBuffer jsonBuffer;
	//JsonObject &profiles_json;
	float _stage_start;

public:
	ReflowController() : ControllerBase()
	{
		_stage_start = 0;
	}

	virtual void handle_reflow(unsigned long now) {

	}

	virtual String name() { return "Reflow Controller v1.0"; }

	// TODO:
	// * sanity checks
	// * profile check
	// * load profile on reflow start
	virtual MODE_t mode(MODE_t m) {
		return ControllerBase::mode(m);
	}

	virtual float target(float t) {
		return ControllerBase::target(t);
	}

	virtual const String& profile(const String& s) {
		//load_profiles();
		return ControllerBase::profile(s);
	}

	virtual const String& profile() {
		//load_profiles();
		return ControllerBase::profile();
	}
};

#endif
