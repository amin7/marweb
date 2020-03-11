/*
 * CStatus.h
 *
 *  Created on: 7 ????. 2019 ?.
 *      Author: User
 */

#pragma once
#include <stdint.h>
#include <ArduinoJson.h>
//https://github.com/bblanchon/ArduinoJson.git

class CStatus {
public:
    virtual void getStatus(JsonObject &root) const =0;
};

