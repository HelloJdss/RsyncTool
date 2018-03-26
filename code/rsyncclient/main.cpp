//
// Created by carrot on 18-3-15.
//

#include "MainMod.h"


using namespace RsyncClient;

int main(int argc, char *argv[])
{
    MainMod::Init(argc, argv, "rsyncclient");

    return MainMod::Run();
}