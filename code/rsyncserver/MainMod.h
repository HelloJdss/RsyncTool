#pragma once
#include <functional>
class MainMod
{
public:
	int Init(int argc, char** argv);

private:
	void _Explain(char* arg);
private:
	int m_argc;
	char** m_argv;

};

void command_h();
void command_d();

static struct command
{
	char* arg;
	char* description;
	std::function<void(void)> pFunc;
} commands[] =
{
	{ "-h", "show all command and descriptions.\n", command_h},
	{ "-d", "debug mode, every log will print to console.\n", nullptr},
};

#define NR_COMMANDS (sizeof(commands) / sizeof(commands[0]) )