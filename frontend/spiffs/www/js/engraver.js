function engraver_applyProbeAreaSize(response){
    var gcodeInfo;
    try {        
        gcodeInfo = JSON.parse(response);
        var grid=document.getElementById("id_ProbeAreaGrid").value;
        document.getElementById("id_ProbeAreaSizeX").value= Math.ceil(gcodeInfo.dimention.max.x);
        document.getElementById("id_ProbeAreaSizeY").value= Math.ceil(gcodeInfo.dimention.max.y);
    } catch (e) {
        console.error("Parsing error:", e);
    }
}

function engraver_ResultError(status,response){
    alert("Error:"+status+",text="+response);
}

function engraver_getGcodeInfo(filename,handler){
    if(filename!=""){
        var url="/gcodeInfo?file="+encodeURI(filename);
        SendGetHttp(url, handler,engraver_ResultError);
    }
}

function engraver_selectFile(filename){
    document.getElementById("id_fileSelected").value=filename;
    if(filename===""){
        document.getElementById("id_getSizeFromFile").classList.add("disabled");
        document.getElementById("id_levelMod").classList.add("disabled");
    }else{
        document.getElementById("id_getSizeFromFile").classList.remove("disabled");
        document.getElementById("id_levelMod").classList.remove("disabled");
    }
    
}


function init_engraver_panel(){
    probe_coner=0;
    engraver_selectFile("");
    document.getElementById("id_ProbeAreaSizeX").max= config.bed.size;
    document.getElementById("id_ProbeAreaSizeX").value=30;    
    document.getElementById("id_ProbeAreaSizeY").max= config.bed.size;
    document.getElementById("id_ProbeAreaSizeY").value=30;
    document.getElementById("id_feedRateXY").max= config.feedrate.max_xy;    
    document.getElementById("id_feedRateXY").value= config.probe.feedrate_xy;
    document.getElementById("id_feedRateProbe").max= config.feedrate.max_z;    
    document.getElementById("id_feedRateProbe").value= config.probe.feedrate_z;
    document.getElementById("id_safeZhop").max= config.bed.size;    
    document.getElementById("id_safeZhop").value= config.probe.safeZHop;
    document.getElementById("id_ProbeAreaGrid").value= config.probe.grid;
    document.getElementById("id_levelDelta").value= config.probe.area.distance;  
    document.getElementById("id_levelDelta").min= config.probe.distance_min;  
    document.getElementById("id_levelDelta").max= config.probe.distance_max;  
}

function engraver_probeArea(sizeX,sizeY,grid,levelDelta,feedRateXY,feedRateProbe){
    url="/probe?mode=area";
    url+="&sizeX="+parseInt(sizeX);
    url+="&sizeY="+parseInt(sizeY);
    url+="&grid="+parseInt(grid);
    url+="&levelDelta="+parseFloat(levelDelta);
    url+="&feedRateXY="+parseInt(feedRateXY);
    url+="&feedRateProbe="+parseInt(feedRateProbe);
    url+="&doubleTouch=0";
    SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
}

function engraver_higlightArea(sizeX,sizeY,grid,zHop,feedRateXY,feedRateProbe){
    url="/probe?mode=higlight";
    url+="&sizeX="+parseInt(sizeX);
    url+="&sizeY="+parseInt(sizeY);
    url+="&grid="+parseInt(grid);
    url+="&zHop="+parseFloat(zHop);
    url+="&feedRateXY="+parseInt(feedRateXY);
    url+="&feedRateProbe="+parseInt(feedRateProbe);
    SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
}

function engraver_probeStop(){
    url="/probe?mode=stop";
    SendGetHttp(url,on_httpStatusResponce,engraver_ResultError);
}

function engraver_GCodeLevelMod(filename){
    if(filename!=""){
        url="/levelmod?file="+filename;
        SendGetHttp(url);
    }
}

function engraver_probeGetResult(){
    url="/probe?mode=get";
    SendGetHttp(url,undefined ,engraver_ResultError);
}