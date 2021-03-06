var ssid_item_scanwifi = -1;
var ssid_subitem_scanwifi = -1;
//scanwifi dialog
function onLoad(){
    refresh_scanwifi()

}

function refresh_scanwifi() {
    $('#AP_list').html("Scanning");
    var url = "/scanwifi";
    SendGetHttp(url, process_scanWifi_answer, getscanWififailed);
}

function process_scanWifi_answer(response_text) {
    var result = true;
    var content = "";
    try {
        var response = JSON.parse(response_text);
        if (typeof response.AP_LIST == 'undefined') {
            result = false;
        } else {
            var aplist = response.AP_LIST;
            //console.log("found " + aplist.length + " AP");
            aplist.sort(function(a, b) {
                return (parseInt(a.SIGNAL) < parseInt(b.SIGNAL)) ? -1 : (parseInt(a.SIGNAL) > parseInt(b.SIGNAL)) ? 1 : 0
            });

            for (var i = aplist.length - 1; i >= 0; i--) {
                const isProtected=(aplist[i].IS_PROTECTED=="1");
                content +='<button type="button" class="btn btn-outline-primary btn-block text-left"';
                content += "onclick='select_ap_ssid(\"" + aplist[i].SSID +"\","+isProtected + ");'>";
                if (isProtected) content += get_icon_svg("lock");
                content+=aplist[i].SSID;
                content+= " (" + aplist[i].SIGNAL+ ")";
                content+="</button>";
            }
        }
    } catch (e) {
        console.error("Parsing error:", e);
        result = false;
    }
    $('#AP_list').html( content);
    return result;
}

function select_ap_ssid(ssid_name, isProtected) {
    var url = "/connectwifi?ssid="+ssid_name;
    if(isProtected){
        var passwd = prompt("connect to "+ssid_name, "your password here");
        if(passwd==null){
            return;
        }
        url +="&pwd="+encodeURIComponent(passwd);
    }else{
        var r = confirm("connect to "+ssid_name+"?");
        if (r == false) {
           return;
        }
    }
    SendGetHttp(url);
 
}

function getscanWififailed(error_code, response) {

}
