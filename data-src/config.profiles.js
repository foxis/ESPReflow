var profiles = null;
var parsed_profiles = null;

function add_PID(name, PID)
{
	var fields = {};

	if (PID != null) {
		fields = {
			'P': PID[0],
			'I': PID[1],
			'D': PID[2],
		};
	}
	fields["name"] = name;
	if (name != null)
		$("#tuner-pid-list").append("<a class=\"dropdown-item\" href=\"#\">" + name + "</a>");

	var template = clone_template("PID", fields);
	if (name == "default") {
		template.find(".remove-section").addClass("disabled");
	}

	if (name == null) {
		$.ajax({
			method: "GET",
 		 	dataType: "json",
 		 	url: get_url("calibration"),
			context: template,
 		 	success: function(data) {
				if (data[0] != 0) {
					template_field(template, "name", null, null, null, "Calibrated");
					template_field(template, "P", null, null, null, data[0]);
					template_field(template, "I", null, null, null, data[1]);
					template_field(template, "D", null, null, null, data[2]);
				}
			}
		});
	}
}

function add_stage(profile, name, stage)
{
	var fields = {};
	if (name != null)
		fields = {
			'stage-name': name,
			'stage-target': stage.target,
			'stage-PID-name': stage.pid,
			'stage-stay': stage.stay,
			'stage-rate': stage.rate
		};

	clone_template("profile-stage", fields, profile);
}

function add_Profile(name, profile)
{
	var fields = {};
	if (name != null)
		fields = {
			'profile-id': name,
			'profile-name': profile.name,
			'profile-stages': profile.stages.join(),
		};

	var template = clone_template("Profile", fields);

	if (name != null)
	{
		$.each(profile, function(id, stage){
			if (id != "name" && id != "pid" && id != "stages")
				add_stage(template, id, stage);
		});
	}

	template.find(":button.add-Stage").click(function(){
		add_stage($(this).parent().parent().parent());
	});
}

function update_profiles_and_modes_with_json(data)
{
	var prl = $("#menu-profiles-list");
	var mdl = $("#menu-modes-list");
	var tl = $("#tuner-setup");

	$("#tuner-pid-list").html("");

	prl.html("");
	$.each(data.profiles, function(id, profile){
		var s = "<a class=\"dropdown-item menu-profile-select\" href=\"#\" id=\"" + id + "\">"+ profile.name +"</a>";
		prl.append(s);
	});
	mdl.html("");
	$.each(data.modes, function(id, name){
		var s = "<a class=\"dropdown-item menu-mode-select\" href=\"#\" id=\"" + id + "\">"+ name +"</a>";
		mdl.append(s);
	});
	$.each(data.tuners, function(id, val){
		var selected = "";
		if (data.tuner.id == val)
			selected = " selected=\"selected\"";
		var s = "<option value=\"" + val +"\"" + selected + ">" + id + "</option>";
		tl.append(s);
	});
	$("#tuner-init").val(data.tuner.init_output);
	$("#tuner-noise").val(data.tuner.noise_band);
	$("#tuner-step").val(data.tuner.output_step);
	$(".menu-profile-select").click(function(){
		ws.send("profile:" + this.id);
	});
	$(".menu-mode-select").click(function(){
		$("#ddm_mode").text(this.text);
		ws.send(this.id);
	});

	$("#profiles-form").find("#PID-list").html("");
	$.each(data.PID, function(id, PID){
		add_PID(id, PID);
	});

	$("#tuner-pid-list").find("a").click(function(){
		$("#tuner-pid-select").val($(this).text());
	});

	$("#profiles-form").find("#Profile-list").html("");
	$.each(data.profiles, function(id, profile){
		add_Profile(id, profile);
	});
}

function update_profiles_and_modes() {
	$.ajax({
		 method: "GET",
		 dataType: "json",
		 retry_count: 3,
		 url: get_url("profiles"),
		 success: function(data) {
			 profiles = data;

			 update_profiles_and_modes_with_json(data);
			 add_message("INFO: profiles.json loaded!");

			 check_if_ready();
		 },
		 error: function(data) {
			 add_message("ERROR: profiles.json is corrupted!");
		 }
	});

	$.ajax({
		method: "GET",
		dataType: "json",
		retry_count: 3,
		url: get_url("calibration"),
		success: function(data) {
			$("#tuner-pid-p").val(data[0]);
			$("#tuner-pid-i").val(data[1]);
			$("#tuner-pid-d").val(data[2]);
		}
	});
}

function load_profiles_setup() {
	update_profiles_and_modes();
}

function checkPID(value) {
	return parsed_profiles.PID[value] != null;
}
function checkStage(value, profile_id) {
	var unique = true;
	$.each(parsed_profiles.profiles[profile_id], function(id, val){
		if (id==value)
			unique = false;
	});
	return unique && checkId(value) && value != "name" && value != "pid" && value != "stages";
}
function checkStages(value, profile_id) {
	var stages = value.replace(' ', '').split(",");
	var error = false;
	$.each(stages, function(id, stage){
		if (!(parsed_profiles.profiles[profile_id][stage] && id != "name" && id != "pid" && id != "stages"))
			error = true;
	});
	return !error;
}

