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

        VIEW_DIR_REQ,           //浏览文件夹文件
        VIEW_DIR_ACK,

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
                case VIEW_DIR_REQ:
                    return std::string("VIEW_DIR_REQ");
                case VIEW_DIR_ACK:
                    return std::string("VIEW_DIR_ACK");
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
