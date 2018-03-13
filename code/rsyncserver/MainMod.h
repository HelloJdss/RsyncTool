#pragma once
#include "cm_define.h"
#include <functional>
#include <string>
using std::string;
class MainMod
{
    DECLARE_SINGLETON(MainMod)
public:
    static int Init(int argc, char** argv);
private:
	static void _Explain(char* arg);
private:
	static int m_argc;
	static char** m_argv;

};

void command_h();
void command_d();

static struct command
{
	string arg;
	string description;
	std::function<void(void)> pFunc;
} commands[] =
{
	{ "-h", "show all command and descriptions.\n", command_h},
	{ "-d", "debug mode, every log will print to console.\n", command_d},
};

#define NR_COMMANDS (sizeof(commands) / sizeof(commands[0]) )