function checkPIDUnique(value) {
	var unique = true;
	$.each(parsed_profiles.PID, function(id, val){
		if (id==value)
			unique = false;
	});
	return unique && checkEmpty(value);
}

function checkProfile(value) {
	var unique = true;
	$.each(parsed_profiles.profiles, function(id, val){
		if (id==value)
			unique = false;
	});
	return unique && checkId(value);
}

function parse_profiles()
{
	parsed_profiles = profiles;
	parsed_profiles.errors = false;

	parsed_profiles.PID = {};
	$("#PID-list").find(".template-section").each(function(){
		var name = template_field(this, "name", checkPIDUnique);
		var P = template_field(this, "P", checkFloat, 0, 1000);
		var I = template_field(this, "I", checkFloat, 0, 1000);
		var D = template_field(this, "D", checkFloat, 0, 1000);

		if (P == null || I == null || D == null)
			parsed_profiles.errors = true;

		parsed_profiles.PID[name] = [parseFloat(P), parseFloat(I), parseFloat(D)];
	});

	parsed_profiles.tuner.id = validate_field($("#tuner-setup"));
	parsed_profiles.tuner.init_output = validate_field($("#tuner-init"), checkFloat, 0, 1);
	parsed_profiles.tuner.noise_band = validate_field($("#tuner-noise"), checkFloat, 0, 5);
	parsed_profiles.tuner.output_step = validate_field($("#tuner-step"), checkFloat, .1, 1);

	if (parsed_profiles.tuner.init_output == null || parsed_profiles.tuner.noise_band == null || parsed_profiles.tuner.output_step == null)
		parsed_profiles.errors = true;

	parsed_profiles.profiles = {};
	$("#Profile-list").find(".profile-template-section").each(function(){
		var id = template_field(this, "profile-id", checkProfile);
		var name = template_field(this, "profile-name", checkEmpty);

		parsed_profiles.profiles[id] = {
			"name": name,
		};

		$(this).find(".profile-stage-list .template-section").each(function(){
			var name = template_field(this, "stage-name", checkStage, id);
			var target = template_field(this, "stage-target", checkFloat, 0, 600);
			var pid_name = template_field(this, "stage-PID-name", checkPID);
			var stay = template_field(this, "stage-stay", checkFloat, 0, 1200);
			var rate = template_field(this, "stage-rate", checkFloat, 0, 5);

			if (name == null || target == null || pid_name == null || stay == null)
				parsed_profiles.errors = true;

			parsed_profiles.profiles[id][name] = {
				"pid": pid_name,
				"target": parseFloat(target),
				"stay": parseFloat(stay),
				"rate": parseFloat(rate)
			};
		});

		var stages = template_field(this, "profile-stages", checkStages, id);

		if (id == null || stages == null)
			parsed_profiles.errors = true;
		else
			parsed_profiles.profiles[id].stages = stages.replace(' ', '').split(",");
	});
}

function profiles_init(){
	var form = $("#profiles-form");

	form.find("#add-PID").click(function() {
		add_PID();
	});

	form.find("#add-Profile").click(function() {
		add_Profile();
	});

	$("#tuner-update-pid").click(function(){
		var name = validate_field($("#tuner-pid-select"), function(val){ return profiles.PID[val] != null; });
		if (name != null) {
			$("#PID-list").find(".template-section").each(function(){
				if ($(this).find(".field-name").val() == name) {
					$(this).find(".field-P").val($("#tuner-pid-p").val());
					$(this).find(".field-I").val($("#tuner-pid-i").val());
					$(this).find(".field-D").val($("#tuner-pid-d").val());
				}
			});
		}
	});
	$("#tuner-new-pid").click(function(){
		var name = validate_field($("#tuner-pid-select"), function(val){ return profiles.PID[val] == null; });
		if (name != null) {
			add_PID(name, [$("#tuner-pid-p").val(), $("#tuner-pid-i").val(), $("#tuner-pid-d").val()]);
		}
	});

	form.find("#profiles-save").click(function() {
		parse_profiles();

		if (!parsed_profiles.errors) {
			delete parsed_profiles.errors;
			$.ajax({
				method: "POST",
				dataType: "json",
				url: get_url("profiles"),
				data: JSON.stringify(parsed_profiles, null, 1),
				contentType: "application/json; charset=utf-8",
				success: function(data) {
					add_message(data.msg);
					update_profiles_and_modes_with_json(profiles);
				},
				error: function(data) {
					add_message("ERROR: failed saving profiles.json !");
				}
			});
		} else {
			add_message("WARNING: form contains errors, please check");
		}
	});

	form.find("#profiles-load").click(function() {
		update_profiles_and_modes();
	});
}
