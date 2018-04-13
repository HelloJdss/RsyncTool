#include <unistd.h>
#include "LogHelper.h"
#include "MainMod.h"

using namespace RsyncServer;

int main(int argc, char *argv[])
{
    MainMod::Init(argc, argv, "rsyncserver");
    return MainMod::Run();
}