import { Component, ChangeDetectorRef } from '@angular/core';
import { WebsocketService } from './websocket.service';
import { ConfigsService } from './configs.service';
import {MediaMatcher} from '@angular/cdk/layout';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss'],
	providers: [
		WebsocketService,
		ConfigsService
	]
})
export class AppComponent {
	config = {};
	profiles = [];
	modes = [];

	constructor(private ws: WebsocketService, private configs: ConfigsService, changeDetectorRef: ChangeDetectorRef, media: MediaMatcher) {
		this.mobileQuery = media.matchMedia('(max-width: 750px)');
    this._mobileQueryListener = () => changeDetectorRef.detectChanges();
    this.mobileQuery.addListener(this._mobileQueryListener);
	}

	mobileQuery: MediaQueryList;
	private _mobileQueryListener: () => void;

  ngOnDestroy(): void {
    this.mobileQuery.removeListener(this._mobileQueryListener);
  }

	ngOnInit() {
		this.ws.connect();
		this.configs.initialize();
	}

	public profilesLoaded(event) {
		this.modes = event.modes;
		this.profiles = event.profiles;
	}

	closeSideNav(start) {
		if (this.mobileQuery.matches)
			start.close();
	}
}
