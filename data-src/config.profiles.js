var profiles = null;

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
			'stage-rate': stage.rate,
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
			'profile-PID-name': profile.pid,
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

function parse_profiles()
{
	profiles.PID = {};
	$("#PID-list").find(".template-section").each(function(){
		var name = template_field(this, "name");
		var P = template_field(this, "P");
		var I = template_field(this, "I");
		var D = template_field(this, "D");
		profiles.PID[name] = [P, I, D];
	});

	profiles.profiles = {};
	$("#Profile-list").find(".profile-template-section").each(function(){
		var id = template_field(this, "profile-id");
		var name = template_field(this, "profile-name");
		var pid_name = template_field(this, "profile-PID-name");
		var stages = template_field(this, "profile-stages");

		profiles.profiles[id] = {
			"name": name,
			"pid": pid_name,
			"stages": stages.replace(' ', '').split(",")
		};

		$(this).find(".profile-stage-list .template-section").each(function(){
			var name = template_field(this, "stage-name");
			var target = template_field(this, "stage-target");
			var rate = template_field(this, "stage-rate");
			var stay = template_field(this, "stage-stay");

			profiles.profiles[id][name] = {
				"target": target,
				"rate": rate,
				"stay": stay
			};
		});

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

		$.ajax({
			method: "POST",
			dataType: "json",
			url: get_url("profiles"),
			data: JSON.stringify(profiles),
			contentType: "application/json; charset=utf-8",
			success: function(data) {
				add_message(data.msg);
				update_profiles_and_modes_with_json(profiles);
			},
			error: function(data) {
				add_message("ERROR: failed saving profiles.json !");
			}
		});
	});

	form.find("#profiles-load").click(function() {
		update_profiles_and_modes();
	});
});
