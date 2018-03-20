//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_MAINMOD_H
#define RSYNCTOOL_MAINMOD_H

#include <string>
#include "cm_define.h"

namespace RsyncClient
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

    private:
        static ST_command m_cmds[];

        static void cmd_h();

        static void cmd_d();

#define NR_CMDS (sizeof(m_cmds) / sizeof(m_cmds[0]))
    };
}

#endif //RSYNCTOOL_MAINMOD_H
