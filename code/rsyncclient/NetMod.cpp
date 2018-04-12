//
// Created by carrot on 18-3-26.
//

#include <fcntl.h>
#include <unistd.h>
#include "NetMod.h"
#include "MainMod.h"
#include "Inspector.h"

#include "ViewDir_generated.h"
#include "Synchronism_generated.h"
#include "tinyxml2.h"
#include "Cmd_generated.h"


using namespace Protocol;
using namespace RsyncClient;
using namespace tinyxml2;

void NetMod::Run()
{
    //this->CreateNewTask(Opcode::REVERSE_SYNC_REQ, "./test.txt", "./test1.txt");

    m_running = true;

    auto ptr = MsgThreadPtr(new MsgThread); //创建一个线程用于消息的接受和处理
    if (ptr->Start())
    {
        ptr->Detach();
    }

    while (m_running && !m_taskMgr.TaskEnd()) //主线程不断检查任务情况
    {
        auto readyTasks = m_taskMgr.GetReadyTasks(100 - m_taskMgr.GetTaskCount(TaskState::Run));

        for (auto i : readyTasks)   //逐个启动任务
        {
            switch (m_taskMgr.GetTask(i).m_type)
            {
                case TaskType::ViewDir:
                    launchViewDirTask(i);
                    //++m_taskRunningCount;
                    break;
                case TaskType::Push:
                    launchPushTask(i);
                    //++m_taskRunningCount;
                    break;
                case TaskType::Pull_Dir :
                case TaskType::Pull_File :
                    launchPullTask(i);
                    //++m_taskRunningCount;
                    break;
                default:
                    break;
            }
        }
        LOG_TRACE("Task Sum: [%d] Ready: [%d] Runing: [%d] Warn: [%d] Abort: [%d] Finished: [%d]", m_taskMgr.GetTaskCount(),
                  m_taskMgr.GetTaskCount(Ready), m_taskMgr.GetTaskCount(TaskState::Run),
                  m_taskMgr.GetTaskCount(Warn), m_taskMgr.GetTaskCount(Abort),
                  m_taskMgr.GetTaskCount(Finished));
        usleep(250 * 1000);
    }
}

void NetMod::Dispatch()
{
    pthread_mutex_lock(&m_mutex);

    ST_PackageHeader header;
    BytesPtr data;
    while (m_msgHelper.ReadMessage(header, &data))
    {
        LOG_INFO("Recv Meg from[%s]: op[%s], taskID[%lu], dataLength:[%lu]", m_serverSocket->GetEndPoint().c_str(),
                 Reflection::GetEnumKeyName(header.getOpCode()).c_str(), header.getTaskId(), data->Size());
        Protocol::Err err = Err_DO_NOT_REPLY;
        switch (header.getOpCode())
        {
            case Opcode::ERROR_CODE:
                onRecvErrorCode(header.getTaskId(), data);
                break;
            case Opcode::VIEW_DIR_ACK :
                err = onRecvViewDirAck(header.getTaskId(), data);
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
                LOG_WARN("Receive UnKnown Opcode, [%s] will close connection!", m_serverSocket->GetEndPoint().c_str());
                m_serverSocket->Close();
                break;
        }

        if (err != Err_DO_NOT_REPLY)
        {
            this->SendErrToServer(header.getTaskId(), err);
        }
    }

    LOG_TRACE("Recv Speed: %lf KB/s, Send Speed: %lf KB/s", m_serverSocket->GetRecvSpeed() * 1000 / 1024,
              m_serverSocket->GetSendSpeed() * 1000 / 1024);
    pthread_mutex_unlock(&m_mutex);
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

    if (err->Code() == Err_SUCCESS)
    {
        m_taskMgr.Finish(taskID);
    }
    else
    {
        m_taskMgr.Abort(taskID, err->TIP()->str());
    }

    LOG_WARN("Receive Err Code: Task[%lu] : [%s] [%s]", taskID, Protocol::EnumNameErr(err->Code()),
             err->TIP() == nullptr ? "<null>" : err->TIP()->c_str());
}

bool NetMod::Init()
{
    m_serverSocket = NetHelper::CreateTCPSocket(INET, false);
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

void NetMod::SendToServer(Protocol::Opcode op, uint32_t taskID, uint8_t *buf, uint32_t size)
{
    ST_PackageHeader header(op, taskID);
    auto bytes = MsgHelper::PackageData(header,
                                        MsgHelper::CreateBytes(buf, size));
    m_serverSocket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));
}

