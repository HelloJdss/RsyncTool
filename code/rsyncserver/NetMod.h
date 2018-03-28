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

namespace RsyncServer
{
    class TCPClient
    {
    public:
        TCPClient(TCPSocketPtr socket) : m_socket(socket)
        {}

        ~TCPClient()
        { LOG_TRACE("~TCPClient"); }

        void Dispatch();

        void SendToClient(Protocol::Opcode op, uint32_t taskID, BytesPtr data);

    private:
        friend class NetMod;

        friend class Inspector;

        friend class MsgThread;

        void onRecvReverseSyncReq(uint32_t taskID, BytesPtr data); //接收到反向同步请求

        void onInspectBlockInfo(uint32_t taskID, const ST_BlockInfo &blockInfo); //每当Inspector生成一块后执行
        MsgHelper m_msgHelper;

        TCPSocketPtr m_socket;

    };

    typedef std::shared_ptr<TCPClient> TCPClientPtr;

    class MsgThread : public Thread
    {
    public:
        void SetArgs(const TCPClientPtr tcpClientPtr);

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

        void Init(uint16_t port = 52077);

        void Run();

        void Stop();

        //void RunThread(void *);
        //void onThreadCreated(void* args);
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
