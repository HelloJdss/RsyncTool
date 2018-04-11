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
#include "BlockInfos_generated.h"

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

        void SendToClient(Protocol::Opcode op, uint32_t taskID, uint8_t * buff, uint32_t size);


    private:
        friend class NetMod;

        friend class Inspector;

        friend class MsgThread;

        void onRecvReverseSyncReq(uint32_t taskID, BytesPtr data); //接收到反向同步请求

        //每当Inspector生成一块后执行
        void onInspectBlockInfo(uint32_t taskID, const ST_BlockInformation &blockInfo, const string &filename, size_t filesize);

        MsgHelper m_msgHelper;

        TCPSocketPtr m_socket;

        RTMap<uint32_t, RTMap<string, const Protocol::FileBlockInfos *> > m_tasks; // task ==> filename ==> BlockInfos ptr

        void onRecvViewDirReq(uint32_t taskID, BytesPtr data);  //接收到浏览文件请求

        void onRecvSyncFile(uint32_t taskID, BytesPtr data);    //接收到文件正向同步请求

        void onRecvRebuildInfo(uint32_t taskID, BytesPtr data); //接收到重建文件信息

        void onRecvRebuildChunk(uint32_t taskID, BytesPtr data); //接收到重建块信息

        RTMap<uint32_t, ST_TaskInfo> m_tasks1;

        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
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
