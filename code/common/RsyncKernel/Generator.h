//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_GENERATOR_H
#define RSYNCTOOL_GENERATOR_H

#include <string>
#include <vector>
#include <unordered_map>

using std::string;
using std::vector;
using std::unordered_map;

#define RTVector    std::vector
#define RTMap       std::unordered_map

struct ST_BlockInfo
{
    string      filename;
    int32_t     splitsize;
    int64_t     order;
    int64_t     offset;
    int64_t     length;
    uint32_t    checksum;
    string      md5;

    ST_BlockInfo()
    {
        filename.clear();
        splitsize = 0;
        order = 0;
        offset = 0;
        length = 0;
        checksum = 0;
        md5.clear();
    }
};

/*
 * Generator 负责一次性生成文件的分块多级签名信息
 */

class Generator
{
    typedef RTVector<ST_BlockInfo> BlockInfoVec;

public:
    const BlockInfoVec& GetBlockInfos(const string& filename);

    bool GenerateBlockInfos(const string& filename, uint32_t splitsize = 512);

    const string &GetDataByMd5(const string &md5);

private:

    RTMap<string, BlockInfoVec>     m_fileInfos;
    RTMap<string, string>           m_Md5ToData;    //MD5 ==> data
};


#endif //RSYNCTOOL_GENERATOR_H
