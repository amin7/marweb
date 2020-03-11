/*
 * sdcard_ex.cpp
 *
 *  Created on: 22 זמגע. 2019 נ.
 *      Author: ominenko
 */

#include "sdcard_ex.h"
#include "fatlib\ArduinoStream.h"
using namespace sdfat;

static uint8_t cidDmp(sdfat::SdFat &sd, ArduinoOutStream &out)
{
    cid_t cid;
    if (!sd.card()->readCID(&cid))
    {
        out << F("readCID failed") << endl;
        return false;
    }
    out << F("\nManufacturer ID: ");
    out << hex << int(cid.mid) << dec << endl;
    out << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
    out << F("Product: ");
    for (uint8_t i = 0; i < 5; i++)
    {
        out << cid.pnm[i];
    }
    out << F("\nVersion: ");
    out << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
    out << F("Serial number: ") << hex << cid.psn << dec << endl;
    out << F("Manufacturing date: ");
    out << int(cid.mdt_month) << '/';
    out << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
    out << endl;
    return true;
}
//------------------------------------------------------------------------------

static void sdcard_type(sdfat::SdFat &sd, sdfat::ArduinoOutStream &out)
{
    const auto cardSize = sd.card()->cardSize();
    out << F("cardSize ");
    if (cardSize == 0)
    {
        out << "cardSize failed" << endl;
        return;
    }
    out << cardSize << endl;
    out << F("Card type: ");
    switch (sd.card()->type()) {
        case SD_CARD_TYPE_SD1:
            out << F("SD1");
            break;
        case SD_CARD_TYPE_SD2:
            out << F("SD2");
            break;
        case SD_CARD_TYPE_SDHC:
            if (cardSize < 70000000)
            {
                out << F("SDHC");
            } else
            {
                out << F("SDXC");
            }
            break;
        default:
            out << F("Unknown");
    }
    out << endl;
}

static void sdcard_filelist(sdfat::SdFat &sd, sdfat::ArduinoOutStream &out, const char *Dir = "/")
{
    SdFile root;
    SdFile file;
    out << F("folder list ") << Dir << endl;
    if (!root.open(Dir))
    {
        out << F("open root failed") << endl;
        return;
    }

    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while (file.openNext(&root, O_RDONLY))
    {
        char filename[128];
        if (file.getName(filename, sizeof(filename)))
        {
            out << filename;
            if (file.isDir())
            {
                out << '/';
            }
            out << endl;
        }
        file.close();
    }
    out << "-------" << endl;
}

void sdcard_info(sdfat::SdFat &sd, Stream &out_stream)
{
    ArduinoOutStream out(out_stream);
    out << "sdcard_info" << endl;
    out << "SdFat version: " << SD_FAT_VERSION << endl;
    const auto cardErrorCode = sd.cardErrorCode();
    if (cardErrorCode)
    {
        sd.initErrorPrint(&out_stream);
        return;
    }    
    sdcard_type(sd, out);
    uint32_t ocr;
    if (!sd.card()->readOCR(&ocr))
    {
        out << "\nreadOCR failed" << endl;
        return;
    }
    out << "OCR: " << hex << ocr << dec << endl;
    sdcard_filelist(sd, out);
}


