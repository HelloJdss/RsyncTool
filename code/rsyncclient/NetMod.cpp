//
// Created by carrot on 18-3-26.
//

#include "NetMod.h"
#include "BlockInfos_generated.h"
#include "ErrorCode_generated.h"
#include "ReconstructList_generated.h"

using namespace Protocol;
using namespace RsyncClient;

void NetMod::Run()
{
    this->CreateNewTask(Opcode::REVERSE_SYNC_REQ, "./test.txt", "./test.txt");
}

void NetMod::Dispatch()
{
    ST_PackageHeader header;
    BytesPtr data;
    while(m_msgHelper.ReadMessage(header, &data))
    {
        LOG_INFO("Recv Meg from[%s]: op[%s], taskID[%lu], dataLength:[%lu]", m_serversocket->GetEndPoint().c_str(),
                 Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());

        switch (header.getOpCode())
        {
            case Opcode::REVERSE_SYNC_ACK:
                onRecvReverseSyncAck(header.getTaskId(), data);
                break;
            case Opcode::ERROR_CODE:
                onRecvErrorCode(header.getTaskId(), data);
                break;
            default:
                LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_serversocket->GetEndPoint().c_str());
                m_serversocket->Close();
                break;
        }
    }
}

NetMod::~NetMod()
{
    m_running = false;
    m_serversocket = nullptr;
}

void NetMod::onRecvErrorCode(uint32_t taskID, BytesPtr data)
{
    auto err = Protocol::GetErrorCode(data->ToChars());
    LOG_WARN("Receive Error Code: Task[%lu] : [%s] [%s]", taskID, Protocol::EnumNameErr(err->Code()),
             err->TIP()->c_str());
}

void NetMod::onRecvReverseSyncAck(uint32_t taskID, BytesPtr data)
{
    auto newblock = Protocol::GetNewBlockAck(data->ToChars());
    RTVector<FileBaseDataPtr> &vector1 = m_task2data[taskID];
    for (auto &it: vector1)
    {
        if (it->m_filePath == newblock->SrcPath()->str())
        {
            it->m_newSize = newblock->Length();
            if (it->m_pnewfile == nullptr)
            {
                it->m_pnewfile = FileHelper::CreateFilePtr(it->m_filePath.append(".tmp"), "w");
                it->m_pnewfile->SetSize(it->m_newSize);
            }
            auto ninfo = newblock->Infos();
            LOG_DEBUG("%s", ninfo->Md5()->c_str());
            if (ninfo->Md5()->str() == "-")
            {
                it->m_pnewfile->WriteBytes(reinterpret_cast<char const *>(ninfo->Data()->data()), ninfo->Length(),
                                           ninfo->Offset(),
                                           true);
                it->m_processLen += ninfo->Length();
                LOG_INFO("Task[%lu] File[%s]: Reconstruct block[%ld] success!", taskID, it->m_filePath.c_str(),
                         ninfo->Order());
            }
            else
            {
                //TODO:根据md5重建
            }

            if (it->m_processLen == it->m_newSize)
            {
                LOG_INFO("Task[%lu] File[%s]: Reconstruct Finished!", taskID, it->m_filePath.c_str());
                //TODO: 重建完成，重命名替换
            }
        }
    }
}

void NetMod::CreateNewTask(Protocol::Opcode op, const string &src, const string &des)
{
    static uint32_t taskID = 0;
    ++taskID;
    switch (op)
    {
        case Opcode::REVERSE_SYNC_REQ:
            createReverseSyncTask(taskID, src, des);
            break;
        default:
            break;
    }
}

void NetMod::createReverseSyncTask(uint32_t taskID, const string &src, const string &des)
{
    if (m_task2generator[taskID] == nullptr)
    {
        m_task2generator[taskID] = GeneratorPtr(new Generator);
    }

    auto baseptr = FileBaseDataPtr(new FileBaseData);

    baseptr->m_filePath = src;

    m_task2data[taskID].push_back(baseptr);

    m_task2generator[taskID]->GenerateBlockInfos(src);


    auto infos = m_task2generator[taskID]->GetBlockInfos(src);

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Protocol::BlockInfo> > blockVec;
    for (auto it : infos)
    {
        blockVec.push_back(
                Protocol::CreateBlockInfo(builder, builder.CreateString(it.filename), it.order, it.offset,
                                          it.length, it.checksum, builder.CreateString(it.md5)));
    }

    auto fbb = Protocol::CreateFileBlockInfos(builder, builder.CreateString(src),
                                              builder.CreateString(des), 512, builder.CreateVector(blockVec));
    builder.Finish(fbb);


    ST_PackageHeader header(Opcode::REVERSE_SYNC_REQ, 1);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
    m_serversocket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));

    m_serversocket->SetRecvTimeOut(30, 0);  //30秒延迟

    m_running = true;

    while (m_running)
    {
        auto count = m_serversocket
                ->Receive(m_msgHelper.GetBuffer() + m_msgHelper.GetStartIndex(), m_msgHelper.GetRemainBytes());
        if (count > 0)
        {
            m_msgHelper.AddCount(count);
            this->Dispatch();
        }
        else if (count == 0)
        {
            LOG_WARN("Time out!");
            m_running = false;
            return;
        }
    }
}

bool NetMod::Init()
{
    m_serversocket = NetHelper::CreateTCPSocket(INET);
    try
    {
        m_serversocket->Connect(SocketAddress(INADDR_ANY, 52077));
        return true;
    }
    catch (int err)
    {
        LOG_LastError();
        return false;
    }
}
