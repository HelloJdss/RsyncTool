//
// Created by carrot on 18-3-19.
//

#include <unistd.h>


#include "FileHelper.h"
#include "MainMod.h"
#include "NetMod.h"

#include <getopt.h>


using namespace RsyncClient;


IMPLEMENT_SINGLETON(Configuration)


extern char *optarg;
extern int optind, opterr, optopt;

ST_command MainMod::m_cmds[] = {
        {'h', "--help(-h)",  "显示所有支持的命令及其描述"},
        {'D', "--debug(-D)", "开启调试模式，所有的调试信息将会打印至控制台"},
        {'L', "--log(-L)",   "更改不同的调试日志等级: 0: DEFAULT 1: FATAL 2: ERROR 3: WARN 4: INFO 5: DEBUG 6: TRACE"},
        {'p', "--push(-p)",  "执行从客户机（本机）目录或文件到服务器（远程）目录的同步任务（上传），命令格式： --push local_dir(file) des_dir@des_ip:port; 例: --push ./a.txt /home/user/@127.0.0.1:48888"},
        {'P', "--pull(-P)",  "执行从服务器（远程）目录或文件到客户机（本地）目录的同步任务（下载），命令格式： --pull local_dir des_dir(file)@des_ip:port; 例: --pull ./ /home/user/a.txt@127.0.0.1:48888"},
        {'f', "--file(-f)",  "执行文件中的同步任务"},
        {'v', "--view(-v)",  "获取文件名列表，命令格式： -v des_dir@des_ip:port; 例： -v /home/user/@127.0.0.1"},
};

bool MainMod::Init(int argc, char *argv[], std::string appName)
{
    int opt;

    struct option longopts[] = {
            {"help",  0, nullptr, 'h'},
            {"debug", 0, nullptr, 'D'},
            {"log",   1, nullptr, 'L'},
            {"push",  1, nullptr, 'p'},
            {"pull",  1, nullptr, 'P'},
            {"file",  1, nullptr, 'f'},
            {"view",  1, nullptr, 'v'},
            {nullptr, 0, nullptr, 0},
    };

    LOG_LEVEL logLevel = g_Configuration->m_log_lv;
    g_LogHelper->Init(logLevel, appName, g_Configuration->m_log_file);

    //加载配置文件
    g_Configuration->LoadXml();

    bool ret = true;
    while ((opt = getopt_long(argc, argv, "-hDL:p:P:f:v:", longopts, nullptr)) != -1)
    {
        switch (opt)
        {
            case 'h':
                for (auto &m_cmd : m_cmds)
                {
                    printf("%s:\t%s\n", m_cmd.arg.c_str(), m_cmd.description.c_str());
                }
                break;
            case 'D':
                g_LogHelper->SetDebugModel();
                break;
            case 'L':
                if (optarg)
                {
                    string arg(optarg);
                    LOG_LEVEL lv = static_cast<LOG_LEVEL>(std::stoi(arg));
                    if (lv >= LOG_LEVEL::MIN && lv <= LOG_LEVEL::MAX)
                    {
                        logLevel = lv;
                    }
                }
                break;
            case 'p':
                if (optind >= argc)
                {
                    printf("参数数量错误，请确认是否按照格式输入！\n");
                    ret = false;
                }
                else
                {
                    if (!cmd_push(string(optarg), string(argv[optind])))
                    {
                        ret = false;
                    }
                }
                break;
            case 'P':
                if (optind >= argc)
                {
                    printf("参数数量错误，请确认是否按照格式输入！\n");
                    ret = false;
                }
                else
                {
                    if (!cmd_pull(string(optarg), string(argv[optind])))
                    {
                        ret = false;
                    }
                }
                break;
            case 'v':
                //TODO: 实现获取远程目录下的文件列表
                ret = cmd_v(string(optarg));
                break;
            case '?':
                //printf("Run \"%s -h\" for more information!\n", argv[0]);
                printf("执行 \"%s -h或--help\" 以获取更多详细信息！\n", argv[0]);
                break;
            default:
                break;
        }
    }
    g_LogHelper->Init(logLevel, appName, g_Configuration->m_log_file);
    LOG_FATAL("FATAL");
    LOG_ERROR("ERROR");
    LOG_WARN("WARN!");
    LOG_DEBUG("DEBUG");
    LOG_INFO("INFO!");
    LOG_TRACE("TRACE");

    LOG_INFO("%s Init:[%s]!", appName.c_str(), ret ? "Success" : "Failed");
    return ret;
}

int MainMod::Run()
{
    LOG_INFO("Run"); //任务完成之前，不断阻塞等待接受消息
    if (g_NetMod->Init())
    {
        g_NetMod->Run();
    }
    return 0;
}


bool MainMod::cmd_push(string src, string des) //local_dir(file) des_dir@des_ip:port;
{
    if(des.front() != '/')
    {
        printf("目标目录格式有误!远程端请使用绝对路径");
        return false;
    }

    auto pos = des.find_last_of('@');
    if (pos != string::npos)
    {
        string desPath = des.substr(0, pos);
        string ipPort = des.substr(pos + 1);

        string desDir;
        if (!FileHelper::SplitDirAndFile(desPath, &desDir))
        {
            printf("目标目录格式有误!");
            return false;
        }

        pos = ipPort.find_last_of(':');
        if (pos != string::npos)
        {
            string ip = ipPort.substr(0, pos);
            uint16_t port = static_cast<uint16_t>(std::stoi(ipPort.substr(pos + 1)));
            printf("set src path: %s des dir: %s, ip: %s port: %u\n", src.c_str(), desDir.c_str(), ip.c_str(), port);
            g_NetMod->AddTask(TaskType::Push, &src, &desDir, ip, port);
            return true;
        }
    }
    printf("参数格式错误，请确认是否按照格式输入！\n");
    return false;
}

