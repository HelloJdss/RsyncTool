//
// Created by carrot on 18-3-15.
//

#include "cm_define.h"
#include "NetMod.h"
#include "ErrorCode_generated.h"
#include "NewBlockInfo_generated.h"

#include "ViewDir_generated.h"


using namespace Protocol;
using namespace RsyncServer;

void NetMod::Init(uint16_t port)
{
    m_listenSocket = NetHelper::CreateTCPSocket(INET);
    m_receivingAddr = SocketAddressPtr(new SocketAddress("127.0.0.1", port));
    //m_receivingAddr = SocketAddressPtr(new SocketAddress(INADDR_ANY, port));
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
                    LOG_INFO("Accept Connection[%s]", newSocket->GetEndPoint().c_str());
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
                                m_clientMap[socket]->m_msgHelper.AddCount(count);

                                //为每个客户端单独创建一个线程
                                //RunThread(this, &m_clientMap[socket], true);
                                auto ptr = MsgThreadPtr(new MsgThread);
                                ptr->SetArgs(m_clientMap[socket]);
                                if (ptr->Start())
                                {
                                    ptr->Detach();
                                    m_threads.push_back(ptr);
                                }
                            }
                            else if (count == 0)
                            {
                                //client disconnect
                                LOG_INFO("RemoteClient[%s] Disconnected!", socket->GetEndPoint().c_str());
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
                        LOG_WARN("ClientMap did not find socket[%s]", socket->GetEndPoint().c_str());
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

    for (long i = m_threads.size() - 1; i >= 0; --i)
    {
        if (m_threads.at(i)->GetState() == Thread::THREAD_STATUS_EXIT)
        {
            m_threads.erase(m_threads.begin() + i); //溢出已经结束的线程
        }
    }
}

void NetMod::Stop()
{
    m_running = false;
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
    while (m_msgHelper.ReadMessage(header, &data))
    {
        LOG_TRACE("Client[%s] has Message!", m_socket->GetEndPoint().c_str());
        LOG_INFO("Recv Meg from[%s]: op[%s], taskID[%u], dataLength:[%u]", m_socket->GetEndPoint().c_str(),
                 Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());

        switch (header.getOpCode())
        {
            case Opcode::REVERSE_SYNC_REQ:
                onRecvReverseSyncReq(header.getTaskId(), data);
                break;

            case Opcode::VIEW_DIR_REQ:
                onRecvViewDirReq(header.getTaskId(), data);
                break;
            default:
                LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_socket->GetEndPoint().c_str());
                m_socket->Close();
                break;
        }
    }
}

void TCPClient::onRecvReverseSyncReq(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Recv Reverse Sync Req!");
    /*
     * 1.拆包
     * 2.检查目标文件(本机)是否存在，若不存在则返回错误码
     * 3.根据taskID建立到数据的映射
     * 4.收到的数据根据checksum建立到md5的映射
     * 5.循环检验本地文件
     * 6.返回重建文件的菜单：可校验一块返回一块，建立流水线作业
     */

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = Protocol::VerifyFileBlockInfosBuffer(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto blocksInfo = Protocol::GetFileBlockInfos(data->ToChars());
    auto pFilename = blocksInfo->DesPath();
    auto blocksize = blocksInfo->Splitsize();
    auto pFile = FileHelper::OpenFile(pFilename->str(), "r");

    if (pFile == nullptr)   //本地不存在该文件
    {
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_NO_SUCH_FILE, builder.CreateString(pFilename));
        builder.Finish(fbb);
        this->SendToClient(Opcode::ERROR_CODE, taskID,
                           MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        return;
    }

    //若存在该文件，则读取文件大小，小于等于块大小的直接传输，大于块大小的重新计算
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
    else if (pFile->Size() <= blocksize)
    {
        char buff[blocksize];
        auto count = pFile->ReadBytes(buff, blocksize);

        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateNewBlockAck(builder, builder.CreateString(blocksInfo->SrcPath()),
                                               builder.CreateString(blocksInfo->DesPath()),
                                               blocksize,
                                               pFile->Size(),
                                               Protocol::CreateNewBlock(builder, 0,
                                                                        pFile->Size(),
                                                                        builder.CreateString("-"),
                                                                        builder.CreateString(buff, count)));
        builder.Finish(fbb);
        this->SendToClient(Opcode::REVERSE_SYNC_ACK, taskID,
                           MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        return;
    }
    else
    {
        m_tasks[taskID][pFilename->str()] = blocksInfo;

        InspectorPtr p = InspectorPtr(new Inspector(taskID, pFilename->str(), blocksize));
        for (auto it : *blocksInfo->Infos())
        {
            ST_BlockInfo info;
            //info.filename = it->Filename()->str();
            //info.order = it->Order();
            info.offset = it->Offset();
            info.length = it->Length();
            info.checksum = it->Checksum();
            info.md5 = it->Md5()->str();
            p->AddInfos(info);
        }
        //p->StartGetBlocks(nullptr);
        p->StartGetBlocks(INSPECTOR_CALLBACK_FUNC(&TCPClient::onInspectBlockInfo, this));

        m_tasks[taskID][pFilename->str()] = nullptr;
    }
}

void TCPClient::SendToClient(Opcode op, uint32_t taskID, BytesPtr data)
{

    ST_PackageHeader header(op, taskID);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(data->ToChars(), data->Size()));
    m_socket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));
}

