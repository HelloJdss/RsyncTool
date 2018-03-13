#pragma once

#include "cm_define.h"
#include <functional>
#include <string>

using std::string;

struct ST_command {
    char arg;
    string description;
    std::function<void(void)> pFunc;
};

class MainMod {
public:
    static void Init(int argc, char *argv[], std::string appName);

    static int Run();

private:
    static ST_command m_cmds[];

    static void cmd_h();
    static void cmd_d();

#define NR_CMDS (sizeof(m_cmds) / sizeof(m_cmds[0]))
};