void NetMod::createViewDirTask(const string &desDir)
{
    TaskInfo newTask(++m_taskIndex, TaskType::ViewDir);
    newTask.m_des = desDir;
    //newTask.Ready();
    m_taskMgr.AddTask(newTask);
    m_taskMgr.Ready(newTask.m_taskID);
    //LOG_INFO("Add Task: [%lu] Type: [View Dir] Des: [%s]", newTask.m_taskID, newTask.m_des.c_str());
}

void NetMod::launchViewDirTask(uint32_t taskID)
{
    LogCheckConditionVoid(m_taskMgr.GetTask(taskID).m_type == TaskType::ViewDir, "Launch Task[%lu] Failed!", taskID);

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateViewDirReq(builder, builder.CreateString(m_taskMgr.GetTask(taskID).m_des));
    builder.Finish(fbb);

    this->SendToServer(Opcode::VIEW_DIR_REQ, taskID, builder.GetBufferPointer(), builder.GetSize());

    //LOG_INFO("Launch Task: [%lu] Type: [View Dir] Des: [%s]", m_taskMgr.GetTask(taskID).m_taskID, m_taskMgr.GetTask(taskID).m_des.c_str());
    m_taskMgr.Launch(taskID);
}

Err NetMod::onRecvViewDirAck(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive View Dir Ack");

    auto pViewDirAck = flatbuffers::GetRoot<ViewDirAck>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pViewDirAck->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    FilePtr fp;
    XMLDocument document;
    XMLElement *root;


    if ((FileHelper::Access(g_Configuration->m_view_output, F_OK) == 0) &&  //文件已经存在，则添加
        ((fp = FileHelper::OpenFile(g_Configuration->m_view_output, "r+")) &&   //必须使用r，可读可写，
         fp->Size() <= g_Configuration->m_view_output_max_size)) //若超过大小，则重置
    {
        if (document.LoadFile(fp->GetPointer()) != 0)
        {
            m_taskMgr.Abort(taskID, document.ErrorStr());
            //--m_taskRunningCount;

            return Err_SUCCESS; //告知服务器任务已完成
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
    des->SetAttribute("Path", m_taskMgr.GetTask(taskID).m_des.c_str());
    des->SetAttribute("Time", utc_timer().utc_fmt);
    root->InsertEndChild(des);



    /*(for (auto it = m_taskMgr.GetTask(taskID).begin(); it != m_tasks.end(); ++it)
    {
        LOG_DEBUG("%d %d %s %s", it->second.m_taskID, it->second.m_type, it->second.m_src.c_str(),
                  it->second.m_des.c_str());
        if (it->second.m_type == TaskType::Pull_Dir && it->second.m_des == m_tasks[taskID].m_des)
        {
            PullDirTaskID = it->second.m_taskID;
        }
    }*/

    int64_t PullDirTaskID = m_taskMgr.Find_First(Ready, Pull_Dir, nullptr, &m_taskMgr.GetTask(taskID).m_des);

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

        if (PullDirTaskID != -1)
        {
            createPullTask(
                    m_taskMgr.GetTask(PullDirTaskID).m_src +
                    item->FilePath()->str().substr(m_taskMgr.GetTask(PullDirTaskID).m_des.size()),
                    item->FilePath()->str());
        }
    }

    fp = nullptr; //关闭已打开的文件指针

    if (document.SaveFile(g_Configuration->m_view_output.c_str()) != 0)
    {
        m_taskMgr.GetTask(taskID).Warn(document.ErrorStr());
        //--m_taskRunningCount;

        return Err_SUCCESS; //告诉服务端任务已成功完成
    }

    m_taskMgr.Finish(taskID);
    //--m_taskRunningCount;

    if (PullDirTaskID != -1)
    {
        m_taskMgr.Finish(PullDirTaskID);
        //--m_taskRunningCount;
    }
    return Err_SUCCESS;
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
            TaskInfo newTask(++m_taskIndex, TaskType::Push);
            newTask.m_src = item;
            newTask.m_des = desDir + item.substr(srcPath.size());
            //newTask.Ready();
            //m_tasks[newTask.m_taskID] = newTask;
            m_taskMgr.AddTask(newTask);
            m_taskMgr.Ready(newTask.m_taskID);
        }
    }
    else
    {
        auto fp = FileHelper::OpenFile(srcPath, "r");
        LogCheckConditionVoid(fp, "Open File Failed!");
        TaskInfo newTask(++m_taskIndex, TaskType::Push);
        newTask.m_src = srcPath;
        newTask.m_des = desDir + fp->BaseName();
        //newTask.Ready();
        //m_tasks[newTask.m_taskID] = newTask;
        m_taskMgr.AddTask(newTask);
        m_taskMgr.Ready(newTask.m_taskID);
    }
}

