import { Injectable } from '@angular/core';
import { Message, MessageDatabase, MessageDataSource } from './message';
import {get_url} from './mock.configs';

@Injectable()
export class WebsocketService {

  constructor() {
		this.url = get_url("ws", "ws");
		this.messages = new MessageDatabase();
		this.reset_readings();
	}

	// lineChart
	public readings = {
		readings: [
	    {data: [65, 59, 80, 81, 56, 55, 40], label: 'Probe'},
	    {data: [28, 48, 40, 19, 86, 27, 90], label: 'Target'},
	  ],
		times: []
	};

  public lineChartData:Array<any> = [
    {data: [65, 59, 80, 81, 56, 55, 40], label: 'Probe'},
    {data: [28, 48, 40, 19, 86, 27, 90], label: 'Target'},
  ];
  public lineChartLabels:Array<any> = [];

	connection_status = "Unknown";

	current_temperature = 0;
	current_target = 80;

	current_profile = "Select Profile";
	current_mode = "unknown";
	selected_mode = "Select Mode";
	current_stage = "unknown";

	heater = false;

	messages = new MessageDatabase;

	onReadings = () => {};

	private ws = null;
	private url = "";

	public send(data) {
		if (this.ws == null) {
			this.connection_status = "Connection Lost";
			return;
		}
		this.ws.send(data);
	}

	public mode(m) {
		this.send(m);
	}

	public profile(id) {
		this.send("profile:" + id)
	}

	public measure_current_temperature() {
		this.send("CURRENT-TEMPERATURE");
	}

	public target(t: number) {
		this.send("target:" + t);
	}

	public reboot() {
		this.send("REBOOT");
	}

	public download_temperature_log() {
		var data, link;
		var readings = this.readings.readings;
		var TempLog = this.readings.times.map(function(val, i){ return {"Time": val, "Temperature": readings[0].data[i], "Target": readings[1].data[i]}; });

		var csv = this.convertArrayOfObjectsToCSV({
				data: TempLog
		});

		if (!csv.match(/^data:text\/csv/i)) {
				csv = 'data:text/csv;charset=utf-8,' + csv;
		}
		data = encodeURI(csv);

		link = document.createElement('a');
		link.setAttribute('href', data);
		link.setAttribute('download', 'temperature_log.csv');
		link.click();
	}

	public measure() {
		this.send("");
	}

	public connect() {
		this.ws = new WebSocket(this.url);

		this.ws.onopen = () =>
		{
			this.send("WATCHDOG");
			this.onConnect();
		};

		this.ws.onmessage = (evt) =>
		{
				var data = JSON.parse(evt.data);

				if (data.profile)
					this.onProfile(data.profile);
				if (data.message)
					this.onMessage(data.message);
				if (data.stage)
					this.onStage(data.stage);
				if (data.heater != null)
					this.onHeater(data.heater);
				if (data.mode)
					this.onMode(data.mode);
				if (data.target)
					this.onTarget(data.target);
				if (data.readings && data.times) {
					if (data.reset) {
						this.reset_readings();
					}
					this.readings.times = this.readings.times.concat(data.times);
					this.readings.readings[0].data = this.readings.readings[0].data.concat(data.readings);
					this.readings.readings[1].data = this.readings.readings[1].data.concat(data.targets);
					this.current_temperature = data.readings[data.readings.length - 1];
					this.onReadings();
				}

				this.send("WATCHDOG");
		}

		this.ws.onclose = () =>
		{
			this.onClose();
			this.ws = null;
		};

		this.ws.onerror = () => {
			this.onError();
			this.ws = null;
		}
	}

	reconnect() {
		if (this.ws == null)
			this.connect();
	}

	isConnected() : boolean {
		return this.connection_status == "Connected";
	}

	canChangeTarget(): boolean {
		return this.ws != null && this.current_mode == "OFF";
	}
	canSaveProfiles(): boolean {
		return this.ws != null && this.current_mode == "OFF";
	}
	canReboot() : boolean {
		return this.ws != null && this.current_mode != "Reflow" && this.current_mode != "Calibrate";
	}
	canSaveSetup() : boolean {
		return this.ws != null && this.current_mode != "Reflow" && this.current_mode != "Calibrate";
	}

	private onProfile(profile) {
		this.current_profile = profile;
	};
	private onMessage(message) {
		this.messages.addMessage(new Message(message));
	};
	private onStage(stage) {
		this.current_stage = stage;
	};
	private onHeater(heater) {
		this.heater = heater;
	};
	private onMode(mode) {
		this.current_mode = mode;
	};
	private onTarget(target) {
		this.current_target = target;
	};
	private reset_readings() {
		this.readings.readings[0].data = [];
		this.readings.readings[1].data = [];
		this.readings.times = [];
	}

	private onConnect() {
		this.connection_status = "Connected";
	};
	private onClose() {
		this.connection_status = "Connection Lost";
	};
	private onError() {
		this.connection_status = "Error connecting";
	};

	private convertArrayOfObjectsToCSV(args) {
		// https://halistechnology.com/2015/05/28/use-javascript-to-export-your-data-as-csv/
	  var result, ctr, keys, columnDelimiter, lineDelimiter, data;

	  data = args.data || null;
	  if (data == null || !data.length) {
	      return null;
	  }

	  columnDelimiter = args.columnDelimiter || ',';
	  lineDelimiter = args.lineDelimiter || '\r\n';

	  keys = Object.keys(data[0]);

	  result = '';
	  result += keys.join(columnDelimiter);
	  result += lineDelimiter;

	  data.forEach(function(item) {
	      ctr = 0;
	      keys.forEach(function(key) {
	          if (ctr > 0) result += columnDelimiter;

	          result += item[key];
	          ctr++;
	      });
	      result += lineDelimiter;
	  });

	  return result;
	}
}