void
TCPClient::onInspectBlockInfo(uint32_t taskID, const ST_BlockInfo &blockInfo, const string &filename, size_t filesize)
{
    LogCheckConditionVoid(m_tasks[taskID][filename] != nullptr, "m_tasks map failed!");
    //char buff[100] = "abcdefg";
    //LOG_DEBUG("%s %s %s", string(buff, 0, 7).c_str(), string().append(buff, 0, 7).c_str(), string().assign(buff, 7).c_str()); // string(buff, 0, 7) will cause core dump
    LOG_DEBUG("Send Block: Offset: %lld Length: %d MD5: %s", blockInfo.offset, blockInfo.length, blockInfo.md5.c_str());

    auto &blocksInfo = m_tasks[taskID][filename];

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateNewBlockAck(builder, builder.CreateString(blocksInfo->SrcPath()),
                                           builder.CreateString(blocksInfo->DesPath()),
                                           blocksInfo->Splitsize(),
                                           filesize,
                                           Protocol::CreateNewBlock(builder, blockInfo.offset,
                                                                    blockInfo.length,
                                                                    builder.CreateString(blockInfo.md5),
                                                                    builder.CreateString(blockInfo.data)));
    builder.Finish(fbb);
    this->SendToClient(Opcode::REVERSE_SYNC_ACK,
                       taskID,
                       MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
}

void TCPClient::onRecvViewDirReq(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive View Dir Req!");
    /*
     * 1.拆包
     * 2.检查目标文件(本机)是否存在，若不存在则返回错误码
     * 3.读取文件列表，打包返回
     */

    auto pViewDirReq = flatbuffers::GetRoot<ViewDirReq>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pViewDirReq->Verify(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto dp = FileHelper::OpenDir(pViewDirReq->DesDir()->str());

    if (!dp)
    {
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_NO_SUCH_DIR, builder.CreateString(strerror(errno)));
        builder.Finish(fbb);

        this->SendToClient(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());
        return;
    }

    auto List = dp->AllFiles();

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Protocol::FileInfo> > vector1;

    for (auto &name:List)
    {
        auto fp = FileHelper::OpenFile(name, "r");
        if (fp)
        {
            vector1.push_back(Protocol::CreateFileInfo(builder,
                                                       builder.CreateString(FileHelper::GetRealPath(fp->Name())),
                                                       fp->Size()));
        }
    }
    auto fbb = Protocol::CreateViewDirAck(builder, builder.CreateVector(vector1));
    builder.Finish(fbb);

    this->SendToClient(Opcode::VIEW_DIR_ACK, taskID, builder.GetBufferPointer(), builder.GetSize());
}

void TCPClient::SendToClient(Protocol::Opcode op, uint32_t taskID, uint8_t *buff, uint32_t size)
{
    this->SendToClient(op, taskID, MsgHelper::CreateBytes(buff, size));
}

void MsgThread::SetArgs(const TCPClientPtr tcpClientPtr)
{
    m_ptr = tcpClientPtr;
}

void MsgThread::Runnable()
{
    m_ptr->Dispatch();
}
