//
// Created by carrot on 18-3-26.
//

#include <fcntl.h>
#include "NetMod.h"
#include "MainMod.h"
#include "Inspector.h"
#include "BlockInfos_generated.h"
//#include "ErrorCode_generated.h"
#include "NewBlockInfo_generated.h"

#include "ViewDir_generated.h"
#include "Synchronism_generated.h"
#include "tinyxml2.h"
#include "Cmd_generated.h"


using namespace Protocol;
using namespace RsyncClient;
using namespace tinyxml2;

int NetMod::LaunchTask()
{
    int count = 0;
    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it)   //逐个启动任务
    {
        switch (it->second.m_type)
        {
            case TaskType::ViewDir:
            case TaskType::Push:
            case TaskType::Pull_Dir:
            case TaskType::Pull_File:
                if (it->second.m_launch_count++ >= 3)
                {
                    it->second.m_type = TaskType::Abort;
                    it->second.m_err = "Launch count over!";
                    LOG_WARN("Task[%lu] Abort, Because it launched too much time!", it->second.m_taskID);
                }
                break;
            default:
                break;
        }

        switch (it->second.m_type)
        {
            case TaskType::ViewDir:
                launchViewDirTask(it->second.m_taskID);
                ++count;
                break;
            case TaskType::Push:
                launchPushTask(it->second.m_taskID);
                ++count;
                break;
            case TaskType::Pull_Dir :
            case TaskType::Pull_File :
                launchPullTask(it->second.m_taskID);
                ++count;
                break;
            default:
                break;
        }
    }
    LOG_DEBUG("Launch [%d] Tasks", count);
    return count;
}

void NetMod::Run()
{
    //this->CreateNewTask(Opcode::REVERSE_SYNC_REQ, "./test.txt", "./test1.txt");


    m_serverSocket->SetRecvTimeOut(10, 0);  //10秒延迟，若10秒仍收不到服务器的消息，则执行下一轮任务

    m_running = true;

    LogCheckConditionVoid(LaunchTask() > 0, "No Tasks!");

    while (m_running)
    {
        try
        {
            auto count = m_serverSocket->Receive(m_msgHelper.GetBuffer() + m_msgHelper.GetStartIndex(),
                                                 m_msgHelper.GetRemainBytes());
            if (count > 0)
            {
                m_msgHelper.AddCount(count);
                this->Dispatch();
            }
            else if (count == 0)
            {
                LOG_WARN("Server Close Connection!");
                m_running = false;
                return;
            }
        }
        catch (int err)
        {
            LOG_FATAL("Catch Exception: [%s]", strerror(err));
            if(LaunchTask() == 0)
            {
                m_running = false;
            }
        }
    }
}

void NetMod::Dispatch()
{
    ST_PackageHeader header;
    BytesPtr data;
    while (m_msgHelper.ReadMessage(header, &data))
    {
        LOG_INFO("Recv Meg from[%s]: op[%s], taskID[%lu], dataLength:[%lu]", m_serverSocket->GetEndPoint().c_str(),
                 Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());

        switch (header.getOpCode())
        {
            case Opcode::REVERSE_SYNC_ACK:
                onRecvReverseSyncAck(header.getTaskId(), data);
                break;
            case Opcode::ERROR_CODE:
                onRecvErrorCode(header.getTaskId(), data);
                break;
            case Opcode::VIEW_DIR_ACK :
                onRecvViewDirAck(header.getTaskId(), data);
                break;
            case Opcode::FILE_DIGEST :
                onRecvFileDigest(header.getTaskId(), data);
                break;

            case Opcode::REBUILD_INFO:
                onRecvRebuildInfo(header.getTaskId(), data);
                break;

            case Opcode::REBUILD_CHUNK:
                onRecvRebuildChunk(header.getTaskId(), data);
                break;

            default:
                LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_serverSocket->GetEndPoint().c_str());
                m_serverSocket->Close();
                break;
        }
    }
}

NetMod::~NetMod()
{
    m_running = false;
    m_serverSocket = nullptr;
}

