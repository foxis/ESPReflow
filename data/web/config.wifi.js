var config = null;

function add_wifi(ssid, psk)
{
	var form = $("#wifi-config-form");
	var template = $("#templates").find("#wifi_ssid_psk_template").clone();
	template.id = "";
	template.show();
	template.find("#ssid").val(ssid);
	template.find("#psk").val(psk);
	template.find("#remove_wifi").click(function(){
		this.parentElement.parentElement.remove();
	});
	form.find("#networks_list").append(template);
}

function load_wifi_setup(){
	if (config == null) {
		$.ajax({
			 method: "GET",
			 dataType: "json",
			 url: get_url("config"),
			 success: function(data) {
				 config = data;

				 $("#networks_list").html("");

				 $("#hostname").val(config.hostname);
				 $("#otaPassword").val(config.otaPassword);
				 $("#user").val(config.user);
				 $("#password").val(config.password);

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

	form.find("#add_wifi").click(function() {
		add_wifi("", "");
	});

	form.find("#save").click(function() {
		config.hostname = $("#hostname").val();
		config.otaPassword = $("#otaPassword").val();
		config.user = $("#user").val();
		config.password = $("#password").val();

		config.networks = {};
		 $("#wifi-config-form").find("#networks_list").find(".form-row").each(function(){
			 var ssid = $(this).find("#ssid").val();
			 var psk = $(this).find("#psk").val();
			 config.networks[ssid] = psk;
		 });

		 $.ajax({
			 method: "POST",
			 dataType: "json",
			 url: get_url("config"),
			 data: JSON.stringify(config),
			 contentType: "application/json; charset=utf-8",
			 success: function(data) {
				 add_message("INFO: config.json saved!");
			 },
			 error: function(data) {
				 add_message("ERROR: failed saving config.json !");
			 }
		 });
	});

	form.find("#load").click(function() {
		config = null;
		load_wifi_setup();
	});

	form.find("#reboot").click(function() {
		ws.send("REBOOT");
	});
});
