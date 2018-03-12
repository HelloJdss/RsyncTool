#include <stdio.h>
#include <string.h>
#include "MainMod.h"
#include "rlog.h"
using namespace RingLog;
int MainMod::Init(int argc, char ** argv)
{
	this->m_argc = argc;
	this->m_argv = argv;
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
		if (strcmp(arg, commands[i].arg) == 0)
		{
			commands[i].pFunc();
		}
	}
}

void command_h()
{
	for (int i = 0; i < NR_COMMANDS; i++)
	{
		printf("%s\t\t%s", commands[i].arg, commands[i].description);
	}
}

void command_d()
{
	LOG_INIT("log", "RsyncTool", LOG_LEVEL::MAX, true);
	LOG_DEBUG("debug model!");
}
