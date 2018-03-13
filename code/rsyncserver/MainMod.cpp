#include <stdio.h>
#include <string.h>
#include "MainMod.h"
#include "rlog.h"
#include "cm_define.h"
using namespace RingLog;

IMPLEMENT_SINGLETON(MainMod)

int MainMod::m_argc = 0;
char** MainMod::m_argv = nullptr;

const ST_command MainMod::m_commands[] =
{
	{ "-h", "show all command and descriptions.\n", _command_h },
	{ "-d", "debug mode, every log will print to console.\n", _command_d },
};

int MainMod::Init(int argc, char ** argv)
{
	m_argc = argc;
	m_argv = argv;
	_Start_Explain(0);
	return 0;
}

void MainMod::_Start_Explain(int order)
{
	if (order >= 0 && order <= m_argc - 1)
	{
		for (int i = 0; i < NR_COMMANDS; i++)
		{
			if (strcmp(m_argv[order], m_commands[i].arg.c_str()) == 0)
			{
				m_commands[i].pFunc();
				break;
			}
		}
		_Start_Explain(order + 1);
	}
	else
	{
		RT_PANIC("order out of range!");
	}
	
}

void MainMod::_command_h()
{
	for (int i = 0; i < NR_COMMANDS; i++)
	{
		printf("%s\t\t%s", m_commands[i].arg.c_str(), m_commands[i].description.c_str());
	}
}

void MainMod::_command_d()
{
	LOG_INIT("log", "RsyncTool", LOG_LEVEL::MAX, true);
	LOG_INIT("log1", "RsyncTool", LOG_LEVEL::MAX, false);
	LOG_DEBUG("debug model!");
}