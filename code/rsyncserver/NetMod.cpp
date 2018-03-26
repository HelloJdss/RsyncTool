//
// Created by carrot on 18-3-15.
//

#include "cm_define.h"
#include "NetMod.h"
#include "FileHelper.h"
#include "BlockInfos_generated.h"
#include "ErrorCode_generated.h"
#include "ReconstructList_generated.h"

using namespace Protocol;
using namespace RsyncServer;

void NetMod::Init(uint16_t port)
{
    m_listenSocket = NetHelper::CreateTCPSocket(INET);
    //m_receivingAddr = SocketAddressPtr(new SocketAddress("127.0.0.1", port));
    m_receivingAddr = SocketAddressPtr(new SocketAddress(INADDR_ANY, port));
    auto err = m_listenSocket->Bind(*m_receivingAddr);
    if (err != NO_ERROR)
    {
        m_inited = false;
        LOG_ERROR("Server Socket Bind Failed: %s", strerror(err));
        return;
    }
    //readableSockets.clear();
    m_readBlockSockets.clear();
    m_readBlockSockets.push_back(m_listenSocket);
    m_inited = true;
    LOG_INFO("Server NetMod Init![%s]", m_listenSocket->GetEndPoint().c_str());
}

void NetMod::Run()
{
    LogCheckConditionVoid(m_inited, "NetMod has not been inited!");
    m_running = true;
    m_listenSocket->Listen();
    //timeval tm;
    //tm.tv_sec = 10;
    //tm.tv_usec = 0;    //set timeout to 10s
    vector<TCPSocketPtr> readableSockets;
    while (m_running)
    {
        removeUnavailableSockets();
        LOG_DEBUG("%d", m_readBlockSockets.size());
        readableSockets.clear();

        if (NetHelper::Select(&m_readBlockSockets, &readableSockets, nullptr, nullptr, nullptr, nullptr, nullptr) > 0)
        {
            for (const TCPSocketPtr &socket : readableSockets)
            {
                if (socket == m_listenSocket)    // new client connection
                {
                    SocketAddress newclientaddr;
                    auto newSocket = m_listenSocket->Accept(newclientaddr);
                    m_readBlockSockets.push_back(newSocket);

                    //TODO: do something when new client connect! ...

                    LOG_INFO("Accept Connection[%s]", newSocket->GetEndPoint().c_str());
                    //newSocket->Send("HelloWorld", 10);
                    m_clientMap[newSocket] = TCPClientPtr(new TCPClient(newSocket));
                }
                else    //receive data from an old client
                {
                    if (m_clientMap.find(socket) != m_clientMap.end())
                    {
                        try
                        {
                            int count = socket->Receive(m_clientMap[socket]->m_msgHelper.GetBuffer() +
                                                        m_clientMap[socket]->m_msgHelper.GetStartIndex(),
                                                        m_clientMap[socket]->m_msgHelper.GetRemainBytes());
                            if (count > 0)
                            {
                                //TODO: do something when recv data from old client
                                m_clientMap[socket]->m_msgHelper.AddCount(count);

                                pthread_t pid;
                                thread_params params;
                                params._this = this;
                                params._args = &m_clientMap[socket];
                                params._needdel = false;
                                params._del_func = nullptr;
                                if (pthread_create(&pid, NULL, t_Thread<NetMod, &NetMod::RunThread>, &params) == 0)
                                {
                                    pthread_detach(pid);
                                }
                                else
                                {
                                    LOG_ERROR("Thread Create Failed: %s", strerror(errno));
                                }
                            }
                            else if (count == 0)
                            {
                                //client disconnect
                                LOG_INFO("RsyncClient[%s] Disconnected!", socket->GetEndPoint().c_str());
                                socket->Close();
                            }
                        }
                        catch (int err)
                        {
                            socket->Close();
                            LOG_LastError();
                            continue;
                        }
                    }
                    else
                    {
                        LOG_WARN("clientMap did not find socket[%s]", socket->GetEndPoint().c_str());
                        socket->Close();
                    }
                }
            }
        }
    }
}

