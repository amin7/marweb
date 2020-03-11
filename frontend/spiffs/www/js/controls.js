var interval_position = -1;
var controls_panel_inited=false;
const cmd_SetPosition000='G92 X0 Y0 Z0';
const cmd_SetPositionZ0='G92 Z0';
const cmd_AutoHome='G28';
const cmd_AutoHomeXY='G28 X Y';
const cmd_AutoHomeX='G28 X';
const cmd_AutoHomeY='G28 Y';
const cmd_AutoHomeZ='G28 Z';
const cmd_DisableSteppers ='M84';
const cmd_RelativePositioning ='G91';

function init_controls_panel() {    
    document.getElementById('control_xy_velocity').value =  config.feedrate.move_xy;
    document.getElementById('control_z_velocity').value =  config.feedrate.move_z;
    controls_panel_inited=true;
    get_Position();
}

function controls_UpdateStatus(status){
    if((typeof status.position)!='undefined'){
        if((typeof status.position.X)!='undefined')
            document.getElementById('control_x_position').innerHTML = status.position.X;
        if((typeof status.position.Y)!='undefined')
            document.getElementById('control_y_position').innerHTML = status.position.Y;
        if((typeof status.position.Z)!='undefined')
            document.getElementById('control_z_position').innerHTML = status.position.Z;
    }
}

function loadmacrolist(macrosName) {    
    var url = "/config/"+macrosName;
    SendGetHttp(url, Macro_build_list, processMacroGetFailed);
}

function Macro_build_list(response_text) {
    var macros_list=[];
    try {        
        macros_list = JSON.parse(response_text);
    } catch (e) {
        console.error("Parsing error:", e);
    }
    if((typeof macros_list.items)!='undefined' && (macros_list.items.length)){
        var content="";
        macros_list.items.forEach(function(item){
            content += control_build_macro_button(item);
        })
        document.getElementById('Macro_list').innerHTML = content;
        document.getElementById('MacroPanel').hidden=false;
    }
}

function processMacroGetFailed(errorcode, response) {
    console.log("Error " + errorcode + " : " + response);
    Macro_build_list("");
}

function get_Position() {
    var command = "M114";
    marlin_addCommand(command);
}

function SendZerocommand(cmd) {
    var command = "G10 L20 P0 " + cmd;
    marlin_addCommand(command);
}

function controls_GotoZero(fZZero) {
    var zHop = parseInt(document.getElementById('id_ProbeZhop').value);
    var feedrateZ = parseInt(document.getElementById('control_z_velocity').value);
    var feedrateXY = parseInt(document.getElementById('control_xy_velocity').value);
    var command =cmd_RelativePositioning+ "\nG0"+" F" + feedrateZ+" Z" + zHop;
    command+="\nG90\nG0"+" F" + feedrateXY+" X0 Y0";
    if(fZZero){
        command+="\nG0"+" F"+feedrateZ+" Z0"
    }
    command+="\nM114";
    marlin_addCommand(command);
}

function control_build_macro_button(item) {
    var content = "";
    content += "<button class='btn btn-outline-primary' type='text' ";
    content += "onclick='marlin_addCommand(\""+item.cmd+"\")'";
    content += "><span style='position:relative; top:3px;'>";     
    content += "</span>";
    if (item.name.length > 0) {
        content += "&nbsp;";
    }
    content += item.name;    
    content += "</button>";
    return content;
}

var ptm_active=false;
function controls_ProbeTargetMultiple_callback(responce){
    console.log(responce);
    if(ptm_active){
        var feedRateZ = parseInt(document.getElementById('control_z_velocity').value);
        var feedRateProbe=document.getElementById('id_ProbeFeed').value;
        var command = cmd_RelativePositioning+"\nG0"+" F" + feedRateZ+" Z" + config.probe.multiple.hop;
        command+= "\nG38.2 F" +parseInt(feedRateProbe)+ " Z" + (-1*(config.probe.multiple.hop+config.probe.multiple.distance));
        marlin_addCommand(command);
        marlin_addCommand("M114",controls_ProbeTargetMultiple_callback);
    }
}

function controls_ProbeTargetMultiple(fActive) {
    ptm_active=fActive;
    if(fActive){
        controls_ProbeTarget(document.getElementById('id_ProbeDistance').value,controls_ProbeTargetMultiple_callback)
    }else{
        clear_command_list();
        controls_gotoZHop(config.probe.multiple.hop);
    }
}

function controls_ProbeTarget(probeDistance_,callback) {
    var probeDistance=parseFloat(probeDistance_);
    console.log(probeDistance);
    var feedRateZ = parseInt(document.getElementById('control_z_velocity').value);
    var feedRateProbe=parseInt(document.getElementById('id_ProbeFeed').value);
    var command = cmd_RelativePositioning;
    if(-20>probeDistance){

        command += "\nG38.2 F" +feedRateZ+ " Z" + (probeDistance+20);
        probeDistance=-20;
    }
    if(-5>probeDistance){
        command += "\nG38.2 F" +feedRateProbe*2+ " Z" + (probeDistance+5);
        probeDistance=-5;
    }
    command += "\nG38.2 F" +feedRateProbe+ " Z" + probeDistance;
    marlin_addCommand(command);
    marlin_addCommand("M114",callback);
}

function controls_gotoToPos(posX,posY){
    console.log(posX,posY);
    if( 0>posX || 0>posY){
        return;
    }
    var feedrate = parseInt(document.getElementById('control_xy_velocity').value);
    var command = "G90\nG1"+" F" + feedrate+" X" + posX + " Y"+posY;
    command+="\nM114"
    marlin_addCommand(command);    
}

function controls_gotoZHop(deltaZ){
    console.log(deltaZ);
    var feedrate = parseInt(document.getElementById('control_z_velocity').value);
    var command = cmd_RelativePositioning+"\nG0"+" F" + feedrate+" Z" + deltaZ;
    command+="\nM114"
    marlin_addCommand(command);    
}