void NetMod::onRecvErrorCode(uint32_t taskID, BytesPtr data)
{
    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = Protocol::VerifyErrorCodeBuffer(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    auto err = Protocol::GetErrorCode(data->ToChars());

    if(err->Code() == Err_SUCCESS)
    {
        m_tasks[taskID].m_type = TaskType::Finished;
        LOG_INFO("Task[%lu] Finished!", taskID);
    }

    LOG_WARN("Receive Error Code: Task[%lu] : [%s] [%s]", taskID, Protocol::EnumNameErr(err->Code()),
             err->TIP() == nullptr ? "<null>" : err->TIP()->c_str());
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
                it->m_pnewfile = FileHelper::OpenFile(it->m_filePath + ".tmp", "w");
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
                auto pData = m_task2generator[taskID]->GetChunkDataByMd5(pBlock->Md5()->str());
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

    m_task2generator[taskID]->Generate(src, blocksize);


    auto infos = m_task2generator[taskID]->GetChunkDigestVec();

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
    m_serverSocket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));

    m_serverSocket->SetRecvTimeOut(30, 0);  //30秒延迟

    m_running = true;

    while (m_running)
    {
        auto count = m_serverSocket->Receive(m_msgHelper.GetBuffer() + m_msgHelper.GetStartIndex(),
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
    m_serverSocket = NetHelper::CreateTCPSocket(INET);
    try
    {
        if (m_serverIp.empty())
        {
            m_serverSocket->Connect(SocketAddress(INADDR_ANY, m_serverPort));
        }
        else
        {
            m_serverSocket->Connect(SocketAddress(m_serverIp.c_str(), m_serverPort));
        }
        return true;
    }
    catch (int err)
    {
        LOG_LastError();
        return false;
    }
}

void
NetMod::AddTask(TaskType taskType, string const *src, string const *des, const string &desIP, uint16_t desPort)
{

    m_serverIp = desIP;
    m_serverPort = desPort;

    switch (taskType)
    {
        case TaskType::Push:
            LogCheckConditionVoid(src, "src Path is <null>");
            LogCheckConditionVoid(des, "des Dir is <null>");
            createPushTask(*src, *des);
            break;
        case TaskType::Pull_File:
            LogCheckConditionVoid(src, "src Dir is <null>");
            LogCheckConditionVoid(des, "des Path is <null>");
            createPullTask(*src, *des);
            break;

        case TaskType::ViewDir:
            LogCheckConditionVoid(des, "des Path is <null>");
            createViewDirTask(*des);
            break;
        default:
            LOG_ERROR("Unknown TaskType!");
            break;
    }
}

void NetMod::createViewDirTask(const string &desDir)
{
    ST_TaskInfo newTask;
    newTask.m_taskID = ++m_taskIndex;
    newTask.m_type = TaskType::ViewDir;
    newTask.m_des = desDir;

    m_tasks[newTask.m_taskID] = newTask;
    LOG_INFO("Add Task: [%lu] Type: [View Dir] Des: [%s]", newTask.m_taskID, newTask.m_des.c_str());
}

void NetMod::launchViewDirTask(uint32_t taskID)
{
    LogCheckConditionVoid(m_tasks[taskID].m_type == TaskType::ViewDir, "Launch Task[%lu] Failed!", taskID);

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateViewDirReq(builder, builder.CreateString(m_tasks[taskID].m_des));
    builder.Finish(fbb);

    this->SendToServer(Opcode::VIEW_DIR_REQ, taskID, builder.GetBufferPointer(), builder.GetSize());

    LOG_INFO("Launch Task: [%lu] Type: [View Dir] Des: [%s]", m_tasks[taskID].m_taskID, m_tasks[taskID].m_des.c_str());
}

void NetMod::onRecvViewDirAck(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive View Dir Ack");

    auto pViewDirAck = flatbuffers::GetRoot<ViewDirAck>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pViewDirAck->Verify(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    FilePtr fp;
    XMLDocument document;
    XMLElement *root;


    if ((FileHelper::Access(g_Configuration->m_view_output, F_OK) == 0) &&  //文件已经存在，则添加
        ((fp = FileHelper::OpenFile(g_Configuration->m_view_output, "r+")) &&   //必须使用r，可读可写，
         fp->Size() <= g_Configuration->m_view_output_max_size)) //若超过大小，则重置
    {
        if (document.LoadFile(fp->GetPointer()) != 0)
        {
            LOG_ERROR("task[%lu] Abort, Err: %s", taskID, document.ErrorStr());
            m_tasks[taskID].m_type = TaskType::Abort;
            m_tasks[taskID].m_err = document.ErrorStr();
            return;
        }
        root = document.RootElement();
    }
    else
    {
        fp = FileHelper::OpenFile(g_Configuration->m_view_output, "w+");
        document.InsertFirstChild(document.NewDeclaration()); //添加声明
        root = document.NewElement("View");
        document.InsertEndChild(root);
    }


    XMLElement *des = document.NewElement("Destination");
    des->SetAttribute("IP", m_serverIp.c_str());
    des->SetAttribute("Path", m_tasks[taskID].m_des.c_str());
    des->SetAttribute("Time", utc_timer().utc_fmt);
    root->InsertEndChild(des);

    uint32_t PullDirTaskID = 0;

    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it)
    {
        LOG_DEBUG("%d %d %s %s", it->second.m_taskID, it->second.m_type, it->second.m_src.c_str(),
                  it->second.m_des.c_str());
        if (it->second.m_type == TaskType::Pull_Dir && it->second.m_des == m_tasks[taskID].m_des)
        {
            PullDirTaskID = it->second.m_taskID;
        }
    }

    LOG_DEBUG("Pull_Dir_ID: %lu", PullDirTaskID);

    int i = 1;
    for (auto item : *pViewDirAck->FileList())
    {
        LOG_INFO("%d: Path: [%s] Size: [%lld]", i++, item->FilePath()->c_str(), item->FileSize());

        //save as xml:
        XMLElement *pInfo = document.NewElement("File");
        pInfo->SetAttribute("Path", item->FilePath()->c_str());
        pInfo->SetAttribute("Size", item->FileSize());
        des->InsertEndChild(pInfo);

        if (PullDirTaskID != 0)
        {
            createPullTask(
                    m_tasks[PullDirTaskID].m_src + item->FilePath()->str().substr(m_tasks[PullDirTaskID].m_des.size()),
                    item->FilePath()->str());
        }
    }

    fp = nullptr; //关闭已打开的文件指针

    if (document.SaveFile(g_Configuration->m_view_output.c_str()) != 0)
    {
        m_tasks[taskID].m_type = TaskType::Error;
        m_tasks[taskID].m_err = document.ErrorStr();
        return;
    }

    m_tasks[taskID].m_type = TaskType::Finished;

    if (PullDirTaskID != 0)
    {
        m_tasks[PullDirTaskID].m_type = TaskType::Finished;
    }
}

void NetMod::createPushTask(const string &srcPath, const string &desDir) // src可能是文件或目录，des一定是目录
{
    if (srcPath.back() == '/') //目录
    {
        auto dp = FileHelper::OpenDir(srcPath);
        LogCheckConditionVoid(dp, "Open Dir Failed!");
        auto vec = dp->AllFiles();
        for (auto &item : vec)
        {
            ST_TaskInfo newTask;
            newTask.m_taskID = ++m_taskIndex;
            newTask.m_type = TaskType::Push;
            newTask.m_src = item;
            newTask.m_des = desDir + item.substr(srcPath.size());
            m_tasks[newTask.m_taskID] = newTask;
            LOG_INFO("Add Task: [%lu] Type: [Push] Src: [%s] Des: [%s]", newTask.m_taskID, newTask.m_src.c_str(),
                     newTask.m_des.c_str());
        }
    }
    else
    {
        auto fp = FileHelper::OpenFile(srcPath, "r");
        LogCheckConditionVoid(fp, "Open File Failed!");
        ST_TaskInfo newTask;
        newTask.m_taskID = ++m_taskIndex;
        newTask.m_type = TaskType::Push;
        newTask.m_src = srcPath;
        newTask.m_des = desDir + fp->BaseName();
        m_tasks[newTask.m_taskID] = newTask;
        LOG_INFO("Add Task: [%lu] Type: [Push] Src: [%s] Des: [%s]", newTask.m_taskID, newTask.m_src.c_str(),
                 newTask.m_des.c_str());
    }
}

void NetMod::launchPushTask(uint32_t taskID)
{
    LogCheckConditionVoid(m_tasks[taskID].m_type == TaskType::Push, "Launch Task[%lu] Failed!", taskID);

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateSyncFile(builder, builder.CreateString(m_tasks[taskID].m_src),
                                        builder.CreateString(m_tasks[taskID].m_des));
    builder.Finish(fbb);

    this->SendToServer(Opcode::SYNC_FILE, taskID, builder.GetBufferPointer(), builder.GetSize());

    LOG_INFO("Launch Task: [%lu] Type: [Push] Src: [%s] Des: [%s]", m_tasks[taskID].m_taskID,
             m_tasks[taskID].m_src.c_str(), m_tasks[taskID].m_des.c_str());
}

void NetMod::SendToServer(Protocol::Opcode op, uint32_t taskID, uint8_t *buf, uint32_t size)
{
    ST_PackageHeader header(op, taskID);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(buf, size));
    m_serverSocket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));
}

