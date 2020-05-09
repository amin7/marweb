# WebDAV_air
SdFat from esp lib
https://github.com/ardyesp/ESPWebDAV.git
https://arduinojson.org/v6/api/

SPIFFS 1MB
#Pins

Pin |   Function  | ESP-8266 Pin| used
TX  |   TXD       |     TXD|
RX  |   RXD       |     RXD|
A0  |   Analog input, max 3.3V input| 
D0  |   IO        | GPIO16|
D1  |   IO, SCL   | GPIO5|
D2  |   IO, SDA   | GPIO4|
D3  |   IO, 10k Pull-up|    GPIO0|
D4  |   IO, 10k Pull-up, BUILTIN_LED|   GPIO2| 
D5  |   IO, SCK   | GPIO14|
D6  |   IO, MISO  | GPIO12|relay
D7  |   IO, MOSI  | GPIO13|
D8  |   IO, 10k Pull-down, SS|  GPIO15|
G   |   Ground    | GND|
5V  |   5V        | -   |
3V3 |   3.3V      | 3.3V|
RST |   Reset     | RST |

card
1 (no wire)
2 CS (D1,D2)
3 MOSI (D7)
4 3.3v
5 SCK (D5)
6 GND
7 MISO (D6)
8

connector
1 GND 
2 card detect <470> (D0)-(GND if inserted)
3 MISO (D6) 
4 SCK  <1k> (D5)
5 3.3 v
6 MOSI <1k> (D7)
7 CS  <1k> (D1,D2)
---------
8 RX <470>RX<1k>GND
9 TX TX


[eclipse launch target]
board: D1 R2 & mini
CPU 160
VTable IRAM
Flash 4MB FS 2MB

[gtest]
https://github.com/google/googletest.git

To access the drive from Windows, type `\\192.168.1.106@8080\DavWWWRoot` at the Run prompt, or use Map Network Drive menu in Windows Explorer
#pecondition
put config.json in fd root folder
 { 'ssid':...,'pwd':...,'hostname':...,'mode':..}"
 mode= [WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2]
 
 testweb.py  -c ./webconfig.json
 
 CFrontendFS.py ../frontend/SPIFFSdlg.html.gz
 
 #create spiffs bin file
 https://github.com/esp8266/arduino-esp8266fs-plugin/issues/51
 
 D:\personal\arduino\packages\esp8266\tools\mkspiffs\2.5.0-4-b40a506\mkspiffs.exe  -p 256 -b 8192 -s 1028096 -c spiffs spiffs.bin 
 D:\personal\arduino\packages\esp8266\hardware\esp8266\2.6.3\tools\esptool\esptool.py -c esp8266 -b 921600 -p COM7 write_flash 0x200000 spiffs.bin
  
D:\user\workspace\arduinocdt\packages\esp8266\tools\mkspiffs\2.5.0-4-b40a506\mkspiffs.exe  -p 256 -b 8192 -s 1028096 -c spiffs spiffs.bin 
D:\user\workspace\arduinocdt\packages\esp8266\hardware\esp8266\2.6.2\tools\esptool\esptool.py -c esp8266 -b 921600 -p COM7 write_flash 0x200000 spiffs.bin

 ~/.platformio/packages/tool-mkspiffs/mkspiffs -p 256 -b 8192 -s 1028096 -c spiffs spiffs.bin
g++ -Wall -g -std=c++11 -DTEST CGcodeParser.cpp -o CGcodeParser 
 
 https://regex101.com/
 
./obj/unittests.exe --gtest_list_tests
./obj/unittests.exe --gtest_filter=CProbeAreaTest*
 
[todo]
lissen only  uart mode
pcb probe
send gcode on print with autoloevel correction
send status
web add cmd wait for end previous

/gcodeInfo?file=/SD/CFFFP_bedlevel.gcode
/probe?mode=once&zHop=1&feedRateZ=200&probeDistance=5&feedRateProbe=20&doubleTouch=0
/probe?mode=area&sizeX=50&sizeY=50&grid=10&levelDelta=1&feedRateXY=200&feedRateProbe=20&doubleTouch=0

run -e d1_mini
run --target upload -e d1_mini
