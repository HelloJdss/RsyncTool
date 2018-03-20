//
// Created by carrot on 2018/3/18.
//

#ifndef RSYNCTOOL_MSGHELPER_H
#define RSYNCTOOL_MSGHELPER_H

#include <memory>
#include <queue>
#include <semaphore.h>
#include "cm_define.h"
#include "cm_struct.h"

#define BUFFER_INIT_SIZE 100

class Bytes
{
public:
    Bytes(const Bytes &bytes);

    Bytes &operator=(const Bytes &bytes);

    ~Bytes();

    std::shared_ptr<Bytes> Skip(size_t num);

    /**
     * @brief Concat a Bytes
     * @param bytes
     * @param from
     * @param to
     * @return
     */
    std::shared_ptr<Bytes> Concat(const Bytes &bytes, size_t from = 0, size_t to = -1);

    size_t Size() const
    { return m_length; }

    char *ToChars() const
    { return reinterpret_cast<char *>(m_bytes); }

    std::vector<char> ToArray() const;

    std::string ToString() const;

private:
    friend class MsgHelper;

    Bytes(unsigned char *in, size_t size);

    Bytes()
    {}

    unsigned char *m_bytes = nullptr;
    size_t m_length = 0;
};

typedef std::shared_ptr<Bytes> BytesPtr;

class MsgHelper
{
public:
    MsgHelper();

    MsgHelper(const MsgHelper &) = delete;

    MsgHelper &operator=(const MsgHelper &) = delete;

    static BytesPtr CreateBytes(char inData[], int size);

    /**
     * @brief [0..7][8..m][m+1..n] Length + Header + Data
     * @param inData
     * @param header
     * @return
     */
    static BytesPtr PackageData(ST_PackageHeader &outheader, BytesPtr inData);

    void ReadMessage(ST_PackageHeader &outheader, BytesPtr *outdata);

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
    int m_buffer_size = 0;

    uint32_t m_processPos, m_currentPos;
    std::queue<BytesPtr> m_msgs;

    pthread_mutex_t m_mutex;
};


#endif //RSYNCTOOL_MSGHELPER_H
