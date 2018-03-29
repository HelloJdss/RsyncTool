//
// Created by carrot on 18-3-23.
//

#include "LogHelper.h"
#include "Generator.h"
#include "MD5.h"
#include "FileHelper.h"
#include "RollingChecksum.h"

Generator::BlockInfoVec const &Generator::GetBlockInfos(const string &filename)
{
    auto it = m_fileInfos.find(filename);
    if (it != m_fileInfos.end())
    {
        return it->second;
    }
    static BlockInfoVec nullVec;
    return nullVec;
}

bool Generator::GenerateBlockInfos(const string &filename, uint32_t splitsize)
{
    auto fp = FileHelper::CreateFilePtr(filename, "rb");
    LogCheckCondition(fp!= nullptr, false, "open file [%s] failed! errno: %s", filename.c_str(), strerror(errno));


    int64_t n = fp->Size();
    if (n == -1)
    {
        LOG_LastError();
        return false;
    }

    LOG_DEBUG("File[%s] Open Success! Length:[%llu], SplitSize: %lu", basename(filename.c_str()), n, splitsize);

    m_fileInfos[filename].clear();

    char buff[splitsize];

    int64_t i = 0;

    while (i < n)
    {
        ST_BlockInfo info;
        info.filename = filename;
        info.order = i / splitsize;
        info.offset = i;

        size_t count = fp->ReadBytes(buff, splitsize);

        i += count;

        info.length = count;

        info.checksum = RollingChecksum::adler32_checksum(buff, count);

        info.md5 = md5(buff, count);

        info.data.assign(buff, count);

        m_fileInfos[filename].push_back(info);

        m_Md5ToData[info.md5] = m_fileInfos[filename].back().md5;   //这种赋值方式是为了利用string的写时复制特性，避免不必要的拷贝

        LOG_DEBUG("[%3lld\%] Finish Block[%lld]: Offset: %lld Length: %lld CheckSum: %u MD5: %s", i * 100 / n,
                  info.order,info.offset, info.length, info.checksum, info.md5.c_str());
    }

    return true;
}

const string &Generator::GetDataByMd5(const string &md5)
{
    auto it = m_Md5ToData.find(md5);
    if (it != m_Md5ToData.end())
    {
        return it->second;
    }

    static string empty;
    return empty;
}
