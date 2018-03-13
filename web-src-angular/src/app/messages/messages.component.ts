import { Component, OnInit, Input, ViewChild } from '@angular/core';
import { WebsocketService } from '../websocket.service';
import { MessageDataSource } from '../message';
import {MatPaginator} from '@angular/material';

@Component({
  selector: 'app-messages',
  templateUrl: './messages.component.html',
  styleUrls: ['./messages.component.css']
})
export class MessagesComponent implements OnInit {
	@Input() filter: string;
	@ViewChild(MatPaginator) paginator: MatPaginator;
	dataSource: MessageDataSource | null;
	displayedColumns = ['badge', 'text'];

  constructor(public ws: WebsocketService)
	{
	}

  ngOnInit() {
		this.dataSource = new MessageDataSource(this.ws.messages, this.paginator);
  }

	ngAfterViewInit() {
		this.dataSource.filter = this.filter;
	}
}
