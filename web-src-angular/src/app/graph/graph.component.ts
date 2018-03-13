import { Component, OnInit, Input } from '@angular/core';
import { WebsocketService } from '../websocket.service';
import { ConfigsService } from '../configs.service';

@Component({
  selector: 'app-graph',
  templateUrl: './graph.component.html',
  styleUrls: ['./graph.component.css']
})
export class GraphComponent implements OnInit {

  constructor(public ws: WebsocketService, public configs: ConfigsService) { }

	lineChartOptions:any = {
    responsive: true
  };
  lineChartColors:Array<any> = [
    { // grey
      backgroundColor: 'transparent',
      borderColor: 'rgb(54, 162, 235)',
      pointBackgroundColor: 'rgba(148,159,177,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(148,159,177,0.8)'
    },
    { // dark grey
      backgroundColor: 'transparent',
      borderColor: 'rgb(255, 99, 132)',
      pointBackgroundColor: 'rgba(77,83,96,1)',
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: 'rgba(77,83,96,1)'
    },
  ];
  lineChartLegend:boolean = true;
  lineChartType:string = 'line';

  ngOnInit() {
  }

	public setMode(id, name) {
		this.ws.mode(id);
		this.ws.selected_mode = name;
	}

	public setProfile(id, name) {
		this.ws.profile(name);
	}

	public canSetProfile() {
		return this.ws.current_mode == "OFF";
	}
}
