//
// Created by carrot on 18-3-26.
//

#ifndef RSYNCTOOL_NETMOD_H
#define RSYNCTOOL_NETMOD_H

#include <cstdint>
#include <unordered_map>
#include "Cmd_generated.h"
#include "cm_define.h"
#include "NetHelper.h"
#include "Generator.h"
#include "FileHelper.h"
#include "LogHelper.h"
#include "Protocol_define.h"
#include "ThreadBase.h"
#include "cm_struct.h"
#include "MsgHelper.h"

namespace RsyncClient
{
    class TaskMgr //封装mutex，保持线程访问安全
    {
    public:
        void AddTask(TaskInfo &task);

        int GetTaskCount();
        int GetTaskCount(TaskState state); //获取任务数量

        void Ready(uint32_t taskID);  //设定任务就绪

        void Launch(uint32_t taskID); //启动任务

        void Abort(uint32_t taskID, string err);  //终止任务

        void Warn(uint32_t taskID, string err);  //任务有警告

        void Finish(uint32_t taskID);  //完成任务

        RTVector<uint32_t> GetTasksByType(TaskType type, uint32_t count = -1);  //获取处于状态的count个任务

        RTVector<uint32_t> GetTasksByState(TaskState state, uint32_t count = -1);  //获取处于状态的count个任务

        TaskInfo* GetTask(uint32_t taskID);

        bool TaskEnd(); //检查任务是否全部结束

        int64_t
        Find_First(TaskState state, TaskType type, string const *src, string const *des);//寻找第一个符合条件的taskID

    private:
        RTMap<uint32_t, TaskInfo> m_tasks;                  //taskID ==> taskInfo 任务列表

        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
    };

    class NetMod
    {
    DECLARE_SINGLETON_EX(NetMod)

    public:
        ~NetMod();

        void AddTask(TaskType taskType, string const *src, string const *des, const string &desIP, uint16_t desPort);

        //返回成功启动的任务数

        bool Init();

        void Run();

        void Stop();

        void Dispatch();

        void SendToServer(Protocol::Opcode op, uint32_t taskID, uint8_t *buf, uint32_t size);

        void SendErrToServer(uint32_t taskID, Protocol::Err err, string tip = "");


    private:

        friend class Inspector;

        friend class MsgThread;

        void createViewDirTask(const string &desDir);  //创建文件浏览任务

        void launchViewDirTask(uint32_t taskID);    //启动文件浏览任务

        Protocol::Err onRecvViewDirAck(uint32_t taskID, BytesPtr data);


        void createPushTask(const string &srcPath, const string &desDir);

        void launchPushTask(uint32_t taskID);

        Protocol::Err onRecvFileDigest(uint32_t taskID, BytesPtr data);

        void onInspectCallBack(uint32_t taskID, const ST_BlockInformation &info);


        void createPullTask(const string &srcDir, const string &desPath);

        void launchPullTask(uint32_t taskID);

        Protocol::Err onRecvRebuildInfo(uint32_t taskID, BytesPtr data); //接收到重建文件信息

        Protocol::Err onRecvRebuildChunk(uint32_t taskID, BytesPtr data); //接收到重建块信息

        void onRecvErrorCode(uint32_t taskID, BytesPtr data); //不再回复，必定结束一个任务

        volatile bool m_running = false;

        MsgHelper m_msgHelper;
        TCPSocketPtr m_serverSocket;

        TaskMgr m_taskMgr;

        string m_serverIp;
        uint16_t m_serverPort = 48888;

        uint32_t m_taskIndex = 10000;

        uint32_t m_taskRunningCount = 0;                       //正在执行的任务数目

        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;   //确保多线程读写安全
    };

#define g_NetMod NetMod::Instance()

    class MsgThread : public Thread
    {
    public:

        void Runnable();

    private:

    };

    typedef std::shared_ptr<MsgThread> MsgThreadPtr;
}


#endif //RSYNCTOOL_NETMOD_H
