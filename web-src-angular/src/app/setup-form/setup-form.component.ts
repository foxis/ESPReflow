import { Component, OnInit } from '@angular/core';
import { ConfigsService, Network } from '../configs.service';
import { WebsocketService } from '../websocket.service';

@Component({
  selector: 'app-setup-form',
  templateUrl: './setup-form.component.html',
  styleUrls: ['./setup-form.component.css']
})
export class SetupFormComponent implements OnInit {
  constructor(public ws: WebsocketService, public configs: ConfigsService) { }

  ngOnInit() {
  }

	valid = false;

	addWifi(ssid) {
		this.configs.networks.push(new Network(ssid, ""));
	}
	removeWifi(i) {
		this.configs.networks.splice(i, 1);
	}

	load() {
		this.configs.load_config();
	}
	save() {
		this.configs.post_config();
	}
	reboot() {
		this.ws.reboot();
	}

	canSave(): boolean {
		return this.ws.canSaveSetup();
	}
	canReboot() : boolean {
		return this.ws.canReboot();
	}
}
