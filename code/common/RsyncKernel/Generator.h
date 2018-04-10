//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_GENERATOR_H
#define RSYNCTOOL_GENERATOR_H


#include <memory>
#include "cm_define.h"
#include "cm_struct.h"

/*
 * Generator 负责一次性生成文件的分块多级签名信息,每个文件一个
 * 输入： 文件名和分块大小
 * 输出： 文件分块信息
 */

class Generator
{
public:
    Generator();

    Generator(const string& filename, uint32_t splitsize);

    bool Generate(const string &filename, uint32_t splitsize);

    const RTVector<ST_BlockInfo> &GetBlockInfoVec(); //获取生成的块签名信息

    const string &GetChunkDataByMd5(const string &md5); //获取分块数据

private:

    RTVector<ST_BlockInfo>          m_blockInfoVec;    //BlockInfos
    RTMap<string, string>           m_Md5ToData;       //MD5  ==> data
};

typedef std::shared_ptr<Generator> GeneratorPtr;


#endif //RSYNCTOOL_GENERATOR_H
