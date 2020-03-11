/*
 * CManageSDControl.cpp
 *
 *  Created on: Feb 10, 2020
 *      Author: family
 */

#include "CManageSDControl.h"
#include "logs.h"

void CManageSDControl::setESPTimeout()
{
    m_ESPReleaseTimeout = millis() + CHANGE_NOTICE_TIMEOUT;
}
bool CManageSDControl::setStateSDcontrol(tStateSDcontrol mode)
{
  switch (mode) {
    case sdStateMarlin:
            returnBus();
            setESPuse(false);
            break;
    case sdStateEsp:
            if (isESPuse())
            {
                break;
            }
            if (false == takeBus())
            {
                returnBus();
                setESPuse(false);
                return false; //no state changed
            }
            setESPuse(true);
      break;
    case sdStateAuto: // will be handled by timeout
            switch (getStateSDcontrol())
            {
                case sdStateEsp:
                    setESPTimeout();
                    setESPuse(true);
                break;
                case sdStateMarlin:
                    m_MarlinReleaseTimeout = millis() + CHANGE_NOTICE_TIMEOUT;
                    setESPuse(false);
                    break;
                default:
                    break;
            }
      break;
    default:
      ERR_LOG("out of range getModeSDcontrol")
      ;
      }
  stateSDcontrol = mode;
  return true;
}

bool CManageSDControl::requestSDcontrol() {
  switch (getStateSDcontrol()) {
    case sdStateMarlin:
      DBG_PRINTLN("sdStateInstert; change it in ui or reboot")
      ;
      return false;
    case sdStateAuto:
            if (isMarlinUseSPI())
            {
                DBG_PRINTLN("Marlin is reading from SD card");
                return false;
            }
            if (!isESPuse())
            {
                if (false == takeBus())
                {
                    DBG_PRINTLN("SD card init error");
                    break;
                }
                setESPuse(true);
            }
            setESPTimeout();
            return true;
    case sdStateEsp:
            return true;
    default:
      ERR_LOG("out of range getModeSDcontrol")
      ;
      break;
  }

  return false;
}
void CManageSDControl::cs_marlin_isr()
  {
  if (!isESPuse())
          m_MarlinReleaseTimeout = millis() + SPI_BLOCKOUT_PERIOD;
  }

bool CManageSDControl::isMarlinUseSPI() const
{
  if (isESPuse()) {
    return false;
  }
    switch (getStateSDcontrol()) {
        case sdStateMarlin:
            return true;
        case sdStateEsp:
            return false;
        default:
            break;
  }
  return millis() < m_MarlinReleaseTimeout;
}

void CManageSDControl::setup()
{
    // ----- SD Card and Server -------
    returnBus();  // startup
    setESPuse(false);
    // Check to see if other master is using the SPI bus
    DBG_PRINT("SD bus waiting (~");
    DBG_PRINTLN(SPI_START_BLOCKOUT_PERIOD);

    m_MarlinReleaseTimeout = millis() + SPI_START_BLOCKOUT_PERIOD;
}

void CManageSDControl::loop()
{
  const auto modeSD = getStateSDcontrol();
  if (isESPuse() && (sdStateAuto == modeSD))
      {
    if (millis() > m_ESPReleaseTimeout)
        { //wait for finish
            DBG_FUNK_LINE();
            returnBus();
            setESPuse(false);
    }
  }
}