void NetMod::removeUnavailableSockets()
{
    for (long i = m_readBlockSockets.size() - 1; i >= 0; --i)
    {
        if (!m_readBlockSockets.at(i)->IsAvailable())
        {
            m_clientMap.erase(*(m_readBlockSockets.begin() + i));
            m_readBlockSockets.erase(m_readBlockSockets.begin() + i);
        }
    }
}

void NetMod::Stop()
{
    m_running = false;
}

void NetMod::RunThread(void *arg)
{
    TCPClientPtr clientPtr = *(TCPClientPtr *) arg;
    while (clientPtr->m_msgHelper.HasMessage())
    {
        LOG_TRACE("Client[%s] has Message!", clientPtr->m_socket->GetEndPoint().c_str());
        clientPtr->Dispatch();
    }
}

NetMod::~NetMod()
{
    m_running = false;
    m_inited = false;
    m_readBlockSockets.clear();
    m_clientMap.clear();
    m_listenSocket = nullptr;
    m_receivingAddr = nullptr;
    LOG_TRACE("~NetMod");
}

using namespace Protocol;

void TCPClient::Dispatch()
{
    ST_PackageHeader header;
    BytesPtr data;
    m_msgHelper.ReadMessage(header, &data);
    LOG_INFO("Recv Meg from[%s]: op[%s], taskID[%u], dataLength:[%u]", m_socket->GetEndPoint().c_str(),
             Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());

    switch (header.getOpCode())
    {
        case Opcode::REVERSE_SYNC_REQ:
            onRecvReverseSyncReq(header.getTaskId(), data);
            break;
        default:
            LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_socket->GetEndPoint().c_str());
            m_socket->Close();
            break;
    }
}

void TCPClient::onRecvReverseSyncReq(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Recv Reverse Sync Req!");
    /*
     * TODO:
     * 1.拆包
     * 2.检查目标文件(本机)是否存在，若不存在则返回错误码
     * 3.根据taskID建立到数据的映射
     * 4.收到的数据根据checksum建立到md5的映射
     * 5.循环检验本地文件
     * 6.返回重建文件的菜单：可校验一块返回一块，建立流水线作业
     */

    auto blocksInfo = Protocol::GetFileBlockInfos(data->ToChars());
    auto pFilename = blocksInfo->DesPath();
    auto pFile = FileHelper::CreateFilePtr(pFilename->str(), "r");
    auto blocksize = blocksInfo->Splitsize();
    if (pFile == nullptr)   //本地不存在该文件
    {

        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_NO_SUCH_FILE, builder.CreateString(pFilename));
        builder.Finish(fbb);
        this->SendToClient(Opcode::ERROR_CODE, taskID,
                           MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        return;
    }

    //若存在该文件，则读取文件大小，小于块大小的直接传输，大于块大小的重新计算
    if (pFile->Size() < 0)
    {
        LOG_LastError();
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_UNKNOWN, builder.CreateString(strerror(errno)));
        builder.Finish(fbb);
        this->SendToClient(Opcode::ERROR_CODE, taskID,
                           MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        return;
    }
    else if (pFile->Size() < blocksize)
    {
        char buff[blocksize];
        auto count = pFile->ReadBytes(buff, blocksize);

        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateNewBlockAck(builder, builder.CreateString(blocksInfo->SrcPath()),
                                               builder.CreateString(blocksInfo->DesPath()), blocksize, pFile->Size(),
                                               Protocol::CreateNewBlock(builder, builder.CreateString(pFilename), 0, 0,
                                                                        pFile->Size(), builder.CreateString("-"),
                                                                        builder.CreateVector((uint8_t *) buff, count)));
        builder.Finish(fbb);
        this->SendToClient(Opcode::REVERSE_SYNC_ACK, taskID,
                           MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        return;
    }


}

void TCPClient::SendToClient(Opcode op, uint32_t taskID, BytesPtr data)
{

    ST_PackageHeader header(op, taskID);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(data->ToChars(), data->Size()));
    m_socket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));
}

