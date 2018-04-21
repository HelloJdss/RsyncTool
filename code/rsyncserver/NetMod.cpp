//
// Created by carrot on 18-3-15.
//
#include <unistd.h>
#include "cm_define.h"
#include "NetMod.h"

#include "ViewDir_generated.h"
#include "Synchronism_generated.h"
#include "Cmd_generated.h"


using namespace Protocol;
using namespace RsyncServer;

void NetMod::Init(uint16_t port)
{
    m_listenSocket = NetHelper::CreateTCPSocket(INET, false);
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
        readableSockets.clear();

        timeval tm;
        tm.tv_sec = 30;
        tm.tv_usec = 0; //设定延时，及时清理内存

        if (NetHelper::Select(&m_readBlockSockets, &readableSockets, nullptr, nullptr, nullptr, nullptr, &tm) > 0)
        {
            for (const TCPSocketPtr &socket : readableSockets)
            {
                if (socket == m_listenSocket)    // new client connection
                {
                    SocketAddress socketAddress;
                    auto newSocket = m_listenSocket->Accept(socketAddress);

                    m_readBlockSockets.push_back(newSocket);
                    LOG_INFO("Accept Connection[%s]", newSocket->GetEndPoint().c_str());
                    m_clientMap[newSocket] = TCPClientPtr(new TCPClient(newSocket));

                    //为每个客户端单独创建一个线程
                    auto ptr = MsgThreadPtr(new MsgThread);
                    ptr->SetArgs(m_clientMap[newSocket]);
                    if (ptr->Start())
                    {
                        ptr->Detach();
                    }
                    m_threads.push_back(ptr);
                }
                else    //receive data from an old client
                {
                    if (m_clientMap.find(socket) != m_clientMap.end())
                    {

                        auto pClient = m_clientMap[socket];
                        auto count = socket->Receive(pClient->m_msgHelper.GetBuffer() +
                                                     pClient->m_msgHelper.GetStartIndex(),
                                                     pClient->m_msgHelper.GetRemainBytes());
                        if (count > 0)
                        {
                            pClient->m_msgHelper.AddCount(count);


                            //RunThread(this, &m_clientMap[socket], true);

                            //m_clientMap[socket]->Dispatch();
                        }
                        else if (count == 0)
                        {
                            //client disconnect
                            LOG_INFO("RemoteClient[%s] Disconnected!", socket->GetEndPoint().c_str());
                            socket->Close();
                        }
                        else
                        {
                            LOG_ERROR("Client[%s] err: %s", socket->GetEndPoint().c_str(), strerror(errno));
                            socket->Close();
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
        else
        {
            LOG_DEBUG("select: %d", m_readBlockSockets.size());
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
            m_threads.erase(m_threads.begin() + i); //移除已经结束的线程
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
    pthread_mutex_lock(&m_mutex); //同一时间只允许一个线程处理调度

    ST_PackageHeader header;
    BytesPtr data;
    while (m_msgHelper.ReadMessage(header, &data))
    {
        //LOG_TRACE("Client[%s] has Message!", m_socket->GetEndPoint().c_str());
        LOG_DEBUG("Recv Meg from[%s]: op[%s], taskID[%u], dataLength:[%u]", m_socket->GetEndPoint().c_str(),
                  Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());

        Protocol::Err err = Err_DO_NOT_REPLY;
        switch (header.getOpCode())
        {
            case Opcode::ERROR_CODE:
                onRecvErrorCode(header.getTaskId(), data);
                break;

            case Opcode::VIEW_DIR_REQ:
                err = onRecvViewDirReq(header.getTaskId(), data);
                break;

            case Opcode::SYNC_FILE:
                err = onRecvSyncFile(header.getTaskId(), data);
                break;

            case Opcode::FILE_DIGEST :
                err = onRecvFileDigest(header.getTaskId(), data);
                break;

            case Opcode::REBUILD_INFO:
                err = onRecvRebuildInfo(header.getTaskId(), data);
                break;

            case Opcode::REBUILD_CHUNK:
                err = onRecvRebuildChunk(header.getTaskId(), data);
                break;

            default:
                LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_socket->GetEndPoint().c_str());
                m_socket->Close();
                break;
        }

        if (err != Err_DO_NOT_REPLY)
        {
            this->SendErrToClient(header.getTaskId(), err);
        }
    }
    pthread_mutex_unlock(&m_mutex);
}

void TCPClient::SendToClient(Opcode op, uint32_t taskID, BytesPtr data)
{
    if (!m_socket->IsAvailable())
    {
        return;
    }

    ST_PackageHeader header(op, taskID);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(data->ToChars(), data->Size()));

    if (m_socket->Send(bytes->ToChars(), static_cast<int>(bytes->Size())) == -1)
    {
        LOG_ERROR("client[%s] send data failed! [%s]", m_socket->GetEndPoint().c_str(), strerror(errno));
        m_socket->Close();
    }
}

void TCPClient::SendToClient(Protocol::Opcode op, uint32_t taskID, uint8_t *buff, uint32_t size)
{
    this->SendToClient(op, taskID, MsgHelper::CreateBytes(buff, size));
}

void TCPClient::SendErrToClient(uint32_t taskID, Protocol::Err err, const string tip)
{
    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateErrorCode(builder, err, builder.CreateString(tip));
    builder.Finish(fbb);
    this->SendToClient(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());
}


bool TCPClient::IsAvailable()
{
    return m_socket->IsAvailable();
}

void MsgThread::SetArgs(const TCPClientPtr &tcpClientPtr)
{
    m_ptr = tcpClientPtr;
}

void MsgThread::Runnable()
{
    while (m_ptr && m_ptr->IsAvailable())
    {
        m_ptr->Dispatch();
        usleep(125 * 1000); //休眠0.125s
    }
    LOG_WARN("Client[%s] Thread Terminal!", m_ptr->m_socket->GetEndPoint().c_str());
}

/**
 * 1.拆包
 * 2.检查目标文件(本机)是否存在，若不存在则返回错误码
 * 3.读取文件列表，打包返回
 */
Err TCPClient::onRecvViewDirReq(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive View Dir Req!");


    auto pViewDirReq = flatbuffers::GetRoot<ViewDirReq>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pViewDirReq->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    auto pTaskInfo = &m_tasks[taskID];
    pTaskInfo->m_type = TaskType::ViewDir;
    pTaskInfo->m_taskID = taskID;
    pTaskInfo->m_des = pViewDirReq->DesDir()->str();
    pTaskInfo->Launch();

    auto dp = FileHelper::OpenDir(pViewDirReq->DesDir()->str());

    if (!dp)
    {
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_NO_SUCH_DIR, builder.CreateString(strerror(errno)));
        builder.Finish(fbb);

        this->SendToClient(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());

        return Err_DO_NOT_REPLY;
    }

    auto List = dp->AllFiles();

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Protocol::FileInfo> > vector1;

    for (auto &name:List)
    {
        if (name.back() == '/')
        {
            //如果是目录
            //auto dp1 = FileHelper::OpenDir(name);

            //if(dp1)
            //{
                vector1.push_back(Protocol::CreateFileInfo(builder,
                                                           builder.CreateString(FileHelper::GetRealPath(name) + "/"),
                                                           0, 0));
                                                           /*dp1->Stat().st_size, dp1->Stat().st_mtim.tv_sec * 1000 +
                                                                                dp1->Stat().st_mtim.tv_nsec / 1000));*/
           // }
            //modify: 频繁打开目录过于缓慢，取消读取目录信息
            continue;
        }

        auto fp = FileHelper::OpenFile(name, "r");
        if (fp)
        {
            vector1.push_back(Protocol::CreateFileInfo(builder,
                                                       builder.CreateString(FileHelper::GetRealPath(fp->Path())),
                                                       fp->Size(), fp->Stat().st_mtim.tv_sec * 1000 +
                                                                   fp->Stat().st_mtim.tv_nsec / 1000));
        }
    }
    auto fbb = Protocol::CreateViewDirAck(builder, builder.CreateVector(vector1));
    builder.Finish(fbb);

    this->SendToClient(Opcode::VIEW_DIR_ACK, taskID, builder.GetBufferPointer(), builder.GetSize());

    return Err_DO_NOT_REPLY;
}

/**
 * 1.拆包
 * 2.根据Des所示的文件生成文件摘要信息，若文件不存在，则块信息填空
 */

Protocol::Err TCPClient::onRecvSyncFile(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Sync File Req!");

    auto pSyncFile = flatbuffers::GetRoot<SyncFile>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pSyncFile->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    auto pTaskInfo = &m_tasks[taskID];
    pTaskInfo->m_type = TaskType::Push;
    pTaskInfo->m_taskID = taskID;
    pTaskInfo->m_src = pSyncFile->SrcPath()->str();
    pTaskInfo->m_des = pSyncFile->DesPath()->str();
    pTaskInfo->Launch();

    if (pSyncFile->DesPath()->str().back() == '/')
    {
        //如果是目录则创建目录，并返回成功
        FileHelper::MakeDir(pSyncFile->DesPath()->str());
        return Err_SUCCESS;
    }

    pTaskInfo->m_generatorPtr = Generator::NewGenerator(pSyncFile->DesPath()->str());

    flatbuffers::FlatBufferBuilder builder;

    if (pTaskInfo->m_generatorPtr) //文件存在
    {
        std::vector<flatbuffers::Offset<ChunkInfo> > vector1;

        auto &DigestVec = pTaskInfo->m_generatorPtr->GetChunkDigestVec();
        for (const auto &item : DigestVec)
        {
            vector1.push_back(Protocol::CreateChunkInfo(builder, item.offset, item.length, item.checksum,
                                                        builder.CreateString(item.md5)));
        }

        auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(pTaskInfo->m_src),
                                              builder.CreateString(pTaskInfo->m_des),
                                              pTaskInfo->m_generatorPtr->GetSplit(),
                                              builder.CreateVector(vector1));
        builder.Finish(fbb);
        this->SendToClient(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());
        return Err_DO_NOT_REPLY;
    }

    //文件不存在,分块大小为0
    auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(pTaskInfo->m_src),
                                          builder.CreateString(pTaskInfo->m_des),
                                          0);
    builder.Finish(fbb);
    this->SendToClient(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());

    return Err_DO_NOT_REPLY;
}

Err TCPClient::onRecvRebuildInfo(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Info!");

    auto pRebuildInfo = flatbuffers::GetRoot<RebuildInfo>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildInfo->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    auto pTaskInfo = &m_tasks[taskID];
    pTaskInfo->m_rebuild_size = pRebuildInfo->Size();

    return Err_DO_NOT_REPLY;
}

Err TCPClient::onRecvRebuildChunk(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Chunk!");

    auto pRebuildChunk = flatbuffers::GetRoot<RebuildChunk>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildChunk->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    auto pTaskInfo = &m_tasks[taskID];

    if (pTaskInfo->m_pf == nullptr)
    {
        pTaskInfo->m_pf = FileHelper::OpenFile(pTaskInfo->m_des + ".tmp", "w"); //读取服务端名称
        LogCheckCondition(pTaskInfo->m_pf, Err_NO_SUCH_FILE, "file Ptr is <null>");
    }

    if (pRebuildChunk->IsMd5())
    {
        LogCheckCondition(pTaskInfo->m_generatorPtr, Err_TASK_INFO_INCOMPLETE, "generator Ptr is <null>");
        auto pData = pTaskInfo->m_generatorPtr->GetChunkDataByMd5(pRebuildChunk->Data()->str());
        LogCheckCondition(pData.length() == pRebuildChunk->Length(), Err_UNKNOWN, "Err!");
        auto count = pTaskInfo->m_pf->WriteBytes(pData.data(), pData.length(), pRebuildChunk->Offset(), false);
        if (count != pData.length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     pTaskInfo->m_des.c_str(), count, pData.length());
        }
        pTaskInfo->m_processLen += pData.length();
        LOG_TRACE("Task[%lu] File[%s]: Reconstruct(md5) block[%lld +=> %ld] success!", taskID,
                  pTaskInfo->m_des.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }
    else
    {
        auto count = pTaskInfo->m_pf
                ->WriteBytes(pRebuildChunk->Data()->data(), pRebuildChunk->Length(), pRebuildChunk->Offset(),
                             false);
        if (count != pRebuildChunk->Length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     pTaskInfo->m_des.c_str(), count, pRebuildChunk->Length());
        }
        pTaskInfo->m_processLen += pRebuildChunk->Length();
        LOG_TRACE("Task[%lu] File[%s]: Reconstruct(data) block[%lld +=> %ld] success!", taskID,
                  pTaskInfo->m_des.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }

    if (pTaskInfo->m_processLen == pTaskInfo->m_rebuild_size)
    {
        LOG_INFO("Task[%lu] File[%s]: Reconstruct Finished!", taskID, pTaskInfo->m_des.c_str());

        pTaskInfo->m_pf = nullptr; //关闭文件读写指针，使缓冲区写入到文件完成
        //重建完成，重命名替换

        FileHelper::Rename(pTaskInfo->m_des + ".tmp", pTaskInfo->m_des);

        pTaskInfo->Finish();

        //发送成功的回包
        /*
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_SUCCESS);
        builder.Finish(fbb);
        this->SendToClient(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());
         */
        return Err_SUCCESS;
    }
    return Err_DO_NOT_REPLY;
}

Err TCPClient::onRecvFileDigest(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive File Digest");

    auto pFileDigest = flatbuffers::GetRoot<FileDigest>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pFileDigest->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    auto pTaskInfo = &m_tasks[taskID];
    if (pTaskInfo->m_taskID == taskID) //先行创建过的任务，需要校验
    {
        LogCheckCondition(pTaskInfo->m_des == pFileDigest->DesPath()->str(), Err_TASK_CONFLICT,
                          "Task[%lu] Conflict!", taskID);
        LogCheckCondition(pTaskInfo->m_src == pFileDigest->SrcPath()->str(), Err_TASK_CONFLICT,
                          "Task[%lu] Conflict!", taskID);
    }


    pTaskInfo->m_taskID = taskID;
    pTaskInfo->m_type = Pull_File;
    pTaskInfo->m_des = pFileDigest->DesPath()->str();
    pTaskInfo->m_src = pFileDigest->SrcPath()->str();
    pTaskInfo->Launch();

    auto fp = FileHelper::OpenFile(pTaskInfo->m_des, "rb");
    LogCheckCondition(fp, Err_NO_SUCH_FILE, "fp is <null>");

    //先发送文件信息

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateRebuildInfo(builder, fp->Size());
    builder.Finish(fbb);

    this->SendToClient(Opcode::REBUILD_INFO, taskID, builder.GetBufferPointer(), builder.GetSize());

    if (pFileDigest->Splitsize() == 0) //对方不存在该文件，则直接传输整个文件
    {
        auto pInspector = Inspector::NewInspector(taskID, fp);
        pInspector->BeginInspect(INSPECTOR_CALLBACK_FUNC(&TCPClient::onInspectCallBack, this));
    }
    else
    {
        //根据摘要重建
        auto pInspector = Inspector::NewInspector(taskID, fp, pFileDigest->Splitsize());

        //添加摘要信息

        for (const auto &item : *pFileDigest->Infos())
        {
            ST_BlockInformation information;
            information.offset = item->Offset();
            information.length = item->Length();
            information.checksum = item->Checksum();
            information.md5 = item->Md5()->str();
            pInspector->AddDigestInfo(information);
        }

        pInspector->BeginInspect(INSPECTOR_CALLBACK_FUNC(&TCPClient::onInspectCallBack, this));
    }
    return Err_DO_NOT_REPLY;
}

void TCPClient::onInspectCallBack(uint32_t taskID, const ST_BlockInformation &info)
{
    flatbuffers::FlatBufferBuilder builder;

    flatbuffers::Offset<RebuildChunk> fbb;
    if (info.md5 == "-")
    {
        fbb = Protocol::CreateRebuildChunk(builder, info.offset, info.length, false, builder.CreateString(info.data));
    }
    else
    {
        fbb = Protocol::CreateRebuildChunk(builder, info.offset, info.length, true, builder.CreateString(info.md5));
    }
    builder.Finish(fbb);

    this->SendToClient(Opcode::REBUILD_CHUNK, taskID, builder.GetBufferPointer(), builder.GetSize());
}

void TCPClient::onRecvErrorCode(uint32_t taskID, BytesPtr data)
{
    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = Protocol::VerifyErrorCodeBuffer(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto err = Protocol::GetErrorCode(data->ToChars());

    if (err->Code() == Err_SUCCESS)
    {
        m_tasks[taskID].Finish();
    }
    else
    {
        m_tasks[taskID].Abort(err->TIP()->str());
    }

    LOG_WARN("Receive Err Code: Task[%lu] : [%s] [%s]", taskID, Protocol::EnumNameErr(err->Code()),
             err->TIP() == nullptr ? "<null>" : err->TIP()->c_str());
}


