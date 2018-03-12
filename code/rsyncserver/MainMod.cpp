#include "MainMod.h"
#include <stdio.h>
#include <string.h>

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
		printf("%s %s", commands[i].arg, commands[i].description);
	}
}