void NetMod::onRecvFileDigest(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive File Digest");

    auto pFileDigest = flatbuffers::GetRoot<FileDigest>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pFileDigest->Verify(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    if (m_tasks[taskID].m_taskID == taskID) //先行创建过的任务，需要校验
    {
        LogCheckConditionVoid(m_tasks[taskID].m_des == pFileDigest->DesPath()->str(), "Task[%lu] Conflict!", taskID);
        LogCheckConditionVoid(m_tasks[taskID].m_src == pFileDigest->SrcPath()->str(), "Task[%lu] Conflict!", taskID);
    }

    m_tasks[taskID].m_des = pFileDigest->DesPath()->str();
    m_tasks[taskID].m_src = pFileDigest->SrcPath()->str();

    auto fp = FileHelper::OpenFile(m_tasks[taskID].m_src, "rb");
    LogCheckConditionVoid(fp, "fp is <null>");

    //先发送文件信息

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateRebuildInfo(builder, fp->Size());
    builder.Finish(fbb);

    this->SendToServer(Opcode::REBUILD_INFO, taskID, builder.GetBufferPointer(), builder.GetSize());

    if (pFileDigest->Splitsize() == 0) //对方不存在该文件，则直接传输整个文件
    {
        auto pInspector = Inspector::NewInspector(taskID, fp, 1024);
        pInspector->BeginInspect(INSPECTOR_CALLBACK_FUNC(&NetMod::onInspectCallBack, this));
    }
    else
    {
        //TODO: 根据摘要重建
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

        pInspector->BeginInspect(INSPECTOR_CALLBACK_FUNC(&NetMod::onInspectCallBack, this));
    }
}

void NetMod::onInspectCallBack(uint32_t taskID, const ST_BlockInformation &info)
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

    this->SendToServer(Opcode::REBUILD_CHUNK, taskID, builder.GetBufferPointer(), builder.GetSize());
}

