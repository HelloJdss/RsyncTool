//
// Created by carrot on 18-3-15.
//

#include "NetMod.h"

void NetMod::Init(uint16_t port) {
    m_listenSocket = NetHelper::CreateTCPSocket(INET);
    m_receivingAddr = SocketAddressPtr(new SocketAddress("127.0.0.1", port));

    if (m_listenSocket->Bind(*m_receivingAddr) != NO_ERROR)
    {
        m_inited = false;
        return;
    }
    m_readableSockets.clear();
    m_readBlockSockets.clear();
    m_readBlockSockets.push_back(m_listenSocket);
    m_inited = true;
    //LOG_INFO("Server NetMod Init![%s]", m_listenSocket->GetEndPoint().c_str());
}

void NetMod::Run() {
    LogCheckConditionVoid(m_inited, "NetMod has not been inited!");
    m_running = true;
    timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = 200000;    //set timeout to 200ms
    while (m_running)
    {
        if (NetHelper::Select(&m_readBlockSockets, &m_readableSockets, nullptr, nullptr, nullptr , nullptr , nullptr))
        {
            for(const TCPSocketPtr& socket : m_readableSockets)
            {
                if(socket == m_listenSocket)    // new client connection
                {
                    SocketAddress newclientaddr;
                    auto newSocket = m_listenSocket->Accept(newclientaddr);
                    m_readBlockSockets.push_back(newSocket);

                    //TODO: do something when new client connect! ...

                    //test
                   // LOG_INFO("Accept Connection[%s]", newSocket->GetEndPoint().c_str());
                    newSocket->Send("HelloWorld", 10);
                }
                else
                {
                    //receive data from an old client
                    char buffer[10];
                    try {
                        int count = socket->Receive(buffer, 10);
                        if (count > 0)
                        {
                            //TODO: do something when recv data from old client
                        }
                        else if (count == 0)
                        {
                            LOG_WARN("recv 0!");
                        }
                    }
                    catch (int err)
                    {
                        continue;
                    }
                }
            }
        }
    }
}
