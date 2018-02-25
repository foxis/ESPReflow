var profiles = null;
var parsed_profiles = null;

function add_PID(name, PID)
{
	var fields = {};

	if (name != null)
		fields = {
			'name': name,
			'P': PID[0],
			'I': PID[1],
			'D': PID[2],
		};

	clone_template("PID", fields);
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
	$(".menu-profile-select").click(function(){
		ws.send("profile:" + this.id);
	});
	$(".menu-mode-select").click(function(){
		ws.send(this.id);
	});

	$("#profiles-form").find("#PID-list").html("");
	$.each(data.PID, function(id, PID){
		add_PID(id, PID);
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
}

function load_profiles_setup() {
	if (profiles == null)
		update_profiles_and_modes();
}

function checkPID(value) {
	return parsed_profiles.PID[value] != null;
}
function checkStage(value) {
	return checkId(value) && value != "name" && value != "pid" && value != "stages";
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

function parse_profiles()
{
	parsed_profiles = profiles;
	parsed_profiles.errors = false;

	parsed_profiles.PID = {};
	$("#PID-list").find(".template-section").each(function(){
		var name = template_field(this, "name", checkEmpty);
		var P = template_field(this, "P", checkFloat, 0, 1000);
		var I = template_field(this, "I", checkFloat, 0, 1000);
		var D = template_field(this, "D", checkFloat, 0, 1000);

		if (P == null || I == null || D == null)
			parsed_profiles.errors = true;

		parsed_profiles.PID[name] = [P, I, D];
	});

	parsed_profiles.profiles = {};
	$("#Profile-list").find(".profile-template-section").each(function(){
		var id = template_field(this, "profile-id", checkId);
		var name = template_field(this, "profile-name", checkEmpty);

		parsed_profiles.profiles[id] = {
			"name": name,
		};

		$(this).find(".profile-stage-list .template-section").each(function(){
			var name = template_field(this, "stage-name", checkId);
			var target = template_field(this, "stage-target", checkFloat, 0, 600);
			var pid_name = template_field(this, "stage-PID-name", checkPID);
			var stay = template_field(this, "stage-stay", checkFloat, 0, 1200);

			if (name == null || target == null || pid_name == null || stay == null)
				parsed_profiles.errors = true;

			parsed_profiles.profiles[id][name] = {
				"target": target,
				"pid": pid_name,
				"stay": stay
			};
		});

		var stages = template_field(this, "profile-stages", checkStages, id);

		if (id == null || stages == null)
			parsed_profiles.errors = true;
		else
			parsed_profiles.profiles[id].stages = stages.replace(' ', '').split(",");
	});
}

$(document).ready(function(){
	var form = $("#profiles-form");

	form.find("#add-PID").click(function() {
		add_PID();
	});

	form.find("#add-Profile").click(function() {
		add_Profile();
	});

	form.find("#profiles-save").click(function() {
		parse_profiles();

		if (!parsed_profiles.errors) {
			delete parsed_profiles.errors;
			$.ajax({
				method: "POST",
				dataType: "json",
				url: get_url("profiles"),
				data: JSON.stringify(parsed_profiles),
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
});
