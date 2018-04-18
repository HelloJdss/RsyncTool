//
// Created by carrot on 18-3-25.
//

#include <cstring>
#include <unistd.h>
#include "FileHelper.h"
#include "LogHelper.h"


File::~File()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
        //LOG_TRACE("Close File[%s]!", m_name.c_str());
    }
}

bool File::Open(const string &filename, char const *mode)
{
    if(strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0)
    {
        string dir;
        if(FileHelper::SplitDirAndFile(filename, &dir) && -1 == access(dir.c_str(), F_OK | W_OK)) //存在目录需要先创建目录
        {
          //  mkdir(dir.c_str(), 0777); //一次只能创建一级目录
          //  LOG_WARN("mkdir: %s", dir.c_str());
          /*  size_t pos = 0;
            while((pos = dir.find_first_of('/', pos)) != string::npos)
            {
                if(access(dir.substr(0, pos).c_str(), F_OK) == -1) //此级目录不存在，则创建
                {
                    if(mkdir(dir.substr(0, pos).c_str(), 0777) != -1) //一次只能创建一级目录
                    {
                        LOG_WARN("mkdir: %s", dir.substr(0, pos).c_str());
                    }
                }
                pos++;
            }
            */
          FileHelper::MakeDir(dir);
        }
    }

    if (m_fp)
    {
        fclose(m_fp);
    }
    m_fp = fopen(filename.c_str(), mode);
    m_name = filename;

    //LOG_TRACE("Open File[%s]:[%s]!", m_name.c_str(), m_fp != nullptr ? "Success" : "Failed");
    return m_fp != nullptr;
}

File::File(const string &file_name, char const *mode)
{
    Open(file_name, mode);
}

size_t File::ReadBytes(char *buffer, size_t nitems)
{
    return Read(buffer, 1, nitems);
}

size_t File::Read(void *buffer, size_t size, size_t nitems)
{
    size_t ret = 0;
    if (m_fp)
    {
        ret = fread(buffer, size, nitems, m_fp);
    }
    return ret;
}

size_t File::WriteBytes(const char *buffer, size_t nitems, bool flush)
{
    return Write(buffer, 1, nitems, flush);
}

size_t File::Write(const void *buffer, size_t size, size_t nitems, bool flush)
{
    size_t ret = 0;
    if (m_fp)
    {
        pthread_mutex_lock(&m_mutex);
        ret = fwrite(buffer, size, nitems, m_fp);
        if (flush)
        {
            fflush(m_fp);
        }
        pthread_mutex_unlock(&m_mutex);
    }
    return ret;
}

bool File::Flush()
{
    if (m_fp)
    {
        fflush(m_fp);
        return true;
    }
    return false;
}

string File::Path()
{
    return m_name;
}

int64_t File::Size()
{
    if (!m_fp)
    {
        return -1;
    }
    int64_t n = ftell(m_fp);
    fseek(m_fp, 0, SEEK_END);
    int64_t ret = ftell(m_fp);
    fseek(m_fp, n, SEEK_SET);

    return ret;
}

bool File::SetSize(size_t size)
{
    if (m_fp)
    {
        if (size > 0)
        {
            int64_t n = ftell(m_fp);    //记录旧指针
            fseek(m_fp, size - 1, SEEK_SET);
            fputc(0, m_fp);
            fseek(m_fp, n, SEEK_SET);
        }
    }
    return false;
}

size_t File::Read(void *buffer, size_t size, size_t nitems, size_t offset)
{
    if (m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return Read(buffer, size, nitems);
    }
    return 0;
}

size_t File::ReadBytes(char *buffer, size_t nitems, size_t offset)
{
    if (m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return ReadBytes(buffer, nitems);
    }
    return 0;
}

size_t File::WriteBytes(const char *buffer, size_t nitems, size_t offset, bool flush)
{
    if (m_fp)
    {
        pthread_mutex_lock(&m_mutex);
        fseek(m_fp, offset, SEEK_SET);
        auto ret = fwrite(buffer, 1, nitems, m_fp);
        if (flush)
        {
            fflush(m_fp);
        }
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }
    return 0;
}

size_t File::Write(const void *buffer, size_t size, size_t nitems, size_t offset, bool flush)
{
    if (m_fp)
    {
        pthread_mutex_lock(&m_mutex);
        fseek(m_fp, offset, SEEK_SET);
        auto ret = fwrite(buffer, size, nitems, m_fp);
        if (flush)
        {
            fflush(m_fp);
        }
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }
    return 0;
}

