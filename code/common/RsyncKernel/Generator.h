//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_GENERATOR_H
#define RSYNCTOOL_GENERATOR_H


#include <memory>
#include "cm_define.h"

/*
 * Generator 负责一次性生成文件的分块多级签名信息,每个文件一个
 * 输入： 文件名和分块大小
 * 输出： 文件分块信息
 */

struct ST_BlockInformation;

class Generator
{
public:

    /**
      * 获取一个新的generator实例
      * @param filename 文件名称
      * @param split 分块的大小，如果不指定，则自动动态生成，块大小最小为512，最大近似为其开方，例如4G的文件，块大小为65536
      * @return GeneratorPtr 新实例的指针
      */

    static std::shared_ptr<Generator> NewGenerator(const std::string &filename, uint32_t split = 0);

    Generator();

    ~Generator();

    bool Generate(const std::string &filename, uint32_t split = 0);

    const RTVector<ST_BlockInformation> &GetChunkDigestVec(); //获取生成的块签名信息

    const std::string &GetChunkDataByMd5(const std::string &md5); //获取分块数据

    uint32_t GetSplit()
    { return m_split; }

private:

    RTVector<ST_BlockInformation>          m_blockInfoVec;
    RTMap<std::string, std::string>           m_Md5ToData;       //MD5  ==> data
    uint32_t m_split = 0;
};

typedef std::shared_ptr<Generator> GeneratorPtr;


#endif //RSYNCTOOL_GENERATOR_H
