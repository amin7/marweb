/*
 * CManageSDControl.h
 *
 *  Created on: Feb 10, 2020
 *      Author: family
 */

#ifndef LIBS_CMANAGESDCONTROL_H_
#define LIBS_CMANAGESDCONTROL_H_
typedef enum {
    sdStateAuto = 0,
    sdStateEsp,
    sdStateMarlin
} tStateSDcontrol;

constexpr auto SPI_BLOCKOUT_PERIOD = 10000UL;
constexpr auto SPI_START_BLOCKOUT_PERIOD = 5000UL;
constexpr auto CHANGE_NOTICE_TIMEOUT = 30000L;

class CManageSDControl {
private:
  bool m_isESPuse = false;
  bool isESPuse() const
  {
    return m_isESPuse;
  }
  void setESPuse(bool use)
      {
    m_isESPuse = use;
  }
    virtual bool takeBus()=0;
    virtual void returnBus()=0;
  
protected:
  volatile long m_MarlinReleaseTimeout = 0;
  long m_ESPReleaseTimeout = 0;
  tStateSDcontrol stateSDcontrol = sdStateAuto;

public:
    bool setStateSDcontrol(tStateSDcontrol mode);
    tStateSDcontrol getStateSDcontrol() const
    {
        return stateSDcontrol;
    }
    bool requestSDcontrol();
    void cs_marlin_isr();
    bool isMarlinUseSPI() const;
    void setup();
    void loop();
    void setESPTimeout();
};

#endif /* LIBS_CMANAGESDCONTROL_H_ */
