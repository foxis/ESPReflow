import { Component, OnInit, Input, Output } from '@angular/core';
import { WebsocketService } from '../websocket.service';
import { ConfigsService } from '../configs.service';

@Component({
  selector: 'app-nav-bar',
  templateUrl: './nav-bar.component.html',
  styleUrls: ['./nav-bar.component.css']
})
export class NavBarComponent implements OnInit {
  constructor(public ws: WebsocketService, public configs: ConfigsService) { }

  ngOnInit() {
  }

	public setMode(id, name) {
		this.ws.mode(id);
		this.ws.selected_mode = name;
	}
}
