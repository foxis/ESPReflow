import {environment} from '../environments/environment';

export const get_url = (url: string, proto?: string) =>
{
	var ip = environment.device_address;

	if (ip == null || window.location.hostname != "" && window.location.hostname != "localhost")
		ip = "://" + window.location.hostname + '/';

	return (proto == null ? "http" : proto) + ip + url;
}

export const mock_calibration = [1, 2, 3];
export const mock_config = {
	"networks": {
		"ssid": "passw"
	},
	"hostname": "ReflowControl",
	"user": "user",
	"password": "",
	"otaPassword": "",
	"measureInterval": 500,
	"reportInterval": 10000
};
export const mock_profiles = {
	"PID": {
		"default": [0.101859, 0.001773, 0.3901274],
		"IRHotPlate": [0.022143, 0.000082, 0.1967621],
		"IRHotPlate-Reflow": [0.042143, 0.0000082, 0.03967621]
	},
	"profiles": {
		"test": {
			"name": "Simple test profile",
			"stages": ["preheat", "soak", "reflow", "cooldown"],
			"preheat": {"pid": "IRHotPlate", "target": 40, "rate": 1, "stay": 40},
			"soak": {"pid": "IRHotPlate", "target": 60, "rate": 0.5, "stay": 40},
			"reflow": {"pid": "IRHotPlate-Reflow", "target": 100, "rate": 2, "stay": 15},
			"cooldown": {"pid": "IRHotPlate", "target": 50, "rate": 2, "stay": 40}
		},
		"test-no-rate": {
			"name": "Simple test profile",
			"stages": ["preheat", "soak", "reflow", "cooldown"],
			"preheat": {"pid": "IRHotPlate", "target": 40, "rate": 0, "stay": 40},
			"soak": {"pid": "IRHotPlate", "target": 60, "rate": 0, "stay": 40},
			"reflow": {"pid": "IRHotPlate-Reflow", "target": 100, "rate": 0, "stay": 15},
			"cooldown": {"pid": "IRHotPlate", "target": 50, "rate": 0, "stay": 40}
		},
		"leaded": {
			"name": "Simple low temp leaded paste profile",
			"stages": ["preheat", "soak", "reflow", "cooldown"],
			"preheat": {"pid": "IRHotPlate", "target": 130, "rate": 2, "stay": 40},
			"soak": {"pid": "IRHotPlate", "target": 180, "rate": 0.5, "stay": 0},
			"reflow": {"pid": "IRHotPlate-Reflow", "target": 230, "rate": 2, "stay": 15},
			"cooldown": {"pid": "IRHotPlate", "target": 50, "rate": 2, "stay": 0}
		},
		"leadfree": {
			"name": "Simple lead free paste profile",
			"stages": ["preheat", "soak", "reflow", "cooldown"],
			"preheat": {"pid": "IRHotPlate", "target": 150, "rate": 2, "stay": 0},
			"soak": {"pid": "IRHotPlate", "target": 200, "rate": 0.5, "stay": 0},
			"reflow": {"pid": "IRHotPlate-Reflow", "target": 250, "rate": 2, "stay": 15},
			"cooldown": {"pid": "IRHotPlate", "target": 50, "rate": 2, "stay": 0}
		}
	},
	"modes": {
		"REFLOW": "Reflow",
		"CALIBRATE": "Calibrate",
		"TARGET_PID": "Keep target"
	},
	"tuners": {
		"ZIEGLER_NICHOLS_PI": 0,
		"ZIEGLER_NICHOLS_PID": 1,
		"TYREUS_LUYBEN_PI": 2,
		"TYREUS_LUYBEN_PID": 3,
		"CIANCONE_MARLIN_PI": 4,
		"CIANCONE_MARLIN_PID": 5,
		"AMIGOF_PI": 6,
		"PESSEN_INTEGRAL_PID": 7,
		"SOME_OVERSHOOT_PID": 8,
		"NO_OVERSHOOT_PID": 9
	},
	"tuner": {
		"id": 8,
		"init_output": 0,
		"noise_band": 1,
		"output_step": 1
	}
};
