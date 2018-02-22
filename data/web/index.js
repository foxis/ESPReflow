var chart_config = {
	type: 'line',
	data: {
		datasets: [{
			data: [],
			lineTension: 0,
			backgroundColor: 'transparent',
			borderColor: '#007bff',
			borderWidth: 4,
			pointBackgroundColor: '#007bff'
		}]
	},
	options: {
		scales: {
			yAxes: [{
				ticks: {
					beginAtZero: true
				}
			}]
		},
		legend: {
			display: false,
		}
	}
}

var ctx = document.getElementById("readings");
var readingsChart = new Chart(ctx, chart_config);
var ws = null;
var mode = null;
var are_we_ready = false;

function check_if_ready(ws_conn) {
	if (profiles && mode && ws) {
		$("#loading").fadeOut(100);
		are_we_ready = true;
	}
}

function get_url(url, proto="http")
{
	// relevant when developing locally without uploading SPIFFS
	var ip = '://192.168.1.68/'

	if (window.location.hostname != "")
		ip = "://" + window.location.hostname + '/';

	return proto + ip + url;
}

function add_message(msg)
{
	var _info = "<span class=\"badge badge-pill badge-info\">INFO</span>";
	var _warn = "<span class=\"badge badge-pill badge-warning\">WARNING</span>";
	var _err = "<span class=\"badge badge-pill badge-danger\">ERROR</span>";
	$("#messages").append("<tr><td>" + msg.replace("INFO:", _info).replace("WARNING:", _warn).replace("ERROR:", _err) + "</td></tr>");
}

$(document).ready(function(){
	ws = new WebSocket(get_url("ws", "ws"));

	ws.onopen = function()
	{
		$("#connected").text("Connected");
		$("#connected").removeClass("btn-danger");
		$("#connected").addClass("btn-success");
		ws.send("get-data");

		update_profiles_and_modes();
		check_if_ready();
	};

	ws.onmessage = function (evt)
	{
			var received_msg = evt.data;
			data = JSON.parse(evt.data);

			if (data.profile)
				$("#ddm_profile").text(data.profile);
			if (data.message)
				add_message(data.message);
			if (data.stage)
				$("#stage").text(data.stage);
			if (data.heater)
				$("#heater").addClass("btn-danger");
			else
				$("#heater").removeClass("btn-danger");
			if (data.mode) {
				mode = data.mode;
				$("#mode").text(data.mode);
				if (data.mode == "Keep Target" || data.mode == "Reflow" || data.mode == "Reach Target")
				{
					chart_config.data.labels = [];
					chart_config.data.datasets[0].data = [];
				}
			}
			if (data.target)
				$("#target_temperature").val(data.target);
			if (data.readings) {
					chart_config.data.labels.push(data.readings.time);

					chart_config.data.datasets[0].data.push(data.readings.temperature);
					$("#current_temperature").text(data.readings.temperature);
					readingsChart.update();
			}
	}

	ws.onclose = function()
	{
		$("#connected").text("Lost Connection");
		$("#connected").removeClass("btn-success");
		$("#connected").addClass("btn-danger");
	};
	$("#heater_on").click(function(){
		ws.send("ON");
	});

	$("#heater_off").click(function(){
		ws.send("OFF");
	});
	$("#heater_off1").click(function(){
		ws.send("OFF");
	});
	$("#target_temperature").change(function(){
		var temp = this.value;
		ws.send("target:" + temp);
	});

	$('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
		if (e.target.id == "nav-wifi-setup-tab"){
			load_wifi_setup();
		} else if (e.target.id == "nav-profiles-tab") {
			load_profiles_setup();
		}
	});
});

$(document).ready(function(){
	check_if_ready();
});

var $ajax_loading = $('#loading');
$(document)
  .ajaxStart(function () {
		if (are_we_ready)
    	$ajax_loading.show();
  })
  .ajaxStop(function () {
		if (are_we_ready)
    	$ajax_loading.hide();
  });
