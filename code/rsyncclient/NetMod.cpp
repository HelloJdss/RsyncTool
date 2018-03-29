//
// Created by carrot on 18-3-26.
//

#include "NetMod.h"
#include "BlockInfos_generated.h"
#include "ErrorCode_generated.h"
#include "NewBlockInfo_generated.h"

using namespace Protocol;
using namespace RsyncClient;

void NetMod::Run()
{
    this->CreateNewTask(Opcode::REVERSE_SYNC_REQ, "./test.txt", "./test1.txt");
}

void NetMod::Dispatch()
{
    ST_PackageHeader header;
    BytesPtr data;
    while (m_msgHelper.ReadMessage(header, &data))
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
    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = Protocol::VerifyErrorCodeBuffer(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto err = Protocol::GetErrorCode(data->ToChars());
    LOG_WARN("Receive Error Code: Task[%lu] : [%s] [%s]", taskID, Protocol::EnumNameErr(err->Code()),
             err->TIP()->c_str());
}

void NetMod::onRecvReverseSyncAck(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("RecvReverseSyncAck");

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = Protocol::VerifyNewBlockAckBuffer(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto pNewBlockAck = Protocol::GetNewBlockAck(data->ToChars());

    RTVector<FileBaseDataPtr> &vector1 = m_task2data[taskID];
    for (auto &it: vector1)
    {
        if (it->m_filePath == pNewBlockAck->SrcPath()->str())
        {
            it->m_newSize = pNewBlockAck->Length();
            if (it->m_pnewfile == nullptr)
            {
                it->m_pnewfile = FileHelper::CreateFilePtr(it->m_filePath + ".tmp", "w");
                it->m_pnewfile->SetSize(it->m_newSize);
            }

            auto pBlock = pNewBlockAck->Infos();
            LOG_DEBUG("%s", pBlock->Md5()->c_str());

            if (pBlock->Md5()->str() == "-")
            {
                LogCheckConditionVoid(pBlock->Data()->str().length() == pBlock->Length(), "Err!");
                it->m_pnewfile->WriteBytes(pBlock->Data()->str().data(), pBlock->Length(),
                                           pBlock->Offset(),
                                           true);
                it->m_processLen += pBlock->Length();
                LOG_INFO("Task[%lu] File[%s]: Reconstruct block[%lld +> %ld] success!", taskID, it->m_filePath.c_str(),
                         pBlock->Offset(), pBlock->Length());
            }
            else
            {
                //TODO:根据md5重建
                auto pData = m_task2generator[taskID]->GetDataByMd5(pBlock->Md5()->str());
                LogCheckConditionVoid(pData.length() == pBlock->Length(), "Err!");
                it->m_pnewfile->WriteBytes(pData.data(), pBlock->Length(),
                                           pBlock->Offset(),
                                           true);
                it->m_processLen += pBlock->Length();
                LOG_INFO("Task[%lu] File[%s]: Reconstruct(md5) block[%lld => %ld] success!", taskID,
                         it->m_filePath.c_str(), pBlock->Offset(), pBlock->Length());
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

void NetMod::createReverseSyncTask(uint32_t taskID, const string &src, const string &des, uint32_t blocksize)
{
    if (m_task2generator[taskID] == nullptr)
    {
        m_task2generator[taskID] = GeneratorPtr(new Generator);
    }

    auto pFileBaseData = FileBaseDataPtr(new FileBaseData);

    pFileBaseData->m_filePath = src;

    m_task2data[taskID].push_back(pFileBaseData);

    m_task2generator[taskID]->GenerateBlockInfos(src, blocksize);


    auto infos = m_task2generator[taskID]->GetBlockInfos(src);

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Protocol::BlockInfo> > blockVec;
    for (auto it : infos)
    {
        blockVec.push_back(Protocol::CreateBlockInfo(builder, it.offset,
                                                     it.length, it.checksum, builder.CreateString(it.md5)));
    }

    auto fbb = Protocol::CreateFileBlockInfos(builder, builder.CreateString(src), builder.CreateString(des),
                                              blocksize, builder.CreateVector(blockVec));
    builder.Finish(fbb);


    ST_PackageHeader header(Opcode::REVERSE_SYNC_REQ, 1);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
    m_serversocket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));

    m_serversocket->SetRecvTimeOut(30, 0);  //30秒延迟

    m_running = true;

    while (m_running)
    {
        auto count = m_serversocket->Receive(m_msgHelper.GetBuffer() + m_msgHelper.GetStartIndex(),
                                             m_msgHelper.GetRemainBytes());
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
