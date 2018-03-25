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

    size_t Read(void* buffer, size_t size, size_t nitems);     //需要指定buffer的数据类型字长size

    size_t WriteBytes(const char* buffer, size_t nitems, bool flush = false);   //flush为true则立即执行flush

    size_t Write(const void* buffer, size_t size, size_t nitems, bool flush = false);

    bool Flush();

    string BaseName();

    string Path();

private:
    friend class FileHelper;

    explicit File(const string &filename, const char *mode = "a+");

    string m_basename;

    string m_path;

    FILE *m_fp = nullptr;
};

class FileHelper
{
public:
    static std::shared_ptr<File> CreateFilePtr(const string &filename, const char *mode = "a+");

    static string GetPath(const string& filename);
};

typedef std::shared_ptr<File> FilePtr;

#endif //RSYNCTOOL_FILEHELPER_H
