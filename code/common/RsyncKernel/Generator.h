//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_GENERATOR_H
#define RSYNCTOOL_GENERATOR_H


#include <memory>
#include "cm_define.h"
#include "cm_struct.h"

/*
 * Generator 负责一次性生成文件的分块多级签名信息,每个Task一个
 * 输入： 文件名和分块大小
 * 输出： 文件分块信息
 */

class Generator
{
    typedef RTVector<ST_BlockInfo> BlockInfoVec;

public:
    const BlockInfoVec& GetBlockInfos(const string& filename);

    bool GenerateBlockInfos(const string& filename, uint32_t splitsize);

    const string &GetDataByMd5(const string &md5);

private:

    RTMap<string, BlockInfoVec>     m_fileInfos;    //name ==> BlockInfos
    RTMap<string, string>           m_Md5ToData;    //MD5  ==> data
};

typedef std::shared_ptr<Generator> GeneratorPtr;


#endif //RSYNCTOOL_GENERATOR_H
