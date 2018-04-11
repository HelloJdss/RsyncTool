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

        //正向同步：push
        SYNC_FILE,     //准备同步的文件列表

        FILE_DIGEST,            //文件摘要信息，一个文件一个

        REBUILD_INFO,           //文件重建信息

        REBUILD_CHUNK,          //文件重建块

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
                    return "VIEW_DIR_REQ";
                case VIEW_DIR_ACK:
                    return "VIEW_DIR_ACK";
                case SYNC_FILE:
                    return "SYNC_FILE";
                case REBUILD_INFO:
                    return "REBUILD_INFO";
                case REBUILD_CHUNK:
                    return "REBUILD_CHUNK";
                case REVERSE_SYNC_REQ:
                    return "REVERSE_SYNC_REQ";
                case REVERSE_SYNC_ACK:
                    return "REVERSE_SYNC_ACK";
                case ERROR_CODE:
                    return "ERROR_CODE";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif //RSYNCTOOL_PROTOCOL_DEFINE_H
