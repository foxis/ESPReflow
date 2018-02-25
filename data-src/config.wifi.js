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
				 $("#otaPassword-confirm").val(config.otaPassword);
				 $("#user").val(config.user);
				 $("#password").val(config.password);
				 $("#password-confirm").val(config.password);
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
		var otaPassword = $("#otaPassword").val();
		var otaPasswordConfirm = $("#otaPassword-confirm").val();
		var password = $("#password").val();
		var passwordConfirm = $("#password-confirm").val();
		var reportInterval = $("#reportInterval").val();
		var measureInterval = $("#measureInterval").val();
		var hostname = $("#hostname").val();
		var error = false;

		$("#wifi-config-form").find(":input").each(function(){
			$(this).removeClass("is-invalid");
			$(this).addClass("is-valid");
		});

		if (otaPassword != otaPasswordConfirm) {
			$("#otaPassword").addClass("is-invalid");
			$("#otaPassword-confirm").addClass("is-invalid");
			error = true;
		}
		if (password != passwordConfirm) {
			$("#password").addClass("is-invalid");
			$("#password-confirm").addClass("is-invalid");
			error = true;
		}
		if (!checkId(hostname)) {
			$("#hostname").addClass("is-invalid");
			error = true;
		}
		if (!checkInt(measureInterval, 10, 1000)) {
			$("#measureInterval").addClass("is-invalid");
			error = true;
		}
		if (!checkInt(reportInterval, 10, 10000)) {
			$("#reportInterval").addClass("is-invalid");
			error = true;
		}

		if (error) {
			add_message("WARNING: Form contains errors, please check");
			return;
		}

		config.hostname = hostname;
		config.otaPassword = otaPassword;
		config.user = $("#user").val();
		config.password = password;
		config.reportInterval = reportInterval;
		config.measureInterval = measureInterval;

		config.networks = {};
		 $("#networks-list").find(".template-section").each(function(){
			 var ssid = template_field(this, "ssid", checkEmpty);
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
