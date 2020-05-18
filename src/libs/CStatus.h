/*
 * CStatus.h
 *
 *  Created on: 7 ????. 2019 ?.
 *      Author: User
 */

#pragma once
#include <stdint.h>
#include <ArduinoJson.h>
#include <iostream>
//https://github.com/bblanchon/ArduinoJson.git
class CStatus {
public:
    //todo
    virtual void getStatus(std::ostream &root) const {

    }
    //remove
    virtual void getStatus(JsonObject &root) const =0;
};

