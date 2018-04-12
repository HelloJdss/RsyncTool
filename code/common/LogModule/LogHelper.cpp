//
// Created by carrot on 18-3-13.
//

#include <cstdarg>
#include <unistd.h>//access, getpid, syscall
#include <sys/syscall.h> //SYS_gettid
#include <sys/stat.h> //mkdir
#include <cstring>
#include <cerrno>
#include "LogHelper.h"

pthread_mutex_t LogHelper::m_mutex = PTHREAD_MUTEX_INITIALIZER;

pid_t gettid()
{
    //return syscall(__NR_gettid); //only Linux can use
    return syscall(SYS_gettid);
}

void LogHelper::TryAppend(LOG_LEVEL lv, const char *lvl, const char *format, ...)
{
    RT_ASSERT(m_inited, "LogHelper has not been inited!");

    int ms;
    //char* log_line = new char[LOG_LEN_LIMIT];
    char log_line[LOG_LEN_LIMIT];
    m_tm.get_curr_sec(&ms);
    int prev_len = snprintf(log_line, LOG_LEN_LIMIT, "[%s.%03d]%s", m_tm.utc_fmt, ms, lvl);

    va_list arg_ptr;
    va_start(arg_ptr, format);

    //TODO: OPTIMIZE IN THE FUTURE: performance too low here!
    int main_len = vsnprintf(log_line + prev_len, static_cast<size_t>(LOG_LEN_LIMIT - prev_len), format, arg_ptr);
    va_end(arg_ptr);
    uint32_t len = prev_len + main_len;

    if (this->m_isdebug)
    {
        if(lv < m_lv)
        {
            char *pre;
            switch (lv)
            {
                case FATAL:
                    pre = const_cast<char *>("\33[1;35m");
                    printf("%s%s\33[0m", pre, log_line);
                    break;
                case ERROR:
                    pre = const_cast<char *>("\33[1;31m");
                    printf("%s%s\33[0m", pre, log_line);
                    break;
                case WARN:
                    pre = const_cast<char *>("\33[1;33m");
                    printf("%s%s\33[0m", pre, log_line);
                    break;
                case INFO:
                    printf("%s", log_line);
                    break;
                case DEBUG:
                    pre = const_cast<char *>("\33[1;34m");
                    printf("%s%s\33[0m", pre, log_line);
                    break;
                case TRACE:
                    pre = const_cast<char *>("\33[1;32m");
                    printf("%s%s\33[0m", pre, log_line);
                    break;
            }
        }
    }

    pthread_mutex_lock(&m_mutex);
    if (m_fp != nullptr)
    {
        fwrite(log_line, sizeof(char), len, m_fp);
        fflush(m_fp);
    }
    pthread_mutex_unlock(&m_mutex);
}

void LogHelper::Init(LOG_LEVEL Lv, const string &AppName, const string &Path)
{
    this->m_lv = Lv;
    this->m_name = AppName;
    this->m_path = Path;
    mkdir(Path.c_str(), 0777);
    //查看是否存在此目录、目录下是否允许创建文件
    if (-1 == access(Path.c_str(), F_OK | W_OK))
    {
        fprintf(stderr, "logdir: %s error: %s\n", Path.c_str(), strerror(errno));
    }
    //打开文件

    if (m_fp != nullptr)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_tm.get_curr_sec();
    char log_path[1024] = {};
    sprintf(log_path, "%s/%s_%d_%02d_%02d_%02d_%02d_%02d.log", Path.c_str(), m_name.c_str(), m_tm.year, m_tm.mon,
            m_tm.day, m_tm.hour, m_tm.min, m_tm.sec);
    m_fp = fopen(log_path, "a");
    RT_ASSERT(m_fp != nullptr, "create log file failed!");
    this->m_inited = true;
}

LogHelper::~LogHelper()
{
    LOG_TRACE("~LogHelper");
    if (m_fp != nullptr)
    {
        fclose(m_fp);
    }
}

utc_timer::utc_timer()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    //set _sys_acc_sec, _sys_acc_min
    _sys_acc_sec = static_cast<uint64_t>(tv.tv_sec);
    _sys_acc_min = _sys_acc_sec / 60;
    //use _sys_acc_sec calc year, mon, day, hour, min, sec
    struct tm cur_tm;
    localtime_r((time_t *) &_sys_acc_sec, &cur_tm);
    year = cur_tm.tm_year + 1900;
    mon = cur_tm.tm_mon + 1;
    day = cur_tm.tm_mday;
    hour = cur_tm.tm_hour;
    min = cur_tm.tm_min;
    sec = cur_tm.tm_sec;
    reset_utc_fmt();
}

uint64_t utc_timer::get_curr_sec(int *p_msec)
{
    struct timeval tv;
    //get current ts
    gettimeofday(&tv, nullptr);
    if (p_msec)
    {
        *p_msec = static_cast<int>(tv.tv_usec / 1000);
    }
    //if not in same seconds
    if ((uint32_t) tv.tv_sec != _sys_acc_sec)
    {
        sec = static_cast<int>(tv.tv_sec % 60);
        _sys_acc_sec = static_cast<uint64_t>(tv.tv_sec);
        //or if not in same minutes
        if (_sys_acc_sec / 60 != _sys_acc_min)
        {
            //use _sys_acc_sec update year, mon, day, hour, min, sec
            _sys_acc_min = _sys_acc_sec / 60;
            struct tm cur_tm;
            localtime_r((time_t *) &_sys_acc_sec, &cur_tm);
            year = cur_tm.tm_year + 1900;
            mon = cur_tm.tm_mon + 1;
            day = cur_tm.tm_mday;
            hour = cur_tm.tm_hour;
            min = cur_tm.tm_min;
            //reformat utc format
            reset_utc_fmt();
        }
        else
        {
            //reformat utc format only sec
            reset_utc_fmt_sec();
        }
    }
    return static_cast<uint64_t>(tv.tv_sec);
}

uint64_t utc_timer::get_curr_msec()
{
    int msec = 0;
    uint64_t now = get_curr_sec(&msec);
    return now * 1000 + msec;
}
