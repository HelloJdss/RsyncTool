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

/**
 * 负责根据拿到的签名信息，对比本地文件并生成重建块,每个文件一个
 * 每重建一块之后，会执行回调
 * 回调参数包括 任务编号，重建块
 */

//typedef std::function<void (uint32_t, const ST_BlockInformation&, const string&, size_t)> INSPECTOR_CALLBACK;
//#define INSPECTOR_CALLBACK_FUNC(_f, _this) std::bind((_f), (_this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

typedef std::function<void (uint32_t, const ST_BlockInformation&)> INSPECTOR_CALLBACK;
#define INSPECTOR_CALLBACK_FUNC(_f, _this) std::bind(_f, _this, std::placeholders::_1, std::placeholders::_2)

class Inspector
{
public:
    static std::shared_ptr<Inspector> NewInspector(uint32_t cb_taskID, const string& filename, uint32_t split);

    static std::shared_ptr<Inspector> NewInspector(uint32_t cb_taskID, FilePtr fp, uint32_t split);

    Inspector(uint32_t taskID, const string& filename, uint32_t split)
    {
        m_taskID = taskID;
        m_filename = filename;
        m_split = split;
    }

    Inspector(uint32_t taskID, FilePtr fp, uint32_t split)
    {
        m_taskID = taskID;
        m_fp = fp;
        m_split = split;
    }

    ~Inspector()
    {
        LOG_TRACE("~Inspector");
    }

    void AddDigestInfo(const ST_BlockInformation &info); //添加块信息，用于执行匹配

    //void StartGetBlocks(INSPECTOR_CALLBACK callback);

    void BeginInspect(INSPECTOR_CALLBACK callback); //开始执行检查

private:
    bool checkMatched(ST_BlockInformation& ret);   //检查是否有匹配块

    RTSet<uint32_t> m_checksums;    //[checksum]
    RTMap<string, ST_BlockInformation> m_md52infos; //md5  ==> BlockInfo
    RTMap<uint32_t, RTVector<string> > m_checksum2md5; //checksum ==> md5
    uint32_t m_split = 0;
    string m_filename;
    uint32_t m_taskID = 0;

    FilePtr m_fp = nullptr;              //path ==> FilePtr
    uint32_t m_checksum = 0;
    string m_buffer;
    int32_t m_start = 0, m_end = 0;
    int64_t m_offset = 0;
};

typedef std::shared_ptr<Inspector> InspectorPtr;


#endif //RSYNCTOOL_INSPECTOR_H
