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
var clone_id = 0;

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

function clone_template(template_id, fields, root) {
	//increment
	clone_id++;
	var list_class = template_id + "-list";
	//loop through each input
	var section = $("#templates #" + template_id + "-template").clone().find(':input').each(function(){
			//set id to store the updated section number
			var newId = this.id + clone_id;
			//update for label
			$(this).prev().attr('for', newId);
			//update id
			this.id = newId;
	}).end()
	.appendTo( root == null ? "#" + list_class : root.find("." + list_class));
	section.removeAttr("id");
	$(section).find(":button.remove-section").click(function() {
		var parent = $(this).parent();
		//fade out section
		for (var i = 0; i < 5; i++)
		{
			if (parent.hasClass("template-section")) {
				$(parent).fadeOut(300, function(){
		        //remove parent element (main section)
		        $(this).remove();
		        return false;
		    });
				break;
			}
			parent = $(parent).parent();
		}
    return false;
	});

	$.each(fields, function(field, value){
		$(section).find(":input.field-"+field).val(value);
	});

	section.fadeIn(300);

	return section;
}

function template_field(what, field, value)
{
	return $(what).find(":input.field-" + field).val();
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
			if (data.heater != null) {
				if (data.heater)
					$("#heater").addClass("btn-danger");
				else
					$("#heater").removeClass("btn-danger");
			}
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

	$('#sidebarCollapse').on('click', function () {
		$('#sidebar').toggleClass('active');
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
