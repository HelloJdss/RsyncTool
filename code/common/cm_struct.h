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

struct ST_BlockInformation //块信息
{
    uint64_t         offset;
    uint32_t         length;
    uint32_t        checksum;
    std::string     md5;

    std::string     data;

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

    void setOpCode(Protocol::Opcode op) { opcode = static_cast<Protocol::Opcode>(htonl(op)); }

    Protocol::Opcode getOpCode() { return static_cast<Protocol::Opcode>(ntohl(opcode)); }

    void setTaskId(uint32_t id) { taskID = htonl(id); }

    uint32_t getTaskId() { return ntohl(taskID); }

    void reset() { bzero(this, sizeof(*this)); }

private:
    Protocol::Opcode opcode;
    uint32_t taskID;
};

enum TaskType
{
    NONE                =   0,
    Push                =   1,  //客户端同步至服务器
    Pull_File           =   2,  //服务器同步文件至客户端
    Pull_Dir            =   3,  //服务器同步目录至客户端，此状态下需要先创建ViewDir任务来获取服务器目录下的文件列表，然后添加每个文件对应的Pull_File任务
    ViewDir             =   4,  //查看目录

    Error               =  97,  //有错误
    Abort               =  98,  //终止
    Finished            =  99,  //已完成的任务
};

struct ST_TaskInfo  //任务信息
{
    uint32_t         m_taskID;
    uint32_t         m_launch_count;    //执行次数，超过一定次数(3次)放弃执行，直接转入Abort状态
    TaskType         m_type;
    std::string      m_src;    //客户端文件或目录路径
    std::string      m_des;    //服务器文件或目录路径
    std::string      m_err;    //错误提示

    int64_t          m_rebuild_size;   //文件重建总大小
    GeneratorPtr     m_generatorPtr;
    uint64_t         m_processLen; //已经处理的文件长度
    FilePtr          m_pf;   //重建的文件指针

    ST_TaskInfo()
    {
        m_taskID = 0;
        m_rebuild_size = 0;
        m_type = NONE;
        m_src.clear();
        m_des.clear();
        m_err.clear();
        m_launch_count = 0;
        m_generatorPtr = nullptr;
        m_rebuild_size = -1;
        m_processLen = 0;
        m_pf = nullptr;
    }
};

#endif //RSYNCTOOL_CM_STRUCT_H
