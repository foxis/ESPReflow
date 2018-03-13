import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs/Observable';
import { of } from 'rxjs/observable/of';
import {get_url, mock_config, mock_profiles, mock_calibration} from './mock.configs'
import { catchError, retry } from 'rxjs/operators';

export class Network {
	constructor(ssid?: string, passw?: string) {
		this.id = Network.count++;
		this.ssid = ssid;
		this.passw = passw;
	}

	static count: number = 0;

	id: number;
	ssid: string;
	passw: string;

	obj() {
		var o = {};
		o[this.ssid] = this.passw;
		return o;
	}
}

export class PID{
	constructor(name?: string, d?: any) {
		this.id = PID.count++;
		this.name = name;
		if (d) {
			this.P = d[0];
			this.I = d[1];
			this.D = d[2];
		}
	}
	static count: number = 0;

	id: number;
	name: string;
	P: number | 0;
	I: number | 0;
	D: number | 0;

	arr() {
		return [this.P, this.I, this.D];
	}

	obj() {
		var o = {};
		o[this.name] = this.arr();
		return o;
	}

	clone() {
		return new PID(this.name + " Copy", this.obj()[this.name]);
	}
}

export class Stage {
	constructor(name?: string, d?: {pid, target, rate, stay}) {
		this.id = Stage.count++;
		this.name = name;
		if (d) {
			this.pid = d.pid;
			this.target = d.target;
			this.rate = d.rate;
			this.stay = d.stay;
		}
	}
	static count: number = 0;

	id: number;
	name: string;
	pid: string | "";
	target: number | 0;
	rate: number | 0;
	stay: number | 0;

	obj() {
		var o = {};
		o[this.name] = {pid: this.pid, target: this.target, rate: this.rate, stay: this.stay};
		return o;
	}

	clone() {
		return new Stage(this.name + " Copy", this.obj()[this.name]);
	}
}

export class Profile {
	constructor(name?: string, d?: {name, stages}) {
		this.id = Profile.count++;
		this.name = name;
		if (d) {
			this.description = d.name;
			this.stage_list = d.stages.join(",");
			this.stages = Object.entries(d).filter(([k, v]) => k != "name" && k != "stages").map(([k, v]) => new Stage(k, v));
		}
	}

	static count: number = 0;
	id: number;
	name: string;
	description: string | "";
	stage_list: string | "";
	stages: Stage[];

	obj() {
		var o = { };
		o[this.name] = this.stages.reduce((acc, x) => Object.assign(acc, x.obj()), {name: this.description, stages: this.stage_list.replace(" ", "").split(",")});
		return o;
	}

	clone() {
		return new Profile(this.name + " Copy", this.obj()[this.name]);
	}
}

interface ITunerList {
	id: number;
	name: string;
}

interface ITuner {
	id:number;
	output: number;
	step: number;
	noise: number;
}

interface IMode {
	id: string;
	name: string;
}

@Injectable()
export class ConfigsService {

  constructor(private http: HttpClient)
	{
	}

	/* config.json */
	_config = null;
	networks: Network[] = [];
	hostname = "-";
	user = "-";
	password = "";
	otaPassword = "";
	measureInterval = 1;
	reportInterval = 1;
	password_confirm = "";
	otaPassword_confirm = "";

	/* profiles.json */
	_profiles = null;
	PID: PID[] = [];
	profiles: Profile[] = [];
	// readonly
	modes: IMode[] = [];
	tuners: ITunerList[];
	tuner: ITuner = {id:0, output:0, step:0, noise:0};

	/* calibration */
	calibration = new PID("", [0,0,0]);

	fetch_config() {
		return this.http.get(get_url("config")).pipe(retry(5));
		//return of(mock_config);
	}
	fetch_profiles() {
		return this.http.get(get_url("profiles")).pipe(retry(5));
		//return of(mock_profiles);
	}
	fetch_calibration() {
		return this.http.get(get_url("calibration")).pipe(retry(5));
		//return of(mock_calibration);
	}
	post_config() {
		this.http.post(get_url("config"), this.serialize_config()).subscribe();
	}
	post_profiles() {
		this.http.post(get_url("profiles"), this.serialize_profiles()).subscribe();
	}

	initialize() {
		this.fetch_config().subscribe(data => {
			this.deserialize_config(data);
			this.fetch_profiles().subscribe(data => {
				this.deserialize_profiles(data);
				this.fetch_calibration().subscribe(data => this.calibration = new PID("Calibration", data));
			});
		});
	}

	load_config() {
		return this.fetch_config().subscribe(data => this.deserialize_config(data));
	}
	load_profiles() {
		return this.fetch_profiles().subscribe(data => this.deserialize_profiles(data));
	}
	load_calibration() {
		return this.fetch_calibration().subscribe(data => {this.calibration = new PID("Calibration", data)});
	}

	serialize_config() {
		return JSON.stringify({
			"networks": this.networks.reduce((acc, val) => Object.assign(acc, val.obj()), {}),
			"hostname": this.hostname,
			"user": this.user,
			"password": this.password,
			"otaPassword": this.otaPassword,
			"measureInterval": this.measureInterval,
			"reportInterval": this.reportInterval
		});
	}
	deserialize_config(data) {
		this.hostname = data.hostname;
		this.user = data.user;
		this.password = data.password;
		this.password_confirm = data.password;
		this.otaPassword = data.otaPassword;
		this.otaPassword_confirm = data.otaPassword;
		this.reportInterval = data.reportInterval;
		this.measureInterval = data.measureInterval;
		this.networks = Object.entries(data.networks).map(([key, val]) => new Network(key, val));
	}

	serialize_profiles() {
		if (this._profiles == null) return "{}";
		var data = this._profiles;

		data.PID = this.PID.reduce((acc, x) => Object.assign(acc, x.obj()), {});
		data.profiles = this.profiles.reduce((acc, x) => Object.assign(acc, x.obj()), {});
		data.tuner.id = <number>this.tuner.id;
		data.tuner.init_output = <number>this.tuner.output;
		data.tuner.noise_band = <number>this.tuner.noise;
		data.tuner.output_step = <number>this.tuner.step;

		return JSON.stringify(data);
	}
	deserialize_profiles(data) {
		this.tuner = {
			id: parseInt(data.tuner.id),
			output: parseFloat(data.tuner.init_output),
			noise: parseFloat(data.tuner.noise_band),
			step: parseFloat(data.tuner.output_step)
		};
		this.tuners = Object.entries(data.tuners).map(([key, val]) => { return {id:val, name:key}; });
		this.modes = Object.entries(data.modes).map(([key, val]) => { return {id:key, name:val}; });
		this.PID = Object.entries(data.PID).map(([key, val]) => new PID(key, val));
		this.profiles = Object.entries(data.profiles).map(([key, val]) => new Profile(key, val));
		this._profiles = data;
	}
}
