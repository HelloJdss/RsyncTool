//
// Created by carrot on 18-3-15.
//

#include <iostream>
#include "MsgHelper.h"
#include "NetHelper.h"
#include "MainMod.h"
#include "Generator.h"
#include "BlockInfos_generated.h"
#include "Protocol/Protocol_define.h"

using namespace RsyncClient;
using namespace Protocol;

int main(int argc, char *argv[])
{
    MainMod::Init(argc, argv, "rsyncclient");

    Generator generator;
    generator.GenerateBlockInfos("./console.txt");

    auto infos = generator.GetBlockInfos("./console.txt");

    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Protocol::BlockInfo> > blockVec;
    for (auto it : infos)
    {
        blockVec.push_back(Protocol::CreateBlockInfo(builder, builder.CreateString(it.filename), it.splitsize, it.order, it.offset,
                                                        it.length, it.checksum, builder.CreateString(it.md5)));
    }

    auto fbb = Protocol::CreateFileBlockInfos(builder, builder.CreateString("./console.txt"), builder.CreateString("./Console.txt"), builder.CreateVector(blockVec));
    builder.Finish(fbb);



    auto socket = NetHelper::CreateTCPSocket(INET);
    try
    {
        socket->Connect(SocketAddress(INADDR_ANY, 52077));
        ST_PackageHeader header(Opcode::REVERSE_SYNC_REQ, 1);
        auto bytes = MsgHelper::PackageData(header, MsgHelper::CreateBytes(builder.GetBufferPointer(), builder.GetSize()));
        socket->Send(bytes->ToChars(), static_cast<int>(bytes->Size()));
        getchar();
    }
    catch (int err)
    {
        LOG_ERROR("%s", strerror(err));
    }
    return 0;
}