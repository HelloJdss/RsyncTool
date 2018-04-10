//
// Created by carrot on 18-3-15.
//

#include <common/FileUtil/FileHelper.h>
#include "MainMod.h"


using namespace RsyncClient;

int main(int argc, char *argv[])
{
    if(MainMod::Init(argc, argv, "rsyncclient"))
    {
        return MainMod::Run();
    }
    return 0;
}