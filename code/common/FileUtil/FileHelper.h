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
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

using std::string;

class File
{
public:

    ~File();

    bool Open(const string &filename, const char *mode = "a+");

    size_t ReadBytes(char *buffer, size_t nitems);

    size_t ReadBytes(char *buffer, size_t nitems, size_t offset);

    size_t Read(void *buffer, size_t size, size_t nitems);     //需要指定buffer的数据类型字长size

    size_t Read(void *buffer, size_t size, size_t nitems, size_t offset);   //从指定位置读取

    size_t WriteBytes(const char *buffer, size_t nitems, bool flush);   //flush为true则立即执行flush

    size_t WriteBytes(const char *buffer, size_t nitems, size_t offset, bool flush); //从指定位置写入

    size_t Write(const void *buffer, size_t size, size_t nitems, bool flush);

    size_t Write(const void *buffer, size_t size, size_t nitems, size_t offset, bool flush);

    bool SetSize(size_t size);  //设置文件的大小

    bool Flush();

    string Name();

    int64_t Size();

    bool Eof();

    struct stat Stat();

    FILE* GetPointer();

private:
    friend class FileHelper;

    explicit File(const string &file_name, const char *mode = "a+");

    string m_name;

    FILE *m_fp = nullptr;

    struct stat m_stat;
};

typedef std::shared_ptr<File> FilePtr;

class Dir
{
public:
    ~Dir();

    string Path();

    std::vector<string> AllFiles(); //获取目录下的所有文件名

private:
    friend class FileHelper;

    explicit Dir(string dir_name);

    void traverse(char *dir, string prefix, std::vector<string>& list);

    string m_path;
    DIR *m_dp = nullptr;
};

typedef std::shared_ptr<Dir> DirPtr;

class FileHelper
{
public:
    static FilePtr OpenFile(const string &file_name, const char *mode = "a+"); //打开文件

    static DirPtr OpenDir(const string &dir_name); //打开目录

    static string GetRealPath(const string &path); //获取完整的绝对路径

    static bool SplitDirAndFile(const string &path, string *outDir, string* outBasename = nullptr); //分离文件所在目录以及文件名

    static int Access(const string &path, int mode); //对目录或文件做测试

private:
};

#endif //RSYNCTOOL_FILEHELPER_H
