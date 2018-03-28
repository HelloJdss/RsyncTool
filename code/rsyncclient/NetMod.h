//
// Created by carrot on 18-3-26.
//

#ifndef RSYNCTOOL_NETMOD_H
#define RSYNCTOOL_NETMOD_H

#include <cstdint>
#include <unordered_map>
#include "cm_define.h"
#include "NetHelper.h"
#include "MsgHelper.h"
#include "Generator.h"
#include "FileHelper.h"
#include "LogHelper.h"
#include "Protocol_define.h"

namespace RsyncClient
{

    class FileBaseData
    {
    public:
        //uint32_t        m_taskID;
        string          m_filePath;   //记录本地文件名及其路径
        //GeneratorPtr    m_generator;  //记录本地已有的文件信息
        uint64_t        m_newSize;    //同步后的文件总长度
        uint64_t        m_processLen; //已经处理的文件长度
        FilePtr         m_pnewfile;   //重建的文件指针

        FileBaseData(/*uint32_t taskID, GeneratorPtr generator*/)
        {
            //m_generator = generator;
            //m_taskID = taskID;
            m_newSize = 0;
            m_processLen = 0;
        }
        ~FileBaseData()
        {
            if(m_processLen != m_newSize)
            {
                LOG_WARN("Task[%lu] File: [%s] Synchronize Abort!");    //未完成的任务，给予警告
            }
            LOG_TRACE("~FileBaseData");
        }
    };

    typedef std::shared_ptr<FileBaseData> FileBaseDataPtr;

    class NetMod
    {
    DECLARE_SINGLETON_EX(NetMod)

    public:
        ~NetMod();

        bool Init();

        void Run();

        void CreateNewTask(Protocol::Opcode op, const string& src, const string& des);  //创建新的同步任务

        void Dispatch();

    private:

        void createReverseSyncTask(uint32_t taskID, const string& src, const string& des);  //创建反向同步任务

        void onRecvErrorCode(uint32_t taskID, BytesPtr data);

        void onRecvReverseSyncAck(uint32_t taskID, BytesPtr data);

        volatile bool m_running = false;

        MsgHelper m_msgHelper;
        TCPSocketPtr m_serversocket;

        RTMap<uint32_t, RTVector<FileBaseDataPtr>> m_task2data;            // task ID   ===> [basedata]
        RTMap<uint32_t, GeneratorPtr> m_task2generator;          // task ID   ===> generator
    };

#define g_NetMod NetMod::Instance()
}


#endif //RSYNCTOOL_NETMOD_H