void NetMod::launchPushTask(uint32_t taskID)
{
    LogCheckConditionVoid(m_taskMgr.GetTask(taskID).m_type == TaskType::Push, "Launch Task[%lu] Failed!", taskID);

    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateSyncFile(builder, builder.CreateString(m_taskMgr.GetTask(taskID).m_src),
                                        builder.CreateString(m_taskMgr.GetTask(taskID).m_des));
    builder.Finish(fbb);

    this->SendToServer(Opcode::SYNC_FILE, taskID, builder.GetBufferPointer(), builder.GetSize());

    m_taskMgr.Launch(taskID);
}

Err NetMod::onRecvFileDigest(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive File Digest");

    auto pFileDigest = flatbuffers::GetRoot<FileDigest>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pFileDigest->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    if (m_taskMgr.GetTask(taskID).m_taskID == taskID) //先行创建过的任务，需要校验
    {
        LogCheckCondition(m_taskMgr.GetTask(taskID).m_des == pFileDigest->DesPath()->str(), Err_TASK_CONFLICT,
                          "Task[%lu] Conflict!", taskID);
        LogCheckCondition(m_taskMgr.GetTask(taskID).m_src == pFileDigest->SrcPath()->str(), Err_TASK_CONFLICT,
                          "Task[%lu] Conflict!", taskID);
    }

    m_taskMgr.GetTask(taskID).m_des = pFileDigest->DesPath()->str();
    m_taskMgr.GetTask(taskID).m_src = pFileDigest->SrcPath()->str();

    auto fp = FileHelper::OpenFile(m_taskMgr.GetTask(taskID).m_src, "rb");
    LogCheckCondition(fp, Err_NO_SUCH_FILE, "fp is <null>");

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

        pInspector->BeginInspect(INSPECTOR_CALLBACK_FUNC(&NetMod::onInspectCallBack, this));
    }

    return Err_DO_NOT_REPLY;
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
        TaskInfo newTask(++m_taskIndex, TaskType::ViewDir);
        newTask.m_des = desPath;
        //newTask.Ready();
        //m_tasks[newTask.m_taskID] = newTask;
        m_taskMgr.AddTask(newTask);
        m_taskMgr.Ready(newTask.m_taskID);

        //再创建Pull_Dir任务
        TaskInfo newTask1(++m_taskIndex, TaskType::Pull_Dir);
        newTask1.m_src = srcDir;
        newTask1.m_des = desPath;
        //newTask1.Ready();
        //m_tasks[newTask1.m_taskID] = newTask1;
        m_taskMgr.AddTask(newTask1);
        m_taskMgr.Ready(newTask1.m_taskID);
    }
    else if (srcDir.back() == '/') //目标主机下的文件，先检查本地是否有该文件
    {
        TaskInfo newTask(++m_taskIndex, TaskType::Pull_File);
        newTask.m_src = srcDir + FileHelper::BaseName(desPath);
        newTask.m_des = desPath;
        //newTask.Ready();
        //m_tasks[newTask.m_taskID] = newTask;
        m_taskMgr.AddTask(newTask);
        m_taskMgr.Ready(newTask.m_taskID);
    }
    else
    {
        TaskInfo newTask(++m_taskIndex, TaskType::Pull_File);
        newTask.m_src = srcDir;
        newTask.m_des = desPath;
        //newTask.Ready();
        //m_tasks[newTask.m_taskID] = newTask;
        m_taskMgr.AddTask(newTask);
        m_taskMgr.Ready(newTask.m_taskID);
    }
}

void NetMod::launchPullTask(uint32_t taskID)
{
    if (m_taskMgr.GetTask(taskID).m_type == TaskType::Pull_Dir)
    {
        //等待ViewDir进行处理
        return;
    }

    LogCheckConditionVoid(m_taskMgr.GetTask(taskID).m_type == TaskType::Pull_File, "Launch Task[%lu] Failed!", taskID);


    m_taskMgr.GetTask(taskID).Launch();

    m_taskMgr.GetTask(taskID).m_generatorPtr = Generator::NewGenerator(m_taskMgr.GetTask(taskID).m_src);

    flatbuffers::FlatBufferBuilder builder;

    if (m_taskMgr.GetTask(taskID).m_generatorPtr) //文件存在
    {
        std::vector<flatbuffers::Offset<ChunkInfo> > vector1;

        auto &DigestVec = m_taskMgr.GetTask(taskID).m_generatorPtr->GetChunkDigestVec();
        for (const auto &item : DigestVec)
        {
            vector1.push_back(Protocol::CreateChunkInfo(builder, item.offset, item.length, item.checksum,
                                                        builder.CreateString(item.md5)));
        }

        auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(m_taskMgr.GetTask(taskID).m_src),
                                              builder.CreateString(m_taskMgr.GetTask(taskID).m_des),
                                              m_taskMgr.GetTask(taskID).m_generatorPtr->GetSplit(),
                                              builder.CreateVector(vector1));
        builder.Finish(fbb);
        this->SendToServer(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());
        return;
    }

    //文件不存在,分块大小为0
    auto fbb = Protocol::CreateFileDigest(builder, builder.CreateString(m_taskMgr.GetTask(taskID).m_src),
                                          builder.CreateString(m_taskMgr.GetTask(taskID).m_des),
                                          0);
    builder.Finish(fbb);
    this->SendToServer(Opcode::FILE_DIGEST, taskID, builder.GetBufferPointer(), builder.GetSize());
}

