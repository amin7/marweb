/*
 * CManageSDControl.h
 *
 *  Created on: Feb 10, 2020
 *      Author: family
 */

#pragma once

class CManageSDControl {
private:
    bool m_isOwned = false;
    virtual bool takeSD_impl()=0;
    virtual void returnSD_impl()=0;

public:
    bool takeSD() {
        if (takeSD_impl()) {
            m_isOwned = true;
            return true;
        }
        returnSD();
        return false;
    }
    void returnSD() {
        m_isOwned = false;
        returnSD_impl();
    }
    bool isOwned() const {
        return m_isOwned;
    }
    void setup() {
        returnSD();
    }
};

