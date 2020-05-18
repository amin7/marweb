/*
 * CStatus.h
 *
 *  Created on: 7 ????. 2019 ?.
 *      Author: User
 */

#pragma once
#include <stdint.h>
#include <iostream>
class CStatus {
public:
    virtual void getStatus(std::ostream &root) const =0;
};
