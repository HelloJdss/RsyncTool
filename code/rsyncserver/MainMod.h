#pragma once
#include "cm_define.h"
#include <functional>
#include <string>
using std::string;

struct ST_command
{
	string arg;
	string description;
	std::function<void(void)> pFunc;
};

class MainMod
{
    DECLARE_SINGLETON(MainMod)
public:
    static int Init(int argc, char** argv);
private:
	static void _Start_Explain(int order);
	static void _command_h();
	static void _command_d();

private:
	static int m_argc;
	static char** m_argv;
	static const ST_command m_commands[];
	#define NR_COMMANDS (sizeof(MainMod::m_commands) / sizeof(MainMod::m_commands[0]))
};
