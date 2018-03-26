//
// Created by carrot on 18-3-23.
//

#include "LogHelper.h"
#include "Generator.h"
#include "RollingChecksum.h"
#include "MD5.h"

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
    FILE *fp;
    fp = fopen(filename.c_str(), "rb");
    if (fp == nullptr)
    {
        LOG_ERROR("%s", strerror(errno));
        return false;
    }


    fseek(fp, 0, SEEK_END);

    int64_t n = ftell(fp);
    if (n == -1)
    {
        LOG_ERROR("%s", strerror(errno));
        return false;
    }

    LOG_TRACE("File[%s] Open Success! Length:[%llu], SplitSize: %lu", basename(filename.c_str()), n, splitsize);

    fseek(fp, 0, SEEK_SET);

    m_fileInfos[filename].clear();

    char buff[splitsize];

    int64_t i = 0;

    while (i < n)
    {
        ST_BlockInfo info;
        info.filename = string(basename(filename.c_str()));
        info.order = i / splitsize;
        info.offset = i;

        size_t count = fread(buff, 1, splitsize, fp);

        i += count;

        info.length = count;

        info.checksum = RollingChecksum::adler32_checksum(buff, splitsize);

        info.md5 = md5(buff, count);

        m_fileInfos[filename].push_back(info);

        m_Md5ToData[info.md5].assign(buff, count);

        LOG_TRACE("[%3lld\%] Finish Block[%lld]: Offset: %lld Length: %lld CheckSum: %u MD5: %s", i * 100 / n,
                  info.order,
                  info.offset, info.length, info.checksum, info.md5.c_str());
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
