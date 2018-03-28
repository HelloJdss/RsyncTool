//
// Created by carrot on 18-3-27.
//

#include <common/cm_define.h>
#include "Inspector.h"
#include "RollingChecksum.h"

void Inspector::AddInfos(const ST_BlockInfo &info)
{
    if(info.filename != m_filename)
    {
        return;
    }
    m_checksums.insert(info.checksum);
    m_checksum2md5[info.checksum].push_back(info.md5);
    m_md52infos[info.md5].push_back(info);
}

bool Inspector::HasNextBlock()
{
    if (m_fileptr == nullptr)
    {
        m_fileptr = FileHelper::CreateFilePtr(m_filename, "r");
    }

    return !m_fileptr->Eof();
}

void Inspector::StartGetBlocks(std::function<void(uint32_t, ST_BlockInfo const &)>* callback)
{

}

RTVector<ST_BlockInfo> Inspector::GetBlocks()
{
    RTVector<ST_BlockInfo> blocks;
    if (!HasNextBlock())
    {
        return blocks;
    }

    size_t count = 0;

    char buff[m_blocksize];

    count = m_fileptr->ReadBytes(buff, m_blocksize);   //读取一整块
    //string.append(buff, count);
    if(m_checksum == 0)
    {
        m_checksum = RollingChecksum::adler32_checksum(buff, count);    //首次校验
    }
    else
    {
        char c1 = m_buffer.front();
        //m_checksum = RollingChecksum::adler32_rolling_checksum(m_checksum, m_blocksize, c1, )
    }

    return blocks;
}

void Inspector::RunThread(void *param)
{

}

void Inspector::onDestroy(void *func_ptr)
{
    delete (std::function<void (uint32_t, const ST_BlockInfo&)>*)func_ptr;
}
