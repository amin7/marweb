/*
 * CFileList.cpp
 *
 *  Created on: 13 груд. 2019 р.
 *      Author: ominenko
 */

#include "CWebFileListSD.h"

#include "Logs.h"
#include <FS.h>
#include <set>
#include "misk.h"
using namespace std;

constexpr auto FILE_LIST_SPLIT_SIZE = 1024; //if more - send date
CWebFileListSD::CWebFileListSD(ESP8266WebServer &_server, sdfat::SdFat &_sdFat, CManageSDControl &sdCnt) :
        server(_server), sdFat(_sdFat), m_sdCnt(sdCnt)
{
}

String CWebFileListSD::normalizePath(const String &path){
    auto parpath = path;
    parpath.trim();
    parpath.replace("//", "/");
    return parpath;
}

void CWebFileListSD::getFileList()
{
    if (!m_sdCnt.isOwned())
    {
        webRetResult(server, err_MarlinRead);
        return;
    }
    DBG_PRINT(F("URI "));
    DBG_PRINTLN(server.uri());
    //get current path
    if (server.hasArg("path"))
    {

        const auto path = normalizePath(server.arg("path"));

        DBG_PRINT(F("path "));
        DBG_PRINTLN(path);

        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        server.sendHeader("Content-Type", "application/json", true);
        server.sendHeader("Cache-Control", "no-cache");
        std::string jsonfile = "{\"path\":\"";
        jsonfile += path.c_str();
        jsonfile += "\"";
        server.sendContent(jsonfile.c_str());

        DBG_PRINTLN("getFileList")
        do
        {
            auto root = sdFat.open(path);
            if (!root.isDir())
            {
                DBG_PRINTLN("no dir");
                break;
            }
            sdfat::SdFile file;
            std::string fileList = ",\"files\":[";
            boolean first = true;
            while (file.openNext(&root))
            {
                if (FILE_LIST_SPLIT_SIZE < fileList.length())
                        {
                    server.sendContent(fileList.c_str());
                    fileList.clear();
                }
                char filename[128];
                if (file.getName(filename, sizeof(filename)))
                    if (isExtMach(filename, "gcode") || file.isDir())
                    {
                        if (!first)
                        {
                            fileList += ',';
                        } else
                        {
                            first = false;
                        }

                        fileList += "[\"";
                        fileList += filename;
                        fileList += "\",";
                        fileList += file.isDir() ? '1' : '0';
                        fileList += ',';
                        fileList += to_string(file.fileSize());
                        fileList += "]";
                    }
                file.close();
            }
            fileList += "]";
            server.sendContent(fileList.c_str());
        } while (0);
        server.sendContent("}");
        server.sendContent("");
        return;
    }
    if (server.hasArg("delete"))
            {
        const auto path = normalizePath(server.arg("delete"));
        DBG_PRINT(F("delete "));
        DBG_PRINTLN(path);

        // delete a file
        auto retVal = sdFat.remove(path.c_str());
        // delete a directory
//        retVal = sd.rmdir(uri.c_str());
        if (!retVal) {
            // send error
            server.send(500, "text/plain", "Unable to delete");
            DBG_PRINTLN(";Unable to delete file/directory");
            return;
        }

        server.send(200, "text/plain", "ok");
        return;
    }
    server.send(400, "text/plain", "icorrect mode");
}

void CWebFileListSD::handleFileDownload()
{
    DBG_FUNK_LINE();
    if (!server.hasArg("file"))
    {
        webRetResult(server, er_no_parameters);
        return;
    }
    if (!m_sdCnt.isOwned())
    {
        webRetResult(server, err_MarlinRead);
        return;
    }
    const auto fileName = server.arg("file");
    DBG_PRINTLN(fileName);

    sdfat::SdFile rFile;
    rFile.open(fileName.c_str(), sdfat::O_READ);
    // check for open error
    if (!rFile.isOpen())
    {
        webRetResult(server, er_fileNotFound);
        return;
    }
    const auto fileSize = rFile.fileSize();
    DBG_PRINT(":");
    DBG_PRINTLN(fileSize);
    server.setContentLength(fileSize);
    server.send(200, "application/octet-stream", "");
    size_t send = 0;
    while (rFile.available())
    {
        uint8_t buf[1460];
        // SD read speed ~ 17sec for 4.5MB file
        const auto numRead = rFile.read(buf, sizeof(buf));
        if (-1 == numRead)
        {
            server.client().stopAll();
            webRetResult(server, er_FileIO);
            rFile.close();
            return;
        }
        server.client().write(buf, numRead);
        send += numRead;
    }
    rFile.close();
    if (fileSize != send)
    {
        DBG_PRINT("=");
        DBG_PRINTLN(send);
    }
}

void CWebFileListSD::handleFileUpload()
{
    if (!m_sdCnt.isOwned())
    {
        webRetResult(server, err_MarlinRead);
        return;
    }
    te_ret rError = er_ok;
    DBG_FUNK_LINE();
    HTTPUpload &upload = server.upload();
    switch (upload.status) {
        case UPLOAD_FILE_START:
            {
            String filename = upload.filename;
            if (!filename.startsWith("/"))
                filename = "/" + filename;
            DBG_PRINT("handleFileUpload Name: ");
            DBG_PRINTLN(filename);
            if (!m_uploadSdFile.open(filename.c_str(), sdfat::O_CREAT | sdfat::O_WRITE))
            {
                rError = er_createFile;
            }
        }
            break;
        case UPLOAD_FILE_WRITE:
            if (!m_uploadSdFile.isOpen())
            {
                rError = er_openFile;
                break;
            }
            // Write the received bytes to the file
            if (upload.currentSize != m_uploadSdFile.write(upload.buf, upload.currentSize))
            {
                rError = er_FileIO;
                break;
            }
            break;
        case UPLOAD_FILE_END:
            // If the file was successfully created
            m_uploadSdFile.close();                               // Close the file again
            DBG_PRINT("handleFileUpload Size: ")
            ;
            DBG_PRINTLN(upload.totalSize)
            ;
            server.send(200);
            break;
        default:
            rError = er_last;
            break;
        }
    if (rError != 0) {
        if (server.client().available() > 0) {
            upload.status = UPLOAD_FILE_ABORTED;
            server.client().stopAll();
            webRetResult(server, rError);
        }
        m_uploadSdFile.close();
    }
}

