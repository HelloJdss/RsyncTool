//
// Created by carrot on 18-3-15.
//

#pragma once
#ifndef RSYNCTOOL_NETMOD_H
#define RSYNCTOOL_NETMOD_H

#include "LogHelper.h"
#include "NetHelper.h"

class NetMod {
    DECLARE_SINGLETON_EX(NetMod)

public:
    void Init(uint16_t port = 52077);

    void Run();

    void Stop();

private:
    void removeUnavailableSockets();

    TCPSocketPtr m_listenSocket = nullptr;
    SocketAddressPtr m_receivingAddr = nullptr;
    vector<TCPSocketPtr> m_readBlockSockets;

    bool m_inited = false;
    volatile bool m_running = false;
};

#define g_NetMod NetMod::Instance()

#endif //RSYNCTOOL_NETMOD_H
