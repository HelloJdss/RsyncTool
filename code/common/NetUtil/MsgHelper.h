//
// Created by carrot on 2018/3/18.
//

#ifndef RSYNCTOOL_MSGHELPER_H
#define RSYNCTOOL_MSGHELPER_H

#include <memory>
#include <queue>
#include "cm_define.h"
#include "cm_struct.h"

#define BUFFER_INIT_SIZE 1500

class Bytes
{
public:
    Bytes(const Bytes &bytes);

    Bytes &operator=(const Bytes &bytes);

    ~Bytes();

    std::shared_ptr<Bytes> Skip(uint32_t num);

    std::shared_ptr<Bytes> Concat(const Bytes &bytes, uint32_t from = 0, uint32_t to = -1);

    uint32_t Size() const
    { return m_length; }

    char *ToChars() const
    { return reinterpret_cast<char *>(m_bytes); }

    std::vector<char> ToArray() const;

    std::string ToString() const;

private:
    friend class MsgHelper;

    Bytes(unsigned char *in, uint32_t size);

    Bytes()
    {}

    uint8_t *m_bytes = nullptr;
    uint32_t m_length = 0;
};

typedef std::shared_ptr<Bytes> BytesPtr;

class MsgHelper
{
public:
    MsgHelper();

    MsgHelper(const MsgHelper &) = delete;

    MsgHelper &operator=(const MsgHelper &) = delete;

    static BytesPtr CreateBytes(char inData[], uint32_t size);

    static BytesPtr CreateBytes(uint8_t inData[], uint32_t size);

    /**
     * @brief [0..7][8..m][m+1..n] Length + Header + Data
     * @param inData
     * @param header
     * @return
     */
    static BytesPtr PackageData(ST_PackageHeader &header, BytesPtr inData);

    bool ReadMessage(ST_PackageHeader &outHeader, BytesPtr *outData);


    unsigned char *GetBuffer()
    { return m_buffer; }

    int GetStartIndex()
    { return m_currentPos; }

    int GetRemainBytes()
    { return m_buffer_size - m_currentPos; }

    void AddCount(int count);

    bool HasMessage();

private:
    void resetBuffer();

    void disposal();

    unsigned char *m_buffer = nullptr;
    size_t m_buffer_size = 0;

    size_t m_processPos, m_currentPos;
    std::queue<BytesPtr> m_msgs;

    pthread_mutex_t m_mutex;
};


#endif //RSYNCTOOL_MSGHELPER_H
