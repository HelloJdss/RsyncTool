#include <unistd.h>
#include <pthread.h>
#include <csignal>
#include "LogHelper.h"
#include "MainMod.h"

extern char *optarg;
extern int optind, opterr, optopt;

using namespace RsyncServer;

ST_command MainMod::m_cmds[] = {
        {'h', "Show all commands and descriptions.",                                              MainMod::cmd_h},
        {'D', "Open debug model. Every log will print to console.",                               MainMod::cmd_D},
        {'d', "Run in daemon.",                                                                   MainMod::cmd_d},
        {'p', "ReSet Listen Ip Port.",                                                            nullptr},
        {'L', "Change Log Level: 0: DEFAULT 1: FATAL 2: ERROR 3: WARN 4: INFO 5: DEBUG 6: TRACE", nullptr},
};

string MainMod::m_log_path = LOG_FILE_PATH;
uint16_t MainMod::m_port = 48888;
LOG_LEVEL MainMod::m_log_lv = LOG_LEVEL::INFO; //服务端默认只输出INFO

void MainMod::Init(int argc, char *argv[], std::string appName)
{
    g_LogHelper->Init(m_log_lv, appName, m_log_path);
    int opt;
    while ((opt = getopt(argc, argv, "-hDL:dp:")) != -1)
    {
        switch (opt)
        {
            case 'h':
            case 'D':
            case 'd':
                for (int i = 0; i < NR_CMDS; ++i)
                {
                    if (m_cmds[i].arg == opt)
                    {
                        if (m_cmds[i].pFunc)
                        {
                            m_cmds[i].pFunc();
                        }
                        break;
                    }
                }
                break;
            case 'p':
                m_port = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'L':
                m_log_lv = (LOG_LEVEL) std::stoi(optarg);
                break;
            case '?':
                printf("Run \"%s -h\" for more information!\n", argv[0]);
                break;
        }
    }
    g_LogHelper->Init(m_log_lv, appName, m_log_path);
    LOG_INFO("%s inited!", appName.c_str());
    LOG_FATAL("FATAL");
    LOG_ERROR("ERROR");
    LOG_WARN("WARN!");
    LOG_DEBUG("DEBUG");
    LOG_INFO("INFO!");
    LOG_TRACE("TRACE");
}

int MainMod::Run()
{
    LOG_INFO("Run");
    //CreateThreadDetached(runNetMod, nullptr);

    struct sigaction acct;

    acct.sa_handler = MainMod::onRecvSignal;
    sigemptyset(&acct.sa_mask);
    acct.sa_flags = 0;

    sigaction(SIGINT, &acct, nullptr);

    runNetMod(&m_port);
    //getchar();
}

void MainMod::cmd_h()
{
    for (int i = 0; i < NR_CMDS; i++)
    {
        printf("-%c:\t%s\n", m_cmds[i].arg, m_cmds[i].description.c_str());
    }
}

void MainMod::cmd_D()
{
    g_LogHelper->SetDebugModel();
}

void *MainMod::runNetMod(void *port)
{
    g_NetMod->Stop();
    if (port)
    {
        g_NetMod->Init(*(uint16_t *) port);
    }
    else
    {
        g_NetMod->Init();
    }
    g_NetMod->Run();
}

void MainMod::onRecvSignal(int sig)
{
    if (sig == SIGINT)
    {
        LOG_FATAL("Receive Sig: SIGINT, Process will exit now...");
        g_NetMod->Stop();
        exit(0);
    }
}

void MainMod::cmd_d() //以守护进程执行
{
    if (FileHelper::OpenFile("/tmp/rsyncServer/log/init", "w") == nullptr)
    {
        printf("No access to '/tmp/rsyncServer/log/', use 'sudo' try again?\n");
        exit(0);
    }
    m_log_path = "/tmp/rsyncServer/log/";
    daemon(0, 0);
    //setsid();
}
