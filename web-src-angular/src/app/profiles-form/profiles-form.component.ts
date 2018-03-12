import { Component, OnInit } from '@angular/core';
import { ConfigsService, PID, Profile, Stage } from '../configs.service';
import { WebsocketService } from '../websocket.service';

@Component({
  selector: 'app-profiles-form',
  templateUrl: './profiles-form.component.html',
  styleUrls: ['./profiles-form.component.css']
})
export class ProfilesFormComponent implements OnInit {
  constructor(private ws: WebsocketService, public configs: ConfigsService) { }

  ngOnInit() {
  }

	selectedPID = "";

	addPID(name: string | "", d?: any) {
		if (!this.configs.PID.some(x => x.name == name)) {
			this.configs.PID.push(new PID(name, d));
		}
	}

	updatePID(name: string | "") {
		// update PID by selectedPID
		this.configs.PID.forEach(x => {
			if (x.name == name) {
				x.P = this.configs.calibration.P;
				x.I = this.configs.calibration.I;
				x.D = this.configs.calibration.D;
			}
		});
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

	canSave() : boolean {
		return this.ws.canSaveProfiles();
	}

	canAddPID(name: string) : boolean {
		let selectedPID = this.selectedPID;
		return this.configs.PID == null || this.configs.PID.find(x=>x.name == selectedPID) == null;
	}
}
