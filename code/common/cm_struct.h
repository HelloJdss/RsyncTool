//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_CM_STRUCT_H
#define RSYNCTOOL_CM_STRUCT_H

#include <strings.h>
#include <cstdint>
#include <netinet/in.h>

#include "Protocol_define.h"
#include "Generator.h"
#include "FileHelper.h"
#include "LogHelper.h"

struct ST_BlockInformation //块信息
{
    uint64_t offset;
    uint32_t length;
    uint32_t checksum;
    std::string md5;

    std::string data;

    //std::string     path; //文件名
    //int64_t         size; //总大小

    ST_BlockInformation()
    {
        offset = 0;
        length = 0;
        checksum = 0;
        md5.clear();
        data.clear();
    }
};

struct ST_PackageHeader //包头
{
    ST_PackageHeader()
    {
        reset();
    }

    ST_PackageHeader(Protocol::Opcode op, uint32_t id)
    {
        opcode = static_cast<Protocol::Opcode>(htonl(op));
        taskID = htonl(id);
    }

    void setOpCode(Protocol::Opcode op)
    { opcode = static_cast<Protocol::Opcode>(htonl(op)); }

    Protocol::Opcode getOpCode()
    { return static_cast<Protocol::Opcode>(ntohl(opcode)); }

    void setTaskId(uint32_t id)
    { taskID = htonl(id); }

    uint32_t getTaskId()
    { return ntohl(taskID); }

    void reset()
    { bzero(this, sizeof(*this)); }

private:
    Protocol::Opcode opcode;
    uint32_t taskID;
};

enum TaskType
{
    NONE = 0,
    Push = 1,  //客户端同步至服务器
    Pull_File = 2,  //服务器同步文件至客户端
    Pull_Dir = 3,  //服务器同步目录至客户端，此状态下需要先创建ViewDir任务来获取服务器目录下的文件列表，然后添加每个文件对应的Pull_File任务
    ViewDir = 4,  //查看目录
};

enum TaskState
{
    UnInit = 0,  //未初始化
    Ready = 1,  //就绪
    Run = 2,  //已启动

    Warn = 97,  //有警告
    Abort = 98,  //终止
    Finished = 99,  //已完成的任务
};

class TaskInfo  //任务信息
{

public:
    TaskInfo(uint32_t taskID, TaskType type);

    TaskInfo();

    ~TaskInfo();

    void Ready();  //设定任务就绪

    void Launch(); //启动任务

    void Abort(string err);  //终止任务

    void Warn(string err);  //任务有警告

    void Finish();  //完成任务

    int64_t GetLaunchTime() const  //获取启动时间
    { return m_launch_time; }

public:
    //以下成员维护任务本身的状态数据
    uint32_t m_taskID;
    TaskType m_type;
    TaskState m_stat;
    uint64_t m_launch_time;    //执行时间，超过一定时间将其终止
    std::string m_err;    //错误提示

    //以下变量用于存储数据
    std::string m_src;    //客户端文件或目录路径
    std::string m_des;    //服务器文件或目录路径

    int64_t m_rebuild_size;   //文件重建总大小
    GeneratorPtr m_generatorPtr;
    uint64_t m_processLen; //已经处理的文件长度
    FilePtr m_pf;   //重建的文件指针
};

inline TaskInfo::TaskInfo(uint32_t taskID, TaskType type)
{
    m_taskID = taskID;
    m_type = type;
    m_stat = TaskState::UnInit;
    m_launch_time = utc_timer().get_curr_msec();

    m_src.clear();
    m_des.clear();
    m_err.clear();
    m_generatorPtr = nullptr;
    m_rebuild_size = -1;
    m_processLen = 0;
    m_pf = nullptr;
}

inline void TaskInfo::Launch()
{
    m_stat = TaskState::Run;
    m_launch_time = utc_timer().get_curr_msec();
    LOG_INFO("Launch Task: [%lu] Type: [%d] Src: [%s] Des: [%s]", m_taskID, m_type,
             m_src.c_str(), m_des.c_str());
}

inline void TaskInfo::Abort(string err)
{
    m_stat = TaskState::Abort;
    m_err = err;
    m_generatorPtr = nullptr; //释放资源
    m_pf = nullptr;
    LOG_ERROR("Abort Task: [%lu] Type: [%d] Src: [%s] Des: [%s] Err: [%s] Use: [%lld] ms", m_taskID, m_type,
             m_src.c_str(), m_des.c_str(), m_err.c_str(), utc_timer().get_curr_msec() - m_launch_time);
}

inline void TaskInfo::Warn(string err)
{
    m_stat = TaskState::Warn;
    m_err = err;
    m_generatorPtr = nullptr; //释放资源
    m_pf = nullptr;
    LOG_WARN("Warn Task: [%lu] Type: [%d] Src: [%s] Des: [%s] Err: [%s] Use: [%lld] ms", m_taskID, m_type,
             m_src.c_str(), m_des.c_str(), m_err.c_str(), utc_timer().get_curr_msec() - m_launch_time);
}

inline void TaskInfo::Finish()
{
    m_stat = TaskState::Finished;
    m_generatorPtr = nullptr; //释放资源
    m_pf = nullptr;
    LOG_INFO("Finish Task: [%lu] Type: [%d] Src: [%s] Des: [%s] Use: [%lld] ms", m_taskID, m_type,
             m_src.c_str(), m_des.c_str(), utc_timer().get_curr_msec() - m_launch_time);
}

inline void TaskInfo::Ready()
{
    m_stat = TaskState::Ready;
    LOG_INFO("Add Task: [%lu] Type: [%d] Src: [%s] Des: [%s]", m_taskID, m_type, m_src.c_str(),
             m_des.c_str());
}

inline TaskInfo::TaskInfo()
{
    m_taskID = 0;
    m_type = TaskType::NONE ;
    m_stat = TaskState::UnInit;
    m_launch_time = utc_timer().get_curr_msec();

    m_src.clear();
    m_des.clear();
    m_err.clear();
    m_generatorPtr = nullptr;
    m_rebuild_size = -1;
    m_processLen = 0;
    m_pf = nullptr;
}

inline TaskInfo::~TaskInfo()
{
    m_generatorPtr = nullptr; //释放资源
    m_pf = nullptr;
}

#endif //RSYNCTOOL_CM_STRUCT_H
