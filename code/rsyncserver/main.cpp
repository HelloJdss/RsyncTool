#include <unistd.h>
#include "LogHelper.h"
#include "MainMod.h"

using namespace RsyncServer;

int main(int argc, char *argv[])
{
    //daemon(0, 0);
    //setsid();
    MainMod::Init(argc, argv, "rsyncserver");
    return MainMod::Run();
}