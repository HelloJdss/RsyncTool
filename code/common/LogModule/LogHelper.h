//
// Created by carrot on 18-3-12.
//

#pragma once
#ifndef RSYNCTOOL_LOGHELPER_H
#define RSYNCTOOL_LOGHELPER_H

#include <cstring>
#include <string>
#include <sys/time.h>
#include <pthread.h>

#include "cm_define.h"

extern pid_t gettid(void);

using std::string;

enum LOG_LEVEL
{
    MIN = 1,
    FATAL = MIN,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE,
    MAX = TRACE,
};

#define LOG_LEN_LIMIT 1024
#define LOG_LEVEL_DEFALT LOG_LEVEL::MAX
#define LOG_FILE_PATH "./log"


class utc_timer
{
public:
    utc_timer();

    uint64_t get_curr_sec(int *p_msec = nullptr); //return sec, arg return 0.msec

    uint64_t get_curr_msec();   //获取现在的总毫秒数

    int year, mon, day, hour, min, sec;
    char utc_fmt[20];

private:
    void reset_utc_fmt()
    {
        snprintf(utc_fmt, 20, "%d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
    }

    void reset_utc_fmt_sec()
    {
        snprintf(utc_fmt + 17, 3, "%02d", sec);
    }

    uint64_t _sys_acc_min;
    uint64_t _sys_acc_sec;

};

class LogHelper
{
DECLARE_SINGLETON_EX(LogHelper)

public:
    ~LogHelper();

    void Init(LOG_LEVEL Lv, const string &AppName, const string &Path);

    void SetDebugModel()
    { this->m_isdebug = true; }

    LOG_LEVEL GetLV()
    { return this->m_lv; }

    void TryAppend(LOG_LEVEL lv, const char *lvl, const char *format, ...); //write to file, need inited first

private:
    FILE *m_fp = nullptr;
    bool m_inited = false;
    bool m_isdebug = false;
    string m_name;
    string m_path;
    LOG_LEVEL m_lv = LOG_LEVEL::MAX;
    utc_timer m_tm;
    static pthread_mutex_t m_mutex;
};

#define g_LogHelper LogHelper::Instance()

//format: [LEVEL][yy-mm-dd h:m:s.ms][m_tid]file_name:line_no(func_name):content
#define LOG_TRACE(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= TRACE) \
        { \
            g_LogHelper->TryAppend(TRACE, "[TRACE]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_DEBUG(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= DEBUG) \
        { \
            g_LogHelper->TryAppend(DEBUG, "[DEBUG]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_INFO(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= INFO) \
        { \
            g_LogHelper->TryAppend(INFO, "[INFO!]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_NORMAL(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= INFO) \
        { \
            g_LogHelper->TryAppend(INFO, "[INFO!]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_WARN(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= WARN) \
        { \
            g_LogHelper->TryAppend(WARN, "[WARN!]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_ERROR(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= ERROR) \
        { \
            g_LogHelper->TryAppend(ERROR, "[ERROR]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_FATAL(fmt, args...) \
    do \
    { \
        if (g_LogHelper->GetLV() >= FATAL) \
        { \
            g_LogHelper->TryAppend(FATAL, "[FATAL]", "[%5u]%s:%d(%s): " fmt "\n", \
                    gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LogCheckConditionVoid(condition, fmt, args...) \
    do { \
        if(!(condition)) { \
            LOG_ERROR(fmt, ##args); \
            return; \
        } \
    } while(0)

#define LogCheckCondition(condition, ret, fmt, args...) \
    do { \
        if(!(condition)) { \
            LOG_ERROR(fmt, ##args); \
            return ret; \
        } \
    } while(0)

#define LOG_LastError() \
    do \
    { \
        LOG_ERROR("%s", strerror(errno)); \
    } while(0)


#endif //RSYNCTOOL_LOGHELPER_H
