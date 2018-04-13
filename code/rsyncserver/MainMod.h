//
// Created by carrot on 18-3-11.
//

#pragma once

#include <functional>
#include <string>
#include "cm_define.h"
#include "NetMod.h"

namespace RsyncServer
{
    using std::string;

    struct ST_command
    {
        char arg;
        string description;

        void (*pFunc)(void);
    };

    class MainMod
    {
    public:
        static void Init(int argc, char *argv[], std::string appName);

        static int Run();

        static string m_log_path;

        static uint16_t m_port;

        static LOG_LEVEL m_log_lv;


    private:
        static void* runNetMod(void* port);

        static ST_command m_cmds[];

        static void cmd_h();

        static void cmd_D();

        static void cmd_d();

        static void onRecvSignal(int sig);

#define NR_CMDS (sizeof(m_cmds) / sizeof(m_cmds[0]))
    };

}

