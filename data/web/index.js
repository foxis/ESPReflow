var config = {
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
var readingsChart = new Chart(ctx, config);
var ws = null;
var profiles = null;
var mode = null;

function check_if_ready(ws_conn) {
	if (profiles && mode && ws)
		$("#loading").fadeOut(100);
}

function update_profiles_and_modes() {
	$.ajax({
		 method: "GET",
		 dataType: "json",
		 url: "http://192.168.1.68/profiles",
		 success: function(data) {
			 profiles = data;

			 var prl = $("#profiles_list");
			 var mdl = $("#modes_list");

			 prl.html("");
			 $.each(data.profiles, function(id, profile){
				 var s = "<a class=\"dropdown-item profile_select\" href=\"#\" id=\"" + id + "\">"+ profile.name +"</a>";
				 prl.append(s);
			 });
			 mdl.html("");
			 $.each(data.modes, function(id, name){
				 var s = "<a class=\"dropdown-item mode_select\" href=\"#\" id=\"" + id + "\">"+ name +"</a>";
				 mdl.append(s);
			 });
			 $(".profile_select").click(function(){
				 ws.send("profile:" + this.id);
			 });
			 $(".mode_select").click(function(){
				 ws.send(this.id);
				 $("#ddm_mode").text(this.text);
			 });
			 add_message("INFO: profiles.json loaded!");

			 check_if_ready();
		 },
		 error: function(data) {
			 add_message("ERROR: profiles.json is corrupted!");
		 }
	});
}

function add_message(msg)
{
	var _info = "<span class=\"badge badge-pill badge-info\">INFO</span>";
	var _warn = "<span class=\"badge badge-pill badge-warning\">WARNING</span>";
	var _err = "<span class=\"badge badge-pill badge-danger\">ERROR</span>";
	$("#messages").append("<tr><td>" + msg.replace("INFO:", _info).replace("WARNING:", _warn).replace("ERROR:", _err) + "</td></tr>");
}

$(document).ready(function(){
	//ws = new WebSocket('ws://' + window.location.hostname + '/ws');
	ws = new WebSocket('ws://192.168.1.68/ws');

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
					config.data.labels = [];
					config.data.datasets[0].data = [];
				}
			}
			if (data.target)
				$("#target_temperature").val(data.target);
			if (data.readings) {
					config.data.labels.push(data.readings.time);

					config.data.datasets[0].data.push(data.readings.temperature);
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

		} else if (e.target.id == "nav-profiles-tab") {

		}
	});
});

$(document).ready(function(){
	check_if_ready();
});
