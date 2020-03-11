#ifndef _FRONT_END_
#define _FRONT_END_
//converted  date time= 2019-11-05 12:26:11.479253
//cmd gen: D:\personal\git\WebDAV_air\libs\CFrontendFS.py ../frontend/ESP8266HTTPUpdateServerM.html
const char* ____frontend_ESP8266HTTPUpdateServerM_html_ PROGMEM ={
"<html>\n"\
"<body>\n"\
    "<form method=\'POST\' action=\'\' enctype=\'multipart/form-data\'>\n"\
          "<input type=\'hidden\' name=\'cmd\' value=\'0\'>\n"\
          "<input type=\'file\' name=\'update\'>\n"\
          "<input type=\'submit\' value=\'Update FW\'>\n"\
    "</form>\n"\
    "<form method=\'POST\' action=\'\' enctype=\'multipart/form-data\'>\n"\
          "<input type=\'hidden\' name=\'cmd\' value=\'100\'>\n"\
          "<input type=\'file\' name=\'update\'>\n"\
          "<input type=\'submit\' value=\'Update SPIFFS\'>\n"\
    "</form>\n"\
"</body>\n"\
"</html>\n"\
};

//converted list
//  CFrontendFS::add(server, "/ESP8266HTTPUpdateServerM.html", ct_html,____frontend_ESP8266HTTPUpdateServerM_html_);
#endif
//EOF
