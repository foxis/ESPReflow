import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import {get_url} from '../mock.configs'

@Component({
  selector: 'app-about',
  templateUrl: './about.component.html',
  styleUrls: ['./about.component.css']
})
export class AboutComponent implements OnInit {

  constructor(private http: HttpClient) { }

	license: string = "";
	thirdPartyLicense: string = "";

  ngOnInit() {
		this.http.get(get_url("LICENSE.txt")).subscribe(data => this.license = <string>data);
		this.http.get(get_url("3rdpartylicenses.txt")).subscribe(data => this.thirdPartyLicense = <string>data);
  }

}