Err NetMod::onRecvRebuildInfo(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Info!");

    auto pRebuildInfo = flatbuffers::GetRoot<RebuildInfo>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildInfo->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");

    m_taskMgr.GetTask(taskID).m_rebuild_size = pRebuildInfo->Size();
    return Err_DO_NOT_REPLY;
}

Err NetMod::onRecvRebuildChunk(uint32_t taskID, BytesPtr data)
{
    LOG_TRACE("Receive Rebuild Chunk!");

    auto pRebuildChunk = flatbuffers::GetRoot<RebuildChunk>(data->ToChars());

    flatbuffers::Verifier V(reinterpret_cast<uint8_t const *>(data->ToChars()), data->Size());
    auto ok = pRebuildChunk->Verify(V);
    LogCheckCondition(ok, Err_VERIFY_FAILED, "Verify Failed!");


    if (m_taskMgr.GetTask(taskID).m_pf == nullptr)
    {
        m_taskMgr.GetTask(taskID).m_pf = FileHelper::OpenFile(m_taskMgr.GetTask(taskID).m_src + ".tmp", "w"); //读取客户端名称
        LogCheckCondition(m_taskMgr.GetTask(taskID).m_pf, Err_NO_SUCH_FILE, "file Ptr is <null>");
    }

    if (pRebuildChunk->IsMd5())
    {
        LogCheckCondition(m_taskMgr.GetTask(taskID).m_generatorPtr, Err_TASK_INFO_INCOMPLETE,
                          "generator Ptr is <null>");
        auto pData = m_taskMgr.GetTask(taskID).m_generatorPtr->GetChunkDataByMd5(pRebuildChunk->Data()->str());
        LogCheckCondition(pData.length() == pRebuildChunk->Length(), Err_UNKNOWN, "Err!");
        auto count = m_taskMgr.GetTask(taskID).m_pf
                ->WriteBytes(pData.data(), pData.length(), pRebuildChunk->Offset(), false);
        if (count != pData.length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     m_taskMgr.GetTask(taskID).m_src.c_str(), count, pData.length());
        }
        m_taskMgr.GetTask(taskID).m_processLen += pData.length();
        LOG_INFO("Task[%lu] File[%s]: Reconstruct(md5) block[%lld +=> %ld] success!", taskID,
                 m_taskMgr.GetTask(taskID).m_src.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }
    else
    {
        auto count = m_taskMgr.GetTask(taskID).m_pf
                ->WriteBytes(pRebuildChunk->Data()->data(), pRebuildChunk->Length(), pRebuildChunk->Offset(),
                             false);
        if (count != pRebuildChunk->Length())
        {
            LOG_WARN("Task[%lu] File[%s]: Write count[%llu] is not equal to data length[%llu]!", taskID,
                     m_taskMgr.GetTask(taskID).m_src.c_str(), count, pRebuildChunk->Length());
        }
        m_taskMgr.GetTask(taskID).m_processLen += pRebuildChunk->Length();
        LOG_INFO("Task[%lu] File[%s]: Reconstruct(data) block[%lld +=> %ld] success!", taskID,
                 m_taskMgr.GetTask(taskID).m_src.c_str(), pRebuildChunk->Offset(), pRebuildChunk->Length());
    }

    if (m_taskMgr.GetTask(taskID).m_processLen == m_taskMgr.GetTask(taskID).m_rebuild_size)
    {
        LOG_INFO("Task[%lu] File[%s]: Reconstruct Finished!", taskID, m_taskMgr.GetTask(taskID).m_src.c_str());

        m_taskMgr.GetTask(taskID).m_pf = nullptr; //关闭文件读写指针，使缓冲区写入到文件完成
        //重建完成，重命名替换

        FileHelper::Rename(m_taskMgr.GetTask(taskID).m_src + ".tmp", m_taskMgr.GetTask(taskID).m_src);

        m_taskMgr.Finish(taskID);
        //--m_taskRunningCount;

        //发送成功的回包
        /*flatbuffers::FlatBufferBuilder builder;
        auto fbb = Protocol::CreateErrorCode(builder, Err_SUCCESS);
        builder.Finish(fbb);
        this->SendToServer(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());*/
        return Err_SUCCESS;
    }
    return Err_DO_NOT_REPLY;
}

