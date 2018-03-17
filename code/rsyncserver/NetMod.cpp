//
// Created by carrot on 18-3-15.
//

#include "NetMod.h"

void NetMod::Init(uint16_t port) {
    m_listenSocket = NetHelper::CreateTCPSocket(INET);
    //m_receivingAddr = SocketAddressPtr(new SocketAddress("127.0.0.1", port));
    m_receivingAddr =  SocketAddressPtr(new SocketAddress(INADDR_ANY, port));
    if (m_listenSocket->Bind(*m_receivingAddr) != NO_ERROR)
    {
        m_inited = false;
        return;
    }
    //readableSockets.clear();
    m_readBlockSockets.clear();
    m_readBlockSockets.push_back(m_listenSocket);
    m_inited = true;
    LOG_INFO("Server NetMod Init![%s]", m_listenSocket->GetEndPoint().c_str());
}

void NetMod::Run() {
    LogCheckConditionVoid(m_inited, "NetMod has not been inited!");
    m_running = true;
    m_listenSocket->Listen();
    //timeval tm;
    //tm.tv_sec = 0;
    //tm.tv_usec = 0;    //set timeout to 200ms
    while (m_running)
    {
        removeUnavailableSockets();
        LOG_DEBUG("%d", m_readBlockSockets.size());
        vector<TCPSocketPtr> readableSockets;

        if (NetHelper::Select(&m_readBlockSockets, &readableSockets, nullptr, nullptr, nullptr , nullptr , nullptr) > 0)
        {
            for(const TCPSocketPtr& socket : readableSockets)
            {
                if(socket == m_listenSocket)    // new client connection
                {
                    SocketAddress newclientaddr;
                    auto newSocket = m_listenSocket->Accept(newclientaddr);
                    m_readBlockSockets.push_back(newSocket);

                    //TODO: do something when new client connect! ...

                    //test
                    LOG_INFO("Accept Connection[%s]", newSocket->GetEndPoint().c_str());
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
                            LOG_INFO("recv %s", buffer);
                            socket->Send("!", 1);
                        }
                        else if (count == 0)
                        {
                            //client disconnect
                            LOG_INFO("Client[%s] Disconnected!", socket->GetEndPoint().c_str());
                            socket->Close();
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

void NetMod::removeUnavailableSockets(){
    for(long i = m_readBlockSockets.size() - 1; i >= 0; --i)
    {
        if(!m_readBlockSockets.at(i)->IsAvailable())
        {
            m_readBlockSockets.erase(m_readBlockSockets.begin() + i);
        }
    }
}
