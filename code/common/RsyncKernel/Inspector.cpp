//
// Created by carrot on 18-3-27.
//

#include "cm_define.h"
#include "Inspector.h"
#include "RollingChecksum.h"
#include "MD5.h"

void Inspector::AddInfos(const ST_BlockInfo &info)
{
    if (info.filename != m_filename)
    {
        return;
    }
    m_checksums.insert(info.checksum);
    m_checksum2md5[info.checksum].push_back(info.md5);
    m_md52infos[info.md5] = info;
}

void Inspector::StartGetBlocks(INSPECTOR_CALLBACK callback)
{
    //TODO: 根据本地文件流，生成新建块流
    //callback(1, ST_BlockInfo());
    m_fileptr = FileHelper::CreateFilePtr(m_filename, "r");
    LogCheckConditionVoid(m_fileptr != nullptr, "m_fileptr is null! filename:[%s]", m_filename.c_str());
    char buff[m_blocksize];
    auto count = m_fileptr->ReadBytes(buff, m_blocksize); //首次读取一整块的数据
    if (count == 0)
    {
        return;
    }

    m_start = 0;
    m_end = count;

    m_checksum = RollingChecksum::adler32_checksum(buff, count);
    m_buffer = string().assign(buff, 0, count);

    while (true)
    {
        ST_BlockInfo info;
        if (checkMatched(info)) //找到块匹配
        {
            if (m_start > 0)
            {
                //前面存在未匹配的部分
                //TODO: 1.打包前面的部分并发送，2打包匹配部分并发送
                ST_BlockInfo info1;
                info1.filename = m_filename;
                info1.order = m_order;
                info1.offset = m_offset;
                info1.length = m_start;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_start);

                callback(m_taskID, info1);

                m_buffer.erase(0, m_start);
                m_order++;
                m_offset += m_end - m_start;
                m_end -= m_start;
                m_start = 0;

            }
            //TODO: 只打包匹配部分，并将缓冲区前移，m_start, m_end修改位置

            callback(m_taskID, info);
            m_order++;
            m_offset += m_end - m_start;
            m_buffer.erase(m_start, m_end - m_start);
            m_end = m_start;
        }
        else //未发现匹配项
        {
            auto count = m_fileptr->ReadBytes(buff, 1);
            if (count == 0)
            {
                //文件读取结束
                //TODO：打包发送所有未发送的数据

                ST_BlockInfo info1;
                info1.filename = m_filename;
                info1.order = m_order;
                info1.offset = m_offset;
                info1.length = m_start;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_end);

                callback(m_taskID, info1);
                m_buffer.erase(0, m_end);
                m_start = m_end = 0;
                return;
            }

            LogCheckConditionVoid(count == 1, "count != 1!");

            m_buffer.append(buff, 0, 1);
            char c1 = m_buffer.at(m_start);
            char c2 = m_buffer.at(m_end);
            m_start += 1;
            m_end += 1;
            m_checksum = RollingChecksum::adler32_rolling_checksum(m_checksum, m_end - m_start, c1, c2); //do next loop

            if(m_start >= m_blocksize)
            {
                ST_BlockInfo info1;
                info1.filename = m_filename;
                info1.order = m_order;
                info1.offset = m_offset;
                info1.length = m_blocksize;
                info1.md5 = "-";
                info1.data.assign(m_buffer, 0, m_blocksize);

                callback(m_taskID, info1);

                m_buffer.erase(0, m_blocksize);
                m_order++;
                m_offset += m_blocksize;
                m_end -= m_blocksize;
                m_start -= m_blocksize;
            }
        }
    }
}

bool Inspector::checkMatched(ST_BlockInfo &ret)
{
    auto it = m_checksums.find(m_checksum);
    if (it != m_checksums.end()) //一级不匹配
    {
        for (const auto &item : m_checksum2md5[m_checksum])
        {
            if (item == md5(string(m_buffer, m_start, m_end - m_start)))
            {
                ret = m_md52infos[item];
                LOG_DEBUG("Match! MD5[%s]", item.c_str());
                return true;
            }
        }
    }
    return false;
}

