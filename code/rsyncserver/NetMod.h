//
// Created by carrot on 18-3-15.
//

#pragma once
#ifndef RSYNCTOOL_NETMOD_H
#define RSYNCTOOL_NETMOD_H

#include <unordered_map>
#include "LogHelper.h"
#include "NetHelper.h"
#include "MsgHelper.h"
#include "Inspector.h"
#include "ThreadBase.h"
#include "Cmd_generated.h"

namespace RsyncServer
{
    class TCPClient
    {
    public:
        TCPClient(TCPSocketPtr socket) : m_socket(socket)
        {
            m_socket->StopStatistics(); //服务端不用统计数据
        }

        ~TCPClient()
        { LOG_TRACE("~TCPClient"); }

        void Dispatch();

        void SendToClient(Protocol::Opcode op, uint32_t taskID, BytesPtr data);

        void SendToClient(Protocol::Opcode op, uint32_t taskID, uint8_t * buff, uint32_t size);


        void SendErrToClient(uint32_t taskID, Protocol::Err err, string tip = "");

        bool IsAvailable();

    private:
        friend class NetMod;

        friend class Inspector;

        friend class MsgThread;

        Protocol::Err onRecvViewDirReq(uint32_t taskID, BytesPtr data);  //接收到浏览文件请求

        Protocol::Err onRecvSyncFile(uint32_t taskID, BytesPtr data);    //接收到文件正向同步请求

        Protocol::Err onRecvRebuildInfo(uint32_t taskID, BytesPtr data); //接收到重建文件信息

        Protocol::Err onRecvRebuildChunk(uint32_t taskID, BytesPtr data); //接收到重建块信息，失败会自动发送Err

        Protocol::Err onRecvFileDigest(uint32_t taskID, BytesPtr data);  //接收到反向同步请求

        void onInspectCallBack(uint32_t taskID, const ST_BlockInformation& info);

        void onRecvErrorCode(uint32_t taskID, BytesPtr data);

        MsgHelper m_msgHelper;

        TCPSocketPtr m_socket;

        RTMap<uint32_t, TaskInfo> m_tasks;

        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

        //std::shared_ptr<SocketThread> m_thread;

        //volatile bool m_available = true;
    };

    typedef std::shared_ptr<TCPClient> TCPClientPtr;

    class MsgThread : public Thread
    {
    public:
        void SetArgs(const TCPClientPtr& tcpClientPtr);

        void Runnable();

    private:
        TCPClientPtr m_ptr;
    };

    typedef std::shared_ptr<MsgThread> MsgThreadPtr;

    class NetMod
    {
    DECLARE_SINGLETON_EX(NetMod)

    public:
        ~NetMod();

        void Init(uint16_t port = 48888);

        void Run();

        void Stop();

    private:
        void removeUnavailableSockets();


        TCPSocketPtr m_listenSocket = nullptr;
        SocketAddressPtr m_receivingAddr = nullptr;

        vector<TCPSocketPtr> m_readBlockSockets;

        unordered_map<TCPSocketPtr, TCPClientPtr> m_clientMap;

        vector<MsgThreadPtr> m_threads;

        bool m_inited = false;
        volatile bool m_running = false;
    };

#define g_NetMod NetMod::Instance()
}

#endif //RSYNCTOOL_NETMOD_H
