import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';
import { ConfigsService, PID, Profile, Stage } from '../configs.service';

@Component({
  selector: 'app-profiles-form',
  templateUrl: './profiles-form.component.html',
  styleUrls: ['./profiles-form.component.css']
})
export class ProfilesFormComponent implements OnInit {
  constructor(private configs: ConfigsService) { }

  ngOnInit() {
  }

	selectedPID = "";

	addPID(copy) {
		this.configs.PID.push(new PID());
	}

	updatePID(i: number) {
		// update PID by selectedPID
		//this.PID[i] = this.makeNewPID(true, this.PID[i].id);
	}

	removePID(i: number) {
		this.configs.PID.splice(i, 1);
	}
	clonePID(i: number) {
		this.configs.PID.push(this.configs.PID[i].clone());
	}

	addProfile() {
		this.configs.profiles.push(new Profile());
	}
	removeProfile(i: number) {
		this.configs.profiles.splice(i, 1);
	}
	cloneProfile(i: number, j: number) {
		this.configs.profiles.push(this.configs.profiles[i].clone());
	}

	addStage(i: number) {
		this.configs.profiles[i].stages.push(new Stage());
	}
	cloneStage(i: number, j: number) {
		this.configs.profiles[i].stages.push(this.configs.profiles[i].stages[j].clone());
	}
	removeStage(i: number, j: number) {
		this.configs.profiles[i].stages.splice(j, 1);
	}

	selectPID(name: string, id: number) {
		this.selectedPID = name;
	}

	load() {
		this.configs.load_profiles();
	}
	load_cal() {
		this.configs.load_calibration();
	}

	save() {
		this.configs.post_profiles();
	}
}