void NetMod::SendErrToServer(uint32_t taskID, Protocol::Err err, string tip)
{
    flatbuffers::FlatBufferBuilder builder;
    auto fbb = Protocol::CreateErrorCode(builder, err, builder.CreateString(tip));
    builder.Finish(fbb);
    this->SendToServer(Opcode::ERROR_CODE, taskID, builder.GetBufferPointer(), builder.GetSize());
}

void MsgThread::Runnable() //不断阻塞接受消息和处理消息
{
    bool m_running = true;
    g_NetMod->m_serverSocket->SetRecvTimeOut(30, 0);  //30秒延迟，若30秒仍收不到服务器的消息则结束
    while (m_running)
    {
        try
        {
            auto count = g_NetMod->m_serverSocket->Receive(
                    g_NetMod->m_msgHelper.GetBuffer() + g_NetMod->m_msgHelper.GetStartIndex(),
                    g_NetMod->m_msgHelper.GetRemainBytes());
            if (count > 0)
            {
                g_NetMod->m_msgHelper.AddCount(count);
                g_NetMod->Dispatch();
            }
            else if (count == 0)
            {
                LOG_WARN("Server Close Connection!");
                m_running = false;
            }
        }
        catch (int err)
        {
            LOG_FATAL("Catch Exception: [%s]", strerror(err));
            m_running = false;
            g_NetMod->m_running = false;
        }
    }
}

void TaskMgr::AddTask(TaskInfo &task)
{
    m_tasks[task.m_taskID] = task;
}

int TaskMgr::GetTaskCount(TaskState state)
{
    pthread_mutex_lock(&m_mutex);

    int ret = 0;

    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it)
    {
        if (it->second.m_stat == state)
        {
            ret++;
        }
    }

    pthread_mutex_unlock(&m_mutex);
    return ret;
}

void TaskMgr::Ready(uint32_t taskID)
{
    m_tasks[taskID].Ready();
}

void TaskMgr::Launch(uint32_t taskID)
{
    m_tasks[taskID].Launch();
}

void TaskMgr::Abort(uint32_t taskID, string err)
{
    m_tasks[taskID].Abort(err);
}

void TaskMgr::Warn(uint32_t taskID, string err)
{
    m_tasks[taskID].Warn(err);
}

void TaskMgr::Finish(uint32_t taskID)
{
    m_tasks[taskID].Finish();
}

vector<uint32_t> TaskMgr::GetReadyTasks(int count)
{
    pthread_mutex_lock(&m_mutex);

    vector<uint32_t> ret;
    int n = 0;

    for (auto it = m_tasks.begin(); it != m_tasks.end() && n < count; ++it)
    {
        if (it->second.m_stat == TaskState::Ready)
        {
            ret.push_back(it->second.m_taskID);
            n++;
        }
    }

    pthread_mutex_unlock(&m_mutex);
    return ret;
}

TaskInfo &TaskMgr::GetTask(uint32_t taskID)
{
    return m_tasks[taskID];
}

bool TaskMgr::TaskEnd()
{
    pthread_mutex_lock(&m_mutex);

    int n = 0;

    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it)
    {
        if (it->second.m_stat >= TaskState::Warn)
        {
            n++;
        }
    }

    pthread_mutex_unlock(&m_mutex);
    return n >= m_tasks.size();
}

int64_t
TaskMgr::Find_First(TaskState state, TaskType type, string const *src, string const *des)
{
    pthread_mutex_lock(&m_mutex);


    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it)
    {
        if (it->second.m_stat == state && it->second.m_type == type)
        {
            bool find = true;
            if (src)
            {
                find = find && (it->second.m_src == *src);
            }
            if (des)
            {
                find = find && (it->second.m_des == *des);
            }
            if (find)
            {
                pthread_mutex_unlock(&m_mutex);
                return it->second.m_taskID;
            }
        }
    }

    pthread_mutex_unlock(&m_mutex);
    return -1;
}

int TaskMgr::GetTaskCount()
{
    return m_tasks.size();
}
