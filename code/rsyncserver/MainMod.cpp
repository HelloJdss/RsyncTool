#include <stdio.h>
#include <string.h>
#include "MainMod.h"
#include "rlog.h"
using namespace RingLog;


IMPLEMENT_SINGLETON(MainMod)

int MainMod::m_argc = 0;
char** MainMod::m_argv = nullptr;

int MainMod::Init(int argc, char ** argv)
{
	m_argc = argc;
	m_argv = argv;
	for (int i = 0; i < m_argc; i++)
	{
		_Explain(argv[i]);
	}
	return 0;
}

void MainMod::_Explain(char * arg)
{
	for (int i = 0; i < NR_COMMANDS; i++)
	{
		if (strcmp(arg, commands[i].arg.c_str()) == 0)
		{
			commands[i].pFunc();
		}
	}
}

void command_h()
{
	for (int i = 0; i < NR_COMMANDS; i++)
	{
		printf("%s\t\t%s", commands[i].arg.c_str(), commands[i].description.c_str());
	}
}

void command_d()
{
	LOG_INIT("log", "RsyncTool", LOG_LEVEL::MAX, true);
	LOG_DEBUG("debug model!");
}
