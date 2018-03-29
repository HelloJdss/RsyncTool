//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_CM_STRUCT_H
#define RSYNCTOOL_CM_STRUCT_H

#include <strings.h>
#include <cstdint>
#include <netinet/in.h>
#include "Protocol/Protocol_define.h"

struct ST_BlockInfo
{
    //std::string     filename;
    //int64_t         order;
    int64_t         offset;
    int32_t         length;
    uint32_t        checksum;
    std::string     md5;
    std::string     data;

    ST_BlockInfo()
    {
        //filename.clear();
        //order = 0;
        offset = 0;
        length = 0;
        checksum = 0;
        md5.clear();
        data.clear();
    }
};

struct ST_PackageHeader
{
    ST_PackageHeader()
    {
        reset();
    }

    ST_PackageHeader(Protocol::Opcode op, uint32_t id)
    {
        opcode = static_cast<Protocol::Opcode>(htonl(op));
        taskID = htonl(id);
    }

    void setOpCode(Protocol::Opcode op)
    { opcode = static_cast<Protocol::Opcode>(htonl(op)); }

    Protocol::Opcode getOpCode()
    { return static_cast<Protocol::Opcode>(ntohl(opcode)); }

    void setTaskId(uint32_t id)
    { taskID = htonl(id); }

    uint32_t getTaskId()
    { return ntohl(taskID); }

    void reset()
    { bzero(this, sizeof(*this)); }

private:
    Protocol::Opcode opcode;
    uint32_t taskID;
};

#endif //RSYNCTOOL_CM_STRUCT_H
