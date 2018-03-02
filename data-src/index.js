var chart_config = {
	type: 'line',
	data: {
		datasets: [{
			labels: [],
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

var readingsChart = null;
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

function checkInt(value, min, max) {
  return !isNaN(value) &&
         parseInt(Number(value)) == value &&
         !isNaN(parseInt(value, 10)) && value <= max && value >= min;
}
function checkFloat(value, min, max) {
  return !isNaN(value) &&
         parseFloat(Number(value)) == value &&
         !isNaN(parseFloat(value, 10)) && value <= max && value >= min;
}
function checkId(value) {
	return value.match(/^[0-9a-zA-Z\-\.]+$/);
}
function checkEmpty(value) {
	return value != null && value != "";
}

function add_message(msg)
{
	var _info = "<span class=\"badge badge-pill badge-info\">INFO</span>";
	var _warn = "<span class=\"badge badge-pill badge-warning\">WARNING</span>";
	var _err = "<span class=\"badge badge-pill badge-danger\">ERROR</span>";
	var _dbg = "<span class=\"badge badge-pill badge-secondary\">DEBUG</span>";
	$("#messages").prepend("<tr><td>" + msg.replace("INFO:", _info).replace("WARNING:", _warn).replace("ERROR:", _err).replace("DEBUG:", _dbg) + "</td></tr>");

	if ($("#messages").find("tr").length > 1024) {
		$("#messages").find("tr").last().remove();
	}
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
		if ($(this).hasClass("disabled"))
			return false;
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

function template_field(what, field, validator, min, max, val)
{
	var field = $(what).find(":input.field-" + field);
	var value;
	value = val == null ? field.val() : field.val(val).val();
	if (validator != null) {
		if (validator(value, min, max)) {
			field.removeClass("is-invalid");
			field.addClass("is-valid");
		} else {
			field.removeClass("is-valid");
			field.addClass("is-invalid");
			return null;
		}
	}
	return value;
}

function ws_connect(url) {
	if (ws != null)
		return;

	ws = new WebSocket(url);

	ws.onopen = function()
	{
		$("#connected").text("Connected");
		$("#connected").removeClass("btn-danger");
		$("#connected").addClass("btn-success");
		ws.send("WATCHDOG");

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
			if (data.readings && data.times) {
					if (data.reset) {
						chart_config.data.labels = [];
						chart_config.data.datasets[0].data = [];
					}

					$.each(data.times, function(id, val) {
						chart_config.data.labels.push(val);
					});
					$.each(data.readings, function(id, val) {
						chart_config.data.datasets[0].data.push(val);
					});

					$("#current_temperature").text(data.readings[0]);
					readingsChart.update();

					ws.send("WATCHDOG");
			}
	}

	ws.onclose = function()
	{
		$("#connected").text("Lost Connection");
		$("#connected").removeClass("btn-success");
		$("#connected").addClass("btn-danger");
		var url = ws.url;
		ws = null;
		setTimeout(ws_connect, 1000, url);
	};

	ws.onerror = function() {
		$("#connected").text("Error Connecting");
		$("#connected").removeClass("btn-success");
		$("#connected").addClass("btn-danger");
		var url = ws.url;
		ws = null;
		setTimeout(ws_connect, 1000, url);
	}
}

function convertArrayOfObjectsToCSV(args) {
	// https://halistechnology.com/2015/05/28/use-javascript-to-export-your-data-as-csv/
  var result, ctr, keys, columnDelimiter, lineDelimiter, data;

  data = args.data || null;
  if (data == null || !data.length) {
      return null;
  }

  columnDelimiter = args.columnDelimiter || ',';
  lineDelimiter = args.lineDelimiter || '\n';

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

function page_is_ready(){
	var ctx = document.getElementById("readings");
	readingsChart = new Chart(ctx, chart_config);

	$("#download-temperature-log").click(function(){
		var data, link;
		var TempLog = [];

		var times = chart_config.data.labels;
		var temps = chart_config.data.datasets[0].data;
		var TempLog = times.map(function(val, i){ return {"Time": val, "Temperature": temps[i]}; });

		var csv = convertArrayOfObjectsToCSV({
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
	});

	$("#heater_on").click(function(){
		if (ws != null) ws.send("ON");
	});

	$("#heater_off").click(function(){
		if (ws != null) ws.send("OFF");
	});
	$("#heater_off1").click(function(){
		if (ws != null) ws.send("OFF");
	});
	$("#heater_cool").click(function(){
		if (ws != null) ws.send("COOLDOWN");
	});
	$("#target_temperature").change(function(){
		var temp = this.value;
		if (checkFloat(temp, 0, 1200)) {
			$(this).removeClass("is-invalid");
			if (ws != null) ws.send("target:" + temp);
		} else {
			$(this).addClass("is-invalid");
		}
	});

	$('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
		if (e.target.id == "nav-wifi-setup-tab"){
			load_wifi_setup();
		} else if (e.target.id == "nav-profiles-tab") {
			load_profiles_setup();
		} else if (e.target.id == "nav-license-tab") {
			$("#license-code").load("LICENSE.txt");
		}
	});

	$('#sidebarCollapse').on('click', function () {
		$('#sidebar').toggleClass('active');
	});

	config_init();
	profiles_init();

	ws_connect(get_url("ws", "ws"));

	check_if_ready();
}

var $ajax_loading = $('#loading');
$(document)
  .ajaxStart(function () {
		if (are_we_ready)
    	$ajax_loading.show();
  })
  .ajaxStop(function () {
		if (are_we_ready)
    	$ajax_loading.hide();
  })
	.ajaxError(function(e, xhr, options, error) {
		if (options.retry_count) {
			options.retry_count--;
			setTimeout($.ajax, 1000, options);
		}
});
