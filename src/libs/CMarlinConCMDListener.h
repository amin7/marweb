/*
 * CMarlinConCMDListener.h
 *
 *  Created on: 23 ñ³÷. 2020 ð.
 *      Author: ominenko
 */

#ifndef LIBS_CMARLINCONCMDLISTENER_H_
#define LIBS_CMARLINCONCMDLISTENER_H_
#include <string>
#include <stdint.h>
#include <queue>
#include <functional>
#include "CMarlinCon.h"

typedef struct {
    std::string cmd;
    std::function<bool(std::string &result)> responce;
} tMarlinCMD;

class CMarlinConCMD_Listener: public CMarlinCon_Listener {
    std::queue<tMarlinCMD> m_cmdList;
    uint32_t m_waitOkCount;
public:
    CMarlinConCMD_Listener();
    std::queue<tMarlinCMD>::size_type getCmdQueueSize() const
    {
        return m_cmdList.size();
    }
    virtual void pushLine(const std::string &line) override;
    virtual std::string getCmd() override;
    bool addCmd(const std::string &cmd);
    bool addCmd(const std::string &cmd, std::function<bool(std::string &result)> responce);
    void clear();
    friend class CProbeAreaTest;
};

class CMarlinRun: public CMarlinConCMD_Listener
{
public:
    typedef enum
    {
        paIdle = 0,
        paRun,
        paDone,
        paError
    } te_mode;
    private:
    te_mode m_mode;
    public:
    CMarlinRun() :
            m_mode(paIdle)
    {
    }
    ;
    te_mode getMode() const
    {
        return m_mode;
    }
    void setMode(te_mode mode)
    {
        m_mode = mode;
    }
    void clear()
    {
        m_mode = paIdle;
        CMarlinConCMD_Listener::clear();
    }
};

#endif /* LIBS_CMARLINCONCMDLISTENER_H_ */
