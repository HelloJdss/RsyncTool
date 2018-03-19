//
// Created by 覃宇骁 on 2018/3/18.
//

#include "MsgHelper.h"

inline Bytes::Bytes(unsigned char *in, int size)
{
    m_bytes = new unsigned char[size];
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
        memcpy(temp, m_bytes, m_length);
        memcpy(temp + m_length, bytes.m_bytes + from, delta);
        delete[] m_bytes;
        m_bytes = temp;
        m_length = m_length + delta;
    }
    return std::shared_ptr<Bytes>(this);
}

BytesPtr MsgHelper::CreateBytes(char *indata, int size)
{
    return BytesPtr(new Bytes(reinterpret_cast<unsigned char *>(indata), size));
}

BytesPtr MsgHelper::PackageData(BytesPtr inData, ST_PackageHeader& header)
{
    Bytes* t = new Bytes();
    int datalength = sizeof(ST_PackageHeader) + inData->Size();
    t->m_length = 4 + datalength;
    t->m_bytes = new unsigned char[t->m_length];
    memcpy(t->m_bytes, &datalength, 4);
    memcpy(t->m_bytes + 4, &header, sizeof(ST_PackageHeader));
    memcpy(t->m_bytes + 4 + sizeof(ST_PackageHeader), inData->ToChars(), inData->Size());
    return BytesPtr(t);
}

