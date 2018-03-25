//
// Created by carrot on 18-3-25.
//

#include <string.h>
#include "FileHelper.h"

inline File::~File()
{
    if (m_fp)
    {
        fclose(m_fp);
    }
}

inline bool File::Open(const string &filename, char const *mode)
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

inline File::File(const string &filename, char const *mode)
{
    Open(filename, mode);
}

inline size_t File::ReadBytes(char *buffer, size_t nitems)
{
    return Read(buffer, 1, nitems);
}

inline size_t File::Read(void *buffer, size_t size, size_t nitems)
{
    size_t ret = 0;
    if (m_fp)
    {
        ret = fread(buffer, size, nitems, m_fp);
    }
    return ret;
}

inline size_t File::WriteBytes(const char *buffer, size_t nitems, bool flush)
{
    return Write(buffer, 1, nitems, flush);
}

inline size_t File::Write(const void *buffer, size_t size, size_t nitems, bool flush)
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

inline bool File::Flush()
{
    if (m_fp)
    {
        fflush(m_fp);
        return true;
    }
    return false;
}

inline string File::Path()
{
    return m_path;
}

string File::BaseName()
{
    return m_basename;
}

inline std::shared_ptr<File> FileHelper::CreateFilePtr(const string &filename, char const *mode)
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
