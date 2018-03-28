//
// Created by carrot on 18-3-25.
//

#include <string.h>
#include "FileHelper.h"

File::~File()
{
    if (m_fp)
    {
        fclose(m_fp);
    }
}

bool File::Open(const string &filename, char const *mode)
{
    if (m_fp)
    {
        fclose(m_fp);
    }
    m_fp = fopen(filename.c_str(), mode);
    m_basename = basename(filename.c_str());

    m_path = FileHelper::GetPath(filename);
    return m_fp != nullptr;
}

File::File(const string &filename, char const *mode)
{
    Open(filename, mode);
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
        ret = fwrite(buffer, size, nitems, m_fp);
        if (flush)
        {
            fflush(m_fp);
        }
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
    return m_path;
}

string File::BaseName()
{
    return m_basename;
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
    if(m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return Read(buffer, size, nitems);
    }
    return 0;
}

size_t File::ReadBytes(char *buffer, size_t nitems, size_t offset)
{
    if(m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return ReadBytes(buffer, nitems);
    }
    return 0;
}

size_t File::WriteBytes(const char *buffer, size_t nitems, size_t offset, bool flush)
{
    if(m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return WriteBytes(buffer, nitems, flush);
    }
    return 0;
}

size_t File::Write(const void *buffer, size_t size, size_t nitems, size_t offset, bool flush)
{
    if(m_fp)
    {
        fseek(m_fp, offset, SEEK_SET);
        return Write(buffer, size, nitems, flush);
    }
    return 0;
}

bool File::Eof()
{
    if(m_fp)
    {
        return static_cast<bool>(feof(m_fp));
    }
    return false;
}

std::shared_ptr<File> FileHelper::CreateFilePtr(const string &filename, char const *mode)
{
    auto fp = std::shared_ptr<File>(new File(filename, mode));
    if (fp->m_fp)
    {
        return fp;
    }
    return nullptr;
}

string FileHelper::GetPath(const string &filename)
{
    char path[512];
    bzero(path, 512);
    realpath(filename.c_str(), path);
    return std::string(path);
}