void NetMod::createPullTask(const string &srcDir, const string &desPath)
{
    if (desPath.back() == '/') //目录
    {
        //先创建浏览目录任务
        ST_TaskInfo newTask;
        newTask.m_taskID = ++m_taskIndex;
        newTask.m_type = TaskType::ViewDir;
        newTask.m_des = desPath;

        m_tasks[newTask.m_taskID] = newTask;
        LOG_INFO("Add Task: [%lu] Type: [View Dir] Des: [%s]", newTask.m_taskID, newTask.m_des.c_str());


        //再创建Pull_Dir任务
        ST_TaskInfo newTask1;
        newTask1.m_taskID = ++m_taskIndex;
        newTask1.m_type = TaskType::Pull_Dir;
        newTask1.m_src = srcDir;
        newTask1.m_des = desPath;
        m_tasks[newTask1.m_taskID] = newTask1;
        LOG_INFO("Add Task: [%lu] Type: [Pull_Dir] Des: [%s]", newTask1.m_taskID, newTask1.m_des.c_str());
    }
    else if (srcDir.back() == '/') //目标主机下的文件，先检查本地是否有该文件
    {
        ST_TaskInfo newTask;
        newTask.m_taskID = ++m_taskIndex;
        newTask.m_type = TaskType::Pull_File;
        newTask.m_src = srcDir + FileHelper::BaseName(desPath);
        newTask.m_des = desPath;
        m_tasks[newTask.m_taskID] = newTask;
        LOG_INFO("Add Task: [%lu] Type: [Pull_File] Src: [%s] Des: [%s]", newTask.m_taskID, newTask.m_src.c_str(),
                 newTask.m_des.c_str());
    }
    else
    {
        ST_TaskInfo newTask;
        newTask.m_taskID = ++m_taskIndex;
        newTask.m_type = TaskType::Pull_File;
        newTask.m_src = srcDir;
        newTask.m_des = desPath;
        m_tasks[newTask.m_taskID] = newTask;
        LOG_INFO("Add Task: [%lu] Type: [Pull_File] Src: [%s] Des: [%s]", newTask.m_taskID, newTask.m_src.c_str(),
                 newTask.m_des.c_str());
    }
}

