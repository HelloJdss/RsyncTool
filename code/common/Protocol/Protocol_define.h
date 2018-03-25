//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_PROTOCOL_DEFINE_H
#define RSYNCTOOL_PROTOCOL_DEFINE_H

namespace Protocol
{

#include <string>

    enum Opcode
    {
        MIN = 0,

        REVERSE_SYNC_REQ,       //反向同步请求
        REVERSE_SYNC_ACK,       //反向同步回复

        ERROR_CODE,             //错误码
        MAX,
    };

    class Reflection
    {

    public:
        static std::string GetEnumKeyName(Opcode opcode)
        {
            switch (opcode)
            {
                case REVERSE_SYNC_REQ:
                    return std::string("REVERSE_SYNC_REQ");
                case REVERSE_SYNC_ACK:
                    return std::string("REVERSE_SYNC_ACK");
                case ERROR_CODE:
                    return std::string("ERROR_CODE");
                default:
                    return std::string("INVALID");
            }
        }
    };
}

#endif //RSYNCTOOL_PROTOCOL_DEFINE_H
