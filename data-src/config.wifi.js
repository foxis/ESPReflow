var config = null;

function add_wifi(ssid, psk)
{
	var fields = {
		'ssid': ssid,
		'psk': psk
	};

	clone_template("networks", fields);
}

function load_wifi_setup(){
	if (config == null) {
		$.ajax({
			 method: "GET",
			 dataType: "json",
			 url: get_url("config"),
			 success: function(data) {
				 config = data;

				 $("#networks-list").html("");

				 $("#hostname").val(config.hostname);
				 $("#otaPassword").val(config.otaPassword);
				 $("#user").val(config.user);
				 $("#password").val(config.password);
				 $("#reportInterval").val(config.reportInterval);
				 $("#measureInterval").val(config.measureInterval);

				 $.each(data.networks, function(ssid, psk){
					 add_wifi(ssid, psk);
				 });

				 add_message("INFO: config.json loaded!");
			 },
			 error: function(data) {
				 add_message("ERROR: config.json is corrupted!");
			 }
		});
	}
}

$(document).ready(function(){
	var form = $("#wifi-config-form");

	form.find("#add-wifi").click(function() {
		add_wifi("", "");
	});

	form.find("#config-save").click(function() {
		config.hostname = $("#hostname").val();
		config.otaPassword = $("#otaPassword").val();
		config.user = $("#user").val();
		config.password = $("#password").val();
		config.reportInterval = $("#reportInterval").val();
		config.measureInterval = $("#measureInterval").val();

		config.networks = {};
		 $("#networks-list").find(".template-section").each(function(){
			 var ssid = template_field(this, "ssid");
			 var psk = template_field(this, "psk");
			 config.networks[ssid] = psk;
		 });

		 $.ajax({
			 method: "POST",
			 dataType: "json",
			 url: get_url("config"),
			 data: JSON.stringify(config),
			 contentType: "application/json; charset=utf-8",
			 success: function(data) {
				 add_message(data.msg);
			 },
			 error: function(data) {
				 add_message("ERROR: failed saving config.json !");
			 }
		 });
	});

	form.find("#config-load").click(function() {
		config = null;
		load_wifi_setup();
	});

	form.find("#config-reboot").click(function() {
		ws.send("REBOOT");
	});
});
