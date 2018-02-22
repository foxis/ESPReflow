var profiles = null;

function add_PID(name, PID)
{
	var form = $("#profiles-form");
	var template = $("#templates").find("#PID-template").clone();

	template.id = "";
	template.show();

	if (name != null) {
		template.find("#name").val(name);
		template.find("#P").val(PID[0]);
		template.find("#I").val(PID[1]);
		template.find("#D").val(PID[2]);
	} else {
		// TODO: get calibration data if available
	}
	template.find("#remove_PID").click(function(){
		this.parentElement.parentElement.remove();
	});

	form.find("#PID-controllers").append(template);
}

function add_stage(profile, name, stage)
{
	var template = $("#templates").find("#profile-stage-template").clone();

	template.show();
	if (name != null) {
		template.find("#name").val(name);
		template.find("#target").val(stage.target);
		template.find("#rate").val(stage.rate);
		template.find("#stay").val(stage.stay);
	}

	template.find("#remove_Stage").click(function(){
		this.parentElement.parentElement.remove();
	});

	profile.find("#profile-stages-list").append(template);
}

function add_Profile(name, profile)
{
	var form = $("#profiles-form");
	var template = $("#templates").find("#Profile-template").clone();

	template.id = "";
	template.show();

	if (name != null)
	{
		template.find("#id").val(name);
		template.find("#name").val(profile.name);
		template.find("#PID-name").val(profile.pid);
		template.find("#stages").val(profile.stages.join());

		$.each(profile, function(id, stage){
			if (id != "name" && id != "pid" && id != "stages")
				add_stage(template, id, stage);
		});
	}

	template.find("#add_Stage").click(function(){
		add_stage($(this));
	});

	template.find("#remove_Profile").click(function(){
		this.parentElement.parentElement.parentElement.remove();
	});

	form.find("#Profile-list").append(template);
}

function update_profiles_and_modes_with_json(data)
{
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

	$("#profiles-form").find("#PID-controllers").html("");
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

$(document).ready(function(){
	var form = $("#profiles-form");

	form.find("#add_PID").click(function() {
		add_PID();
	});

	form.find("#add_Profile").click(function() {
		add_Profile();
	});

	form.find("#save").click(function() {

		$.ajax({
			method: "POST",
			dataType: "json",
			url: get_url("profiles"),
			data: JSON.stringify(profiles),
			contentType: "application/json; charset=utf-8",
			success: function(data) {
				add_message("INFO: profiles.json saved!");
				update_profiles_and_modes_with_json(profiles);
			},
			error: function(data) {
				add_message("ERROR: failed saving profiles.json !");
			}
		});
	});

	form.find("#load").click(function() {
		update_profiles_and_modes();
	});
});