bool MainMod::cmd_pull(string src, string des) //local_dir des_dir(file)@des_ip:port;
{
    if(des.front() != '/')
    {
        printf("目标目录格式有误!远程端请使用绝对路径");
        return false;
    }

    string srcDir;
    if (!FileHelper::SplitDirAndFile(src, &srcDir))
    {
        printf("源目录格式有误!");
        return false;
    }

    auto pos = des.find_last_of('@');
    if (pos != string::npos)
    {
        string desPath = des.substr(0, pos);
        string ipPort = des.substr(pos + 1);

        pos = ipPort.find_last_of(':');
        if (pos != string::npos)
        {
            string ip = ipPort.substr(0, pos);
            uint16_t port = static_cast<uint16_t>(std::stoi(ipPort.substr(pos + 1)));
            printf("src Dir: %s des path: %s, ip: %s port: %u\n", srcDir.c_str(), desPath.c_str(), ip.c_str(), port);
            g_NetMod->AddTask(TaskType::Pull_File, &srcDir, &desPath, ip, port);
            return true;
        }
    }

    printf("参数格式错误，请确认是否按照格式输入！\n");
    return false;
}

bool MainMod::cmd_v(string des) //des_dir(file)@des_ip:port
{
    auto pos = des.find_last_of('@');
    if (pos != string::npos)
    {
        string desPath = des.substr(0, pos);
        string ipPort = des.substr(pos + 1);

        string desDir;
        if (!FileHelper::SplitDirAndFile(desPath, &desDir))
        {
            printf("源目录格式有误!");
            return false;
        }

        pos = ipPort.find_last_of(':');
        if (pos != string::npos)
        {
            string ip = ipPort.substr(0, pos);
            uint16_t port = static_cast<uint16_t>(std::stoi(ipPort.substr(pos + 1)));
            printf("set dir: %s, ip: %s port: %u\n", desDir.c_str(), ip.c_str(), port);
            g_NetMod->AddTask(TaskType::ViewDir, nullptr, &desDir, ip, port);
            return true;
        }

    }
    return false;
}

int Configuration::LoadXml(char const *xmlPath)
{
    XMLDocument doc;
    if (doc.LoadFile(xmlPath) != 0)
    {
        RT_ERROR("Load xml file failed! Configuration will set default! Err: %s", doc.ErrorStr());
        return SaveAsXml(xmlPath);
    }

    XMLElement *root = doc.RootElement();

    LogCheckCondition(root, -1, "Explain Xml Failed!");

    XMLElement *log = root->FirstChildElement("log");
    LogCheckCondition(log, -1, "Explain Xml Failed!");

    XMLElement *log_lv = log->FirstChildElement("level");
    LogCheckCondition(log_lv, -1, "Explain Xml Failed!");

    m_log_lv = static_cast<LOG_LEVEL>(std::stoi(string(log_lv->GetText())));

    XMLElement *log_path = log->FirstChildElement("path");
    LogCheckCondition(log_path, -1, "Explain Xml Failed!");

    m_log_file = string(log_path->GetText());

    XMLElement *view = root->FirstChildElement("view");
    LogCheckCondition(view, -1, "Explain Xml Failed!");

    XMLElement *view_path = view->FirstChildElement("output");
    LogCheckCondition(view_path, -1, "Explain Xml Failed!");

    m_view_output = string(view_path->GetText());

    XMLElement *view_path_size = view->FirstChildElement("max_size");
    LogCheckCondition(view_path, -1, "Explain Xml Failed!");

    m_view_output_max_size = std::stoi(string(view_path_size->GetText()));

    return 0;
}

int Configuration::SaveAsXml(char const *xmlPath)
{
    auto fp = FileHelper::OpenFile(xmlPath, "w"); //tiny xml2必须先手动创建不存在的文件

    fp = nullptr;

    XMLDocument doc;
    if (doc.LoadFile(xmlPath) != 0)
    {
        RT_ERROR("Load xml file failed! Err: %s", doc.ErrorStr());
    }
    //添加申明可以使用如下两行
    XMLDeclaration *declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);

    XMLElement *root = doc.NewElement("Client");

    XMLElement *log = doc.NewElement("log");

    XMLElement *log_lv = doc.NewElement("level");
    log_lv->InsertEndChild(doc.NewText(std::to_string(m_log_lv).c_str()));
    log->InsertEndChild(log_lv);

    XMLElement *log_path = doc.NewElement("path");
    log_path->InsertEndChild(doc.NewText(m_log_file.c_str()));
    log->InsertEndChild(log_path);

    root->InsertEndChild(log);


    XMLElement *view = doc.NewElement("view");

    XMLElement *view_output = doc.NewElement("output");
    view_output->InsertEndChild(doc.NewText(m_view_output.c_str()));
    view->InsertEndChild(view_output);

    XMLElement *view_output_size = doc.NewElement("max_size");
    view_output_size->InsertEndChild(doc.NewText(std::to_string(m_view_output_max_size).c_str()));
    view->InsertEndChild(view_output_size);

    root->InsertEndChild(view);

    doc.InsertEndChild(root);

    return doc.SaveFile(xmlPath);
}
