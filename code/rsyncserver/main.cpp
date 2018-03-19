#include "LogHelper.h"
#include "MainMod.h"

int main(int argc, char *argv[])
{
    MainMod::Init(argc, argv, "rsyncserver");
    return MainMod::Run();
}