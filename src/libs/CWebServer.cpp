/*
 *  origin FSWebServer - Example WebServer with SPIFFS backend for esp8266
*/
#include <sstream>
#include <set>
#include "CWebServer.h"
#include "CGcodeBedLevelProducer.h"
#include "CWebFileListSD.h"
#include "CGcodeParser.h"
#include "misk.h"
#include "CBedLevel.h"
#include "logs.h"

typedef enum {
    file_No = 0,
    file_Present,
    file_PresentGZ
} tExistsFile;

tExistsFile isExistsFile(const String &path)
{
    const auto pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz))
        return file_PresentGZ;
    if (SPIFFS.exists(path))
        return file_Present;
    return file_No;
}

bool CWebServer::handleFileRead(const String &path)
{
    DBG_PRINT(F("handleFileRead: "));
    DBG_PRINT(path);
    DBG_PRINT(" ");
    const auto contentType = getMimeType(path);
    
    File file;
    switch (isExistsFile(path)) {
        case file_No:
            DBG_PRINTLN("no file")
            return false;
        case file_Present:
            file = SPIFFS.open(path, "r");
            break;
        case file_PresentGZ:
            file = SPIFFS.open(path + ".gz", "r");
            break;
        default:
            return false;
    }
    const auto count = m_server.streamFile(file, contentType);
    DBG_PRINTLN(count);
    file.close();
    return true;
}

void CWebServer::handleFile()
{
    auto path = m_server.uri();

    if (path == "/") //homepage
    {
        path = "/index.html";
    }

    if (file_No != isExistsFile(path))
    {
        handleFileRead(path);
        return;
    }

    path = "/www" + path;
    if (file_No != isExistsFile(path))
    {
        handleFileRead(path);
        return;
    }

    ERR_LOG(String("FileNotFound :") + m_server.uri());
    webRetResult(m_server, er_fileNotFound);
}

void CWebServer::handleProbesGet() {
    if (m_ProbeArea.getMode() != CMarlinRun::paDone) {
        webRetResult(m_server, er_incorrectMode);
        return;
    }
    DBG_FUNK_LINE();
    const auto sizeX = m_ProbeArea.getSizeX();
    const auto sizeY = m_ProbeArea.getSizeY();
    const auto grid = m_ProbeArea.getGrid();
    const auto ZheighArray = m_ProbeArea.getZheighArray();
#if 1
#else
    grid = 1;
    for (sizeY = 0; sizeY < 20; sizeY += grid) {
        for (sizeX = 0; sizeX < 20; sizeX += grid) {
            ZheighArray.push_back(sizeX);
        }
    }
#endif

    m_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    m_server.sendHeader("Content-Type", "application/json", true);
    m_server.sendHeader("Cache-Control", "no-cache");
    std::ostringstream jsonbeg;
    jsonbeg << "{";
    jsonbeg << "\"sizeX\":" << sizeX << ",";
    jsonbeg << "\"sizeY\":" << sizeY << ",";
    jsonbeg << "\"grid\":" << grid << ",";
    jsonbeg << "\"zHeigh\": [";
    m_server.sendContent(jsonbeg.str().c_str());

    DBG_PRINTLN(ZheighArray.size())
    auto add_comma = false;
    std::ostringstream row;
    for (auto val : ZheighArray)
    {
        if (add_comma) {
            row << ",";
        }
        add_comma = true;
        row << val;
    }
    m_server.sendContent(row.str().c_str());

    m_server.sendContent("]}");
    m_server.sendContent("");
}

void CWebServer::handleProbes()
{
    auto retVal = er_last;
    do
    {
        if (!m_server.hasArg("mode"))
        {
            retVal = er_no_parameters;
            break;
        }
        const auto newMode = m_server.arg("mode");
        DBG_PRINT("newMode ");
        DBG_PRINTLN(newMode);

        if (newMode == "stop")
        {
            m_ProbeArea.stop();
            retVal = er_ok;
            break;
        }

        if (m_ProbeArea.getMode() == CMarlinRun::paRun)
        {
            retVal = er_incorrectMode;
            break;
        }
        if (newMode == "area")
        {
            if (!m_server.hasArg("sizeX") ||
            !m_server.hasArg("sizeY") ||
            !m_server.hasArg("grid") ||
            !m_server.hasArg("levelDelta") ||
            !m_server.hasArg("feedRateXY") ||
            !m_server.hasArg("feedRateProbe") ||
            !m_server.hasArg("doubleTouch"))
            {
                retVal = er_no_parameters;
                break;
            }
            const auto sizeX=m_server.arg("sizeX").toInt();
            const auto sizeY=m_server.arg("sizeY").toInt();
            const auto grid=m_server.arg("grid") .toInt();
            const auto levelDelta=m_server.arg("levelDelta") .toDouble();
            const auto feedRateXY=m_server.arg("feedRateXY") .toInt();
            const auto feedRateProbe=m_server.arg("feedRateProbe") .toInt();
            const auto doubleTouch=m_server.arg("doubleTouch").toInt();
            if (!m_ProbeArea.run(sizeX, sizeY, grid, levelDelta, feedRateXY, feedRateProbe, doubleTouch))
           {
                retVal = er_errorResult;
                break;
             }
            retVal = er_ok;
            break;
        }
        if (newMode == "get")
        {
            handleProbesGet();
            return;
        }
        else
        {
            retVal = er_incorrectMode;
        }
    } while (0);
    webRetResult(m_server, retVal);
}

