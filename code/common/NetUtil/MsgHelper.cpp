//
// Created by carrot on 2018/3/18.
//

#include <cstring>
#include "LogHelper.h"
#include "MsgHelper.h"

inline Bytes::Bytes(unsigned char *in, size_t size)
{
    m_bytes = new unsigned char[size];
    bzero(m_bytes, size);
    memcpy(m_bytes, in, size);
    m_length = size;
}

inline Bytes::~Bytes()
{
    delete[] m_bytes;
    m_bytes = nullptr;
    m_length = 0;
}

inline Bytes::Bytes(const Bytes &bytes)
{
    m_bytes = new unsigned char[bytes.m_length];
    bzero(m_bytes, bytes.m_length);
    memcpy(m_bytes, bytes.m_bytes, bytes.m_length);
    m_length = bytes.m_length;
}

Bytes &Bytes::operator=(const Bytes &bytes)
{
    if (this == &bytes)
    {
        return *this;
    }

    m_bytes = new unsigned char[bytes.m_length];
    bzero(m_bytes, bytes.m_length);
    memcpy(m_bytes, bytes.m_bytes, bytes.m_length);
    m_length = bytes.m_length;

    return *this;
}

inline std::shared_ptr<Bytes> Bytes::Skip(size_t num)
{
    if(num >= m_length)
    {
        return nullptr;
    }
    return std::shared_ptr<Bytes>(new Bytes(m_bytes + num, m_length - num));
}

std::shared_ptr<Bytes> Bytes::Concat(const Bytes &bytes, size_t from, size_t to)
{
    to = to >= bytes.m_length ? bytes.m_length : to;
    if (to >= from)
    {
        size_t delta = to - from;
        unsigned char* temp = new unsigned char[m_length + delta];
        bzero(temp, m_length + delta);
        memcpy(temp, m_bytes, m_length);
        memcpy(temp + m_length, bytes.m_bytes + from, delta);
        delete[] m_bytes;
        m_bytes = temp;
        m_length = m_length + delta;
    }
    return std::shared_ptr<Bytes>(this);
}

std::vector<char> Bytes::ToArray() const
{
    std::vector<char> t;
    for (size_t i = 0; i < m_length; ++i)
    {
        t.push_back(m_bytes[i]);
    }
    return t;
}

std::string Bytes::ToString() const
{
    return std::string((char*)m_bytes);
}

BytesPtr MsgHelper::CreateBytes(char *indata, int size)
{
    return BytesPtr(new Bytes(reinterpret_cast<unsigned char *>(indata), size));
}

BytesPtr MsgHelper::PackageData(ST_PackageHeader& header, BytesPtr inData)
{
    Bytes* t = new Bytes();
    size_t datalength = sizeof(ST_PackageHeader) + inData->Size();
    t->m_length = 8 + datalength;
    t->m_bytes = new unsigned char[t->m_length];
    bzero(t->m_bytes, t->m_length);
    memcpy(t->m_bytes, &datalength, sizeof(datalength));
    memcpy(t->m_bytes + 8, &header, sizeof(ST_PackageHeader));
    memcpy(t->m_bytes + 8 + sizeof(ST_PackageHeader), inData->ToChars(), inData->Size());
    return BytesPtr(t);
}

MsgHelper::MsgHelper()
{
    pthread_mutex_init(&m_mutex, NULL);
    resetBuffer();
}

inline void MsgHelper::resetBuffer()
{
    if(m_buffer != nullptr)
    {
        delete [] m_buffer;
    }
    m_buffer = new unsigned char[BUFFER_INIT_SIZE];
    bzero(m_buffer, BUFFER_INIT_SIZE);
    m_buffer_size = BUFFER_INIT_SIZE;
    m_processPos = 0;
    m_currentPos = 0;
}

void MsgHelper::AddCount(int count)
{
    if (count <= 0) return;

    m_currentPos += count;

    disposal();

    if(GetRemainBytes() <= 0)
    {
        int newsize = m_buffer_size * 2;
        unsigned char *t = new unsigned char[newsize];
        bzero(t, newsize);
        memcpy(t, m_buffer + m_processPos, m_buffer_size - m_processPos);
        m_processPos = 0;
        delete [] m_buffer;
        m_buffer = t;
        m_buffer_size = newsize;
        LOG_TRACE("MsgHelper::buffer size reset to %d", m_buffer_size);
    }
}

void MsgHelper::disposal()
{
    while (true)
    {
        if (m_currentPos - m_processPos <= 8)
        {
            return;
        }
        size_t datacount = *(size_t*)(m_buffer + m_processPos);
        if((m_currentPos - m_processPos) >= datacount + 8 && m_processPos + 8 + datacount < m_buffer_size)
        {
            pthread_mutex_lock(&m_mutex);
            m_msgs.push(CreateBytes(reinterpret_cast<char *>(m_buffer + m_processPos + 8), datacount));
            pthread_mutex_unlock(&m_mutex);

            m_processPos += datacount + 8;
            if (m_processPos == m_currentPos)
            {
                resetBuffer();
                return;
            }
        }
        else
        {
            return;
        }
    }
}

void MsgHelper::ReadMessage(ST_PackageHeader &outheader, BytesPtr* outdata)
{
    *outdata = nullptr;
    outheader.reset();
    if (HasMessage())
    {
        pthread_mutex_lock(&m_mutex);
        BytesPtr bytesPtr = m_msgs.front();
        m_msgs.pop();
        pthread_mutex_unlock(&m_mutex);

        outheader = *(ST_PackageHeader*)bytesPtr->m_bytes;
        *outdata = CreateBytes(reinterpret_cast<char *>(bytesPtr->m_bytes + sizeof(ST_PackageHeader)),
                               bytesPtr->m_length - sizeof(ST_PackageHeader));
    }
}

inline bool MsgHelper::HasMessage()
{
    pthread_mutex_lock(&m_mutex);
    bool ret = m_msgs.size() > 0;
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