bool File::Eof()
{
    if (m_fp)
    {
        return static_cast<bool>(feof(m_fp));
    }
    return false;
}

struct stat File::Stat()
{
    stat(m_name.c_str(), &m_stat); //返回文件本身的信息
    return m_stat;
}

FILE *File::GetPointer()
{
    return m_fp;
}

string File::BaseName()
{
    return basename(m_name.c_str());
}

FilePtr FileHelper::OpenFile(const string &file_name, char const *mode)
{
    auto fp = std::shared_ptr<File>(new File(file_name, mode));
    if (fp->m_fp)
    {
        return fp;
    }
    return nullptr;
}

string FileHelper::GetRealPath(const string &path)
{
    char buff[1024];
    bzero(buff, 1024);
    realpath(path.c_str(), buff);
    return std::string(buff);
}

bool FileHelper::SplitDirAndFile(const string &path, string *outDir, string* outBasename)
{
    auto pos = path.find_last_of('/');
    if (pos != string::npos)
    {
        if(outDir)
        {
            *outDir = path.substr(0, pos + 1);
        }
        if(outBasename)
        {
            *outBasename = path.substr(pos + 1);
        }
        return true;
    }
    return false;
}

DirPtr FileHelper::OpenDir(const string &dir_name)
{
    auto dp = std::shared_ptr<Dir>(new Dir(dir_name));
    if (dp->m_dp)
    {
        return dp;
    }
    return nullptr;
}

int FileHelper::Access(const string &path, int mode)
{
    return access(path.c_str(), mode);
}

string FileHelper::BaseName(const string &path)
{
    return basename(path.c_str());
}

int FileHelper::Rename(const string &src, const string &des)
{
    return rename(src.c_str(), des.c_str());
}

void FileHelper::MakeDir(const string &dir)
{
    size_t pos = 0;
    while((pos = dir.find_first_of('/', pos)) != string::npos)
    {
        if(access(dir.substr(0, pos).c_str(), F_OK) == -1) //此级目录不存在，则创建
        {
            if(mkdir(dir.substr(0, pos).c_str(), 0777) != -1) //一次只能创建一级目录
            {
                LOG_WARN("mkdir: %s", dir.substr(0, pos).c_str());
            }
        }
        pos++;
    }
}

Dir::~Dir()
{
    if(!m_dp)
    {
        closedir(m_dp);
    }
}

Dir::Dir(string dir_name)
{
    if(dir_name.back() == '/')
    {
        dir_name.pop_back();
    }
    m_path = dir_name;
    m_dp = opendir(dir_name.c_str());
    LOG_TRACE("Open dir[%s]", m_path.c_str());
}

string Dir::Path()
{
    return m_path;
}

void Dir::traverse(char *dir, string prefix, std::vector<string>& list)
{
    DIR *dp;
    struct dirent *entry;
    struct stat stat1;

    if((dp = opendir(dir)) == nullptr)
    {
        LOG_WARN("Cannot Open Dir:%s Err:%s", dir, strerror(errno));
        return;
    }

    chdir(dir);

    prefix.append(dir).append("/");

    while ((entry = readdir(dp)) != nullptr)
    {
        lstat(entry->d_name, &stat1); //使用lstat来处理符号链接指向的本身文件
        if(S_ISDIR(stat1.st_mode))
        {
            if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
            {
                continue;
            }

            //进入下层目录中
            string str = prefix + entry->d_name + "/";
            list.push_back(str); //写入目录名
            LOG_TRACE("Add Dir[%s]", str.c_str());
            traverse(entry->d_name, prefix, list);
        }
        else
        {
            LOG_TRACE("Add File[%s]", (prefix + entry->d_name).c_str());

            list.push_back(prefix + entry->d_name);
        }
    }

    chdir("..");
    closedir(dp);
}

std::vector<string> Dir::AllFiles()
{
    std::vector<string> list;
    if(m_dp)
    {
        closedir(m_dp);
        traverse(const_cast<char *>(m_path.c_str()), "", list);
        m_dp = opendir(m_path.c_str());
    }
    else
    {
        traverse(const_cast<char *>(m_path.c_str()), "", list);
    }
    return list;
}
