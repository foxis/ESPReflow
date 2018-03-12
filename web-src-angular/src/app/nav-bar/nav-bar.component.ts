import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';
import { WebsocketService } from '../websocket.service';
import { ConfigsService } from '../configs.service';

@Component({
  selector: 'app-nav-bar',
  templateUrl: './nav-bar.component.html',
  styleUrls: ['./nav-bar.component.css']
})
export class NavBarComponent implements OnInit {
  constructor(public ws: WebsocketService, public configs: ConfigsService) { }

	@Input() isMobile: boolean;
	@Output() toggle: EventEmitter<any> = new EventEmitter();

  ngOnInit() {
  }

	toggleEmit() {
		this.toggle.emit();
	}
}
