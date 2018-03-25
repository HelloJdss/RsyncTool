//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_CM_STRUCT_H
#define RSYNCTOOL_CM_STRUCT_H

#include <strings.h>
#include <cstdint>
#include <netinet/in.h>
#include <common/Protocol/Protocol_define.h>

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
