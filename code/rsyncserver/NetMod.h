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

    private:

        friend class NetMod;

        MsgHelper m_msgHelper;

        TCPSocketPtr m_socket;

    };

    typedef std::shared_ptr<TCPClient> TCPClientPtr;

    class NetMod
    {
    DECLARE_SINGLETON_EX(NetMod)

    public:
        void Init(uint16_t port = 52077);

        void Run();

        void Stop();

        void RunThread(void *);

    private:
        void removeUnavailableSockets();

        TCPSocketPtr m_listenSocket = nullptr;
        SocketAddressPtr m_receivingAddr = nullptr;

        vector<TCPSocketPtr> m_readBlockSockets;

        std::unordered_map<TCPSocketPtr, TCPClientPtr> m_clientMap;

        bool m_inited = false;
        volatile bool m_running = false;
    };

#define g_NetMod NetMod::Instance()
}

#endif //RSYNCTOOL_NETMOD_H