void NetMod::launchPullTask(uint32_t taskID)
{
    if (m_tasks[taskID].m_type == TaskType::Pull_Dir)
    {
        //等待ViewDir进行处理
        return;
    }

    LogCheckConditionVoid(m_tasks[taskID].m_type == TaskType::Pull_File, "Launch Task[%lu] Failed!", taskID);


    m_tasks[taskID].m_generatorPtr = Generator::NewGenerator(m_tasks[taskID].m_src);

    flatbuffers::FlatBufferBuilder builder;

    if (m_tasks[taskID].m_generatorPtr) //文件存在
    {
        std::vector<flatbuffers::Offset<ChunkInfo> > vector1;

        auto &DigestVec = m_tasks[taskID].m_generatorPtr->GetChunkDigestVec();
        for (const auto &item : DigestVec)
        {
            vector1.push_back(Protocol::CreateChunkInfo(builder, item.offset, item.length, item.checksum,
                                                        builder.CreateString(item.md5)));
        }

        auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(m_tasks[taskID].m_src),
                                              builder.CreateString(m_tasks[taskID].m_des),
                                              m_tasks[taskID].m_generatorPtr->GetSplit(),
                                              builder.CreateVector(vector1));
        builder.Finish(fbb);
        this->SendToServer(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());
        return;
    }

    //文件不存在,分块大小为0
    auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(m_tasks[taskID].m_src),
                                          builder.CreateString(m_tasks[taskID].m_des),
                                          0);
    builder.Finish(fbb);
    this->SendToServer(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());
}

void NetMod::onRecvRebuildInfo(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Info!");

    auto pRebuildInfo = flatbuffers::GetRoot<RebuildInfo>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildInfo->Verify(V);
    LogCheckConditionVoid(ok, "Verify Failed!");

    m_tasks[taskID].m_rebuild_size = pRebuildInfo->Size();
}

void NetMod::onRecvRebuildChunk(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Chunk!");

    auto pRebuildChunk = flatbuffers::GetRoot<RebuildChunk>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildChunk->Verify(V);
    LogCheckConditionVoid(ok, "Verify Failed!");


    if (m_tasks[taskID].m_pf == nullptr)
    {
        m_tasks[taskID].m_pf = FileHelper::OpenFile(m_tasks[taskID].m_src + ".tmp", "w"); //读取客户端名称
        LogCheckConditionVoid(m_tasks[taskID].m_pf, "file Ptr is <null>");
    }

    if (pRebuildChunk->IsMd5())
    {
        LogCheckConditionVoid(m_tasks[taskID].m_generatorPtr, "generator Ptr is <null>");
        auto pData = m_tasks[taskID].m_generatorPtr->GetChunkDataByMd5(pRebuildChunk->Data()->str());
        LogCheckConditionVoid(pData.length() == pRebuildChunk->Length(), "Err!");
        auto count = m_tasks[taskID].m_pf->WriteBytes(pData.data(), pData.length(), pRebuildChunk->Offset(), false);
        if (count != pData.length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     m_tasks[taskID].m_src.c_str(), count, pData.length());
        }
        m_tasks[taskID].m_processLen += pData.length();
        LOG_INFO("Task[%lu] File[%s]: Reconstruct(md5) block[%lld +=> %ld] success!", taskID,
                 m_tasks[taskID].m_src.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }
    else
    {
        auto count = m_tasks[taskID].m_pf->WriteBytes(pRebuildChunk->Data()->data(), pRebuildChunk->Length(), pRebuildChunk->Offset(),
                                                       false);
        if (count != pRebuildChunk->Length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     m_tasks[taskID].m_src.c_str(), count, pRebuildChunk->Length());
        }
        m_tasks[taskID].m_processLen += pRebuildChunk->Length();
        LOG_INFO("Task[%lu] File[%s]: Reconstruct(data) block[%lld +=> %ld] success!", taskID,
                 m_tasks[taskID].m_src.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }

    if (m_tasks[taskID].m_processLen == m_tasks[taskID].m_rebuild_size)
    {
        LOG_INFO("Task[%lu] File[%s]: Reconstruct Finished!", taskID, m_tasks[taskID].m_src.c_str());

        m_tasks[taskID].m_pf = nullptr; //关闭文件读写指针，使缓冲区写入到文件完成
        //TODO: 重建完成，重命名替换

        m_tasks[taskID].m_type = TaskType::Finished;

        //发送成功的回包
        flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_SUCCESS);
        builder.Finish(fbb);
        this->SendToServer(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());
        return;
    }

}
