//
// Created by carrot on 18-3-23.
//

#include "LogHelper.h"
#include "cm_struct.h"
#include "Generator.h"
#include "MD5.h"
#include "FileHelper.h"
#include "RollingChecksum.h"
#include "cmath"

const RTVector<ST_BlockInformation> &Generator::GetChunkDigestVec()
{
    return m_blockInfoVec;
}

bool Generator::Generate(const string &filename, uint32_t split)
{
    m_split = split;

    auto fp = FileHelper::OpenFile(filename, "rb");
    LogCheckCondition(fp!= nullptr, false, "Open file [%s] failed! errno: %s", filename.c_str(), strerror(errno));

    int64_t n = fp->Size();
    if (n == -1)
    {
        LOG_LastError();
        return false;
    }

    if(m_split < 512)
    {
        m_split = (uint32_t)std::sqrt(n + 512 * 512);
    }

    //LOG_DEBUG("File[%s] Open Success! Length:[%llu], SplitSize: %lu", basename(filename.c_str()), n, m_split);

    m_blockInfoVec.clear();
    m_Md5ToData.clear();

    char buff[m_split];

    int64_t i = 0;

    while (i < n)
    {
        ST_BlockInformation info;
        info.offset = i;

        size_t count = fp->ReadBytes(buff, m_split);

        i += count;

        info.length = count;

        info.checksum = RollingChecksum::adler32_checksum(buff, count);

        info.md5 = md5(buff, count);

        info.data.assign(buff, count);

        m_blockInfoVec.push_back(info);

        m_Md5ToData[info.md5] = m_blockInfoVec.back().data;   //这种赋值方式是为了利用string的写时复制特性，避免不必要的拷贝

        LOG_TRACE("[%3lld\%] Finish Block: Offset: [%lld] Length: [%d] CheckSum: [%u] MD5: [%s]", i * 100 / n,
                  info.offset, info.length, info.checksum, info.md5.c_str());
    }

    return true;
}

const string &Generator::GetChunkDataByMd5(const string &md5)
{
    return m_Md5ToData[md5];
}

Generator::Generator()
{
    m_blockInfoVec.clear();
    m_Md5ToData.clear();
}

std::shared_ptr<Generator> Generator::NewGenerator(const std::string &filename, uint32_t split)
{
    auto ptr = std::make_shared<Generator>();
    if(ptr->Generate(filename, split))
    {
        return ptr;
    }
    return nullptr;
}

Generator::~Generator()
{
    m_Md5ToData.clear();
    m_blockInfoVec.clear();
    //LOG_TRACE("~Generator");
}
