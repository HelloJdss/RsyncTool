//
// Created by carrot on 18-3-25.
//

#ifndef RSYNCTOOL_FILEHELPER_H
#define RSYNCTOOL_FILEHELPER_H

/*
 * 本模块包含文件的相关操作
 */

#include <string>
#include <memory> //shared_ptr

using std::string;

class File
{
public:

    ~File();

    bool Open(const string &filename, const char *mode = "a+");

    size_t ReadBytes(char* buffer, size_t nitems);

    size_t ReadBytes(char* buffer, size_t nitems, size_t offset);

    size_t Read(void* buffer, size_t size, size_t nitems);     //需要指定buffer的数据类型字长size

    size_t Read(void* buffer, size_t size, size_t nitems, size_t offset);   //从指定位置读取

    size_t WriteBytes(const char* buffer, size_t nitems, bool flush);   //flush为true则立即执行flush

    size_t WriteBytes(const char* buffer, size_t nitems, size_t offset, bool flush); //从指定位置写入

    size_t Write(const void* buffer, size_t size, size_t nitems, bool flush);

    size_t Write(const void* buffer, size_t size, size_t nitems, size_t offset, bool flush);

    bool SetSize(size_t size);  //设置文件的大小

    bool Flush();

    string BaseName();

    string Path();

    int64_t Size();

    bool Eof();

private:
    friend class FileHelper;

    explicit File(const string &filename, const char *mode = "a+");

    string m_basename;

    string m_path;

    FILE *m_fp = nullptr;
};

typedef std::shared_ptr<File> FilePtr;

class FileHelper
{
public:
    static std::shared_ptr<File> CreateFilePtr(const string &filename, const char *mode = "a+");

    static string GetPath(const string& filename);
};

#endif //RSYNCTOOL_FILEHELPER_H
