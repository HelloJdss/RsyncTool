//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_INSPECTOR_H
#define RSYNCTOOL_INSPECTOR_H

#include <cstdint>
#include <string>


using std::string;
struct ST_NewBlock
{
    string      filename;
    int32_t     splitsize;
    int64_t     order;
    int64_t     offset;
    int64_t     length;
    string      md5;    //如果为空，则说明可以通过原有数据重建，否则应该使用data段的数据重建
    string      data;   //如果非空，则为该块的具体数据

    ST_NewBlock()
    {
        filename.clear();
        splitsize = 0;
        order = 0;
        offset = 0;
        length = 0;
        md5.clear();
        data.clear();
    }
};

/*
 * Inspector负责根据拿到的多级签名列表，检验本地文件，生成重建文件信息列表并返回
 */

class Inspector
{
    bool CheckFileExist(const string& filePathname);
};


#endif //RSYNCTOOL_INSPECTOR_H
