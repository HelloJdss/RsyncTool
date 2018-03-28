//
// Created by carrot on 18-3-27.
//

#ifndef RSYNCTOOL_INSPECTOR_H
#define RSYNCTOOL_INSPECTOR_H

#include <functional>
#include "cm_define.h"
#include "cm_struct.h"
#include "FileHelper.h"
#include "LogHelper.h"


using std::string;
using std::unordered_set;
using std::unordered_map;

/*
 * 负责根据拿到的签名信息，对比本地文件并生成重建块,每个文件一个
 */

typedef std::function<void (uint32_t, const ST_BlockInfo&)> INSPECTOR_CALLBACK;
#define INSPECTOR_CALLBACK_FUNC(_f, _this) std::bind((_f), (_this), std::placeholders::_1, std::placeholders::_2)


class Inspector
{
public:
    Inspector(uint32_t taskID, const string& filename, uint32_t blocksize)
    {
        m_taskID = taskID;
        m_filename = filename;
        m_blocksize = blocksize;
    }

    ~Inspector()
    {
        LOG_TRACE("~Inspector");
    }

    void AddInfos(const ST_BlockInfo &info);

    void StartGetBlocks(INSPECTOR_CALLBACK callback);

private:
    bool checkMatched(ST_BlockInfo& ret);   //检查是否有匹配块

    RTSet<uint32_t> m_checksums;    //[checksum]
    RTMap<string, ST_BlockInfo> m_md52infos; //md5  ==> BlockInfo
    RTMap<uint32_t, RTVector<string> > m_checksum2md5; //checksum ==> md5
    uint32_t m_blocksize = 0;
    string m_filename;
    uint32_t m_taskID = 0;

    FilePtr m_fileptr = nullptr;              //path ==> FilePtr
    uint32_t m_checksum = 0;
    string m_buffer;
    size_t m_start = 0, m_end = 0;
    uint64_t m_order = 0;
    uint64_t m_offset = 0;
};

typedef std::shared_ptr<Inspector> InspectorPtr;


#endif //RSYNCTOOL_INSPECTOR_H
