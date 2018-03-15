#include <unistd.h>
#include "LogHelper.h"
#include "MainMod.h"

extern char *optarg;
extern int optind, opterr, optopt;

ST_command MainMod::m_cmds[] = {
        {'h', "Show all commands and descriptions.", MainMod::cmd_h},
        {'d', "Open debug model. Every log will print to console.", MainMod::cmd_d},
};

void MainMod::Init(int argc, char *argv[], std::string appName) {
    g_LogHelper->Init(LOG_LEVEL_DEFALT, appName, LOG_FILE_PATH);
    int opt;
    while ((opt = getopt(argc, argv, "-hd")) != -1) {
        switch (opt) {
            case 'h':
            case 'd':
                for (int i = 0; i < NR_CMDS; ++i) {
                    if (m_cmds[i].arg == opt){
                        m_cmds[i].pFunc();
                        break;
                    }
                }
                break;
            case '?':
                printf("Run \"%s -h\" for more information!\n", argv[0]);
                break;
        }
    }
    LOG_INFO("%s inited!", appName.c_str());
}

int MainMod::Run() {
    LOG_INFO("Run");
    return 0;
}

void MainMod::cmd_h() {
    for(int i = 0; i < NR_CMDS; i++)
    {
        printf("-%c:\t%s\n", m_cmds[i].arg, m_cmds[i].description.c_str());
    }
}

void MainMod::cmd_d() {
    g_LogHelper->SetDebugModel();
    LOG_WARN("warn");
    LOG_DEBUG("debug");
    LOG_FATAL("fatal");
    LOG_INFO("info");
    LOG_TRACE("trace");
    LOG_ERROR("hello world");
}