void CWebServer::handleGetGCodeInfo()
{
    if (!m_server.hasArg("file"))
    {

        webRetResult(m_server, er_no_parameters);
        return;
    }

    if (!m_sdCnt.requestSDcontrol())
    {
        webRetResult(m_server, err_MarlinRead);
        return;
    }
    m_server.sendHeader("Cache-Control", "no-cache");
    const auto fileName = m_server.arg("file");
    DBG_PRINTLN(fileName);
    sdfat::SdFile gcode(fileName.c_str(), sdfat::O_RDONLY);

    // check for open error
    if (!gcode.isOpen())
    {
        webRetResult(m_server, er_fileNotFound);
        return;
    }

    CGcodeParser parser;
    CGcodeInfo info(parser);
    int16_t n;
    char buff[256];
    uint32_t yelddelay = 0;
    while ((n = gcode.fgets(buff, sizeof(buff))) > 0)
    {
        const auto loc_millis = millis();
        if (loc_millis > yelddelay)
        {
            yelddelay = loc_millis + 300;
            yield();
        }
        if (sizeof(buff) == n)
        {
            webRetResult(m_server, er_BuffOverflow);
            return;
        }
        if ('\n' == buff[n - 1])
        {
            buff[n - 1] = 0; //remove end line
        }
        if (false == parser.addCmd(buff))
        {
            String err = "parce err ";
            err += "line=";
            err += parser.getLineTotal();
            err += ",data=";
            err += buff;
            m_server.send(400, "text/plain", err);
            return;
        }
    }
    gcode.close();
    StaticJsonDocument < 500 > gcodeInfo;
    auto root = gcodeInfo.to<JsonObject>();
    root["file"] = fileName.c_str();
    auto json_dimention = root.createNestedObject("dimention");
    auto json_dimention_max = json_dimention.createNestedObject("max");
    auto json_dimention_min = json_dimention.createNestedObject("min");
    json_dimention_max["x"] = info.getMax().getX();
    json_dimention_max["y"] = info.getMax().getY();
    json_dimention_max["z"] = info.getMax().getZ();
    json_dimention_min["x"] = info.getMin().getX();
    json_dimention_min["y"] = info.getMin().getY();
    json_dimention_min["z"] = info.getMin().getZ();
    root["lineTotal"] = parser.getLineTotal();
    root["lineProcessed"] = parser.getLineProcessed();

    m_server.send(200, "application/json", gcodeInfo.as<String>());
}

void CWebServer::handleLevelMod()
{
    if (!m_sdCnt.requestSDcontrol())
    {
        webRetResult(m_server, err_MarlinRead);
        return;
    }
    if (CMarlinRun::paDone != m_ProbeArea.getMode())
    {
        webRetResult(m_server, er_incorrectMode);
        return;
    }
    m_server.sendHeader("Cache-Control", "no-cache");
    if (!m_server.hasArg("file"))
    {
        webRetResult(m_server, er_no_parameters);
        return;
    }

    const auto fileName = m_server.arg("file");
    DBG_PRINTLN(fileName);
    auto gcode = sdfat::SdFile(fileName.c_str(), sdfat::O_RDONLY);
    auto leveledName = std::string(fileName.c_str());
    leveledName.insert(leveledName.find_last_of('.'), "_");

    auto gcode_leveled = sdfat::SdFile(leveledName.c_str(), sdfat::O_CREAT | sdfat::O_WRITE);

    // check for open error
    if (!gcode.isOpen() || !gcode_leveled.isOpen())
    {
        webRetResult(m_server, er_fileNotFound);
        return;
    }

    CGcodeParser parser;
    CGcodeBedLevelProducer producer(parser, [&](const std::string &str) -> bool
    {
        if (-1 == gcode_leveled.write(str.c_str())) {
            return false;
        }
        return true;
    }, m_ProbeArea);
    int16_t n;
    char buff[256];
    uint32_t yelddelay = 0;
    while ((n = gcode.fgets(buff, sizeof(buff))) > 0)
    {
        const auto loc_millis = millis();
        if (loc_millis > yelddelay)
        {
            yelddelay = loc_millis + 300;
            yield();
        }
        if (sizeof(buff) == n)
        {
            webRetResult(m_server, er_BuffOverflow);
            return;
        }
        if ('\n' == buff[n - 1])
        {
            buff[n - 1] = 0; //remove end line
        }
        if (false == parser.addCmd(buff))
        {
            String err = "parce err ";
            err += "line=";
            err += parser.getLineTotal();
            err += ",data=";
            err += buff;
            m_server.send(400, "text/plain", err);
            return;
        }
    }
    gcode.close();
    gcode_leveled.close();
    m_sdCnt.setESPTimeout();
    webRetResult(m_server, er_ok);

}

void CWebServer::getStatus(JsonObject &root) const
{
    auto mode = root.createNestedObject("mode");
    mode["ProbeArea"] = static_cast<unsigned>(m_ProbeArea.getMode());
}
;
