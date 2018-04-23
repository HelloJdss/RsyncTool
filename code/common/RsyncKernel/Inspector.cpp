//
// Created by carrot on 18-3-27.
//

#include "cm_define.h"
#include "Inspector.h"
#include "RollingChecksum.h"
#include "MD5.h"

void Inspector::AddDigestInfo(const ST_BlockInformation &info)
{
    m_checksums.insert(info.checksum);
    m_checksum2md5[info.checksum].push_back(info.md5);
    m_md52infos[info.md5] = info;
}

bool Inspector::checkMatched(ST_BlockInformation &ret)
{
    auto it = m_checksums.find(m_checksum);
    if (it != m_checksums.end()) //一级不匹配
    {
        for (const auto &item : m_checksum2md5[m_checksum])
        {
            if (item == md5(string(m_buffer, m_start, m_end - m_start)))
            {
                ret = m_md52infos[item];
                //LOG_DEBUG("Match! Offset: [%llu] Length: [%llu] MD5: [%s]", ret.offset, ret.length, item.c_str());
                return true;
            }
        }
    }
    return false;
}

void Inspector::BeginInspect(INSPECTOR_CALLBACK callback)
{
    //根据本地文件流，生成新建块流
    if(m_fp == nullptr)
    {
        m_fp = FileHelper::OpenFile(m_filename, "rb");
        LogCheckConditionVoid(m_fp != nullptr, "m_fp is null! filename:[%s]", m_filename.c_str());
    }

    LOG_TRACE("Checking File[%s]...", m_fp->Path().c_str());

    m_limit = m_fp->Size();  //读取此刻的大小

    char buff[m_split];

    auto count = m_fp->ReadBytes(buff, shouldStop() ? 0 : m_split); //首次读取一整块的数据
    m_processedSize += count;

    if (count == 0)
    { //空文件
        ST_BlockInformation info;
        info.offset = 0;
        info.length = 0;
        info.md5 = "-";
        info.data.clear();

        callback(m_taskID, info);
        return;
    }

    m_start = 0;
    m_end = count;

    m_checksum = RollingChecksum::adler32_checksum(buff, count);

    m_buffer.clear();
    for (int i = 0; i < count; ++i)
    {
       m_buffer.push_back(buff[i]);
    }


    while (true)
    {
        ST_BlockInformation info;
        bzero(buff, m_split);
        //LOG_DEBUG("checksum[%lu]", m_checksum);
        if (checkMatched(info)) //找到块匹配
        {
            if (m_start > 0)
            {
                //前面存在未匹配的部分
                /*
                 * 1.打包前面的部分并发送，2打包匹配部分并发送
                 */
                ST_BlockInformation info1;
                info1.offset = m_offset;
                info1.length = m_start;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_start);

                callback(m_taskID, info1);

                m_buffer.erase(0, m_start);
//                m_order++;
                m_offset += m_start;
                m_end -= m_start;
                m_start = 0;

            }
            //只打包匹配部分，并将缓冲区前移，m_start, m_end修改位置

            info.offset = m_offset;

            callback(m_taskID, info); //必须更新偏移量，因为info的结构体信息是旧的

//            m_order++;
            m_offset += m_end - m_start;
            m_buffer.erase(m_start, m_end - m_start);
            m_end = m_start;

            count = m_fp->ReadBytes(buff, shouldStop() ? 0 : m_split); //读取一整块的数据
            m_processedSize += count;

            if (count == 0)
            {
                return;
            }

            m_start = 0;
            m_end = count;

            m_checksum = RollingChecksum::adler32_checksum(buff, count);

            m_buffer.clear();
            for (int i = 0; i < count; ++i)
            {
                m_buffer.push_back(buff[i]);
            }
        }
        else //未发现匹配项
        {
            count = m_fp->ReadBytes(buff, shouldStop() ? 0 : 1);
            m_processedSize += count;

            if (count == 0)
            {
                //文件读取结束
                //打包发送所有未发送的数据

                ST_BlockInformation info1;
                info1.offset = m_offset;
                info1.length = m_end;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_end);

                callback(m_taskID, info1);
                m_buffer.erase(0, m_end);
                m_start = m_end = 0;
                return;
            }

            LogCheckConditionVoid(count == 1, "count != 1!");
            //LOG_TRACE("m_buff.size() = [%llu], m_start [%d] m_end [%d]", m_buffer.size(), m_start, m_end);
            m_buffer.push_back(buff[0]);
            //LOG_TRACE("m_buff.size() = [%llu], m_start [%d] m_end [%d]", m_buffer.size(), m_start, m_end);
            char c1 = m_buffer.at(m_start);
            char c2 = m_buffer.at(m_end);
            m_start += 1;
            m_end += 1;
            m_checksum = RollingChecksum::adler32_rolling_checksum(m_checksum, m_end - m_start, c1, c2); //do next loop

            if(m_start >= m_split)
            {
                ST_BlockInformation info1;
                info1.offset = m_offset;
                info1.length = m_split;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_split);

                callback(m_taskID, info1);

                m_buffer.erase(0, m_split);
                m_offset += m_split;
                m_end -= m_split;
                m_start -= m_split;
            }
        }
    }
}

std::shared_ptr<Inspector> Inspector::NewInspector(uint32_t cb_taskID, const string &filename, uint32_t split)
{
    return std::make_shared<Inspector>(cb_taskID, filename, split);
}

std::shared_ptr<Inspector> Inspector::NewInspector(uint32_t cb_taskID, FilePtr fp, uint32_t split)
{
    return std::make_shared<Inspector>(cb_taskID, fp, split);
}

inline bool Inspector::shouldStop()
{
    return m_processedSize >= m_limit;
}

