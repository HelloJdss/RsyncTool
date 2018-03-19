//
// Created by carrot on 18-3-15.
//

#ifndef RSYNCTOOL_NETHELPER_H
#define RSYNCTOOL_NETHELPER_H

#include <netinet/in.h>
#include <cstring> //memcpy
#include <memory> //shared_ptr
#include <vector>
#include "LogModule/LogHelper.h"

#define NO_ERROR 0

using std::string;
using std::vector;

class SocketAddress //now only support IPV4
{
public:
    SocketAddress()
    { bzero(GetAsSockAddrIn(), sizeof(*GetAsSockAddrIn())); };

    SocketAddress(uint32_t inAddr, uint16_t inPort);

    SocketAddress(const char *inAddr, uint16_t inPort);

    SocketAddress(const sockaddr &inSockAddr)
    { memcpy(&m_sockaddr, &inSockAddr, sizeof(sockaddr)); }

    sockaddr_in *GetAsSockAddrIn()
    { return reinterpret_cast<sockaddr_in *>(&m_sockaddr); }

    size_t GetSize() const
    { return sizeof(sockaddr); }

private:
    friend class UDPSocket;

    friend class TCPSocket;

    sockaddr m_sockaddr;
};

typedef std::shared_ptr<SocketAddress> SocketAddressPtr;

class UDPSocket
{
public:
    ~UDPSocket();

    int Bind(SocketAddress &inSrcAddr); //if success return NO_ERROR

    int SendTo(const void *inData, int inLen, const SocketAddress &inTo); //Exception: errno

    int RecvFrom(void *inBuffer, int inLen, SocketAddress &outFrom); //Exception: errno

    void Close();

    string GetEndPoint() const;

    bool IsAvailable() const
    { return m_socket != -1; }

private:
    friend class NetHelper;

    UDPSocket(int inSocket) : m_socket(inSocket)
    {}

    int m_socket = -1;
    char *m_ip = const_cast<char *>("unknown");
    uint16_t m_port = 0;
};

typedef std::shared_ptr<UDPSocket> UDPSocketPtr;


class TCPSocket
{
public:
    ~TCPSocket();

    void Connect(const SocketAddress &inDesAddr); //Exception: errno

    int Bind(SocketAddress &inSrcAddr); //if success return NO_ERROR

    void Listen(int inBackLog = 32); //Exception: errno

    std::shared_ptr<TCPSocket> Accept(SocketAddress &inFromAddr);

    int Send(const void *inData, int inLen);//Exception: errno

    int Receive(void *inBuffer, int inLen);//Exception: errno

    void Close();

    string GetEndPoint() const;

    bool IsAvailable() const
    { return m_socket != -1; }

private:
    friend class NetHelper;

    TCPSocket(int inSocket) : m_socket(inSocket)
    {}

    int m_socket = -1;
    char *m_ip = const_cast<char *>("unknown");
    uint16_t m_port = 0;
};

typedef std::shared_ptr<TCPSocket> TCPSocketPtr;

enum SocketAddressFamily
{
    INET = AF_INET,
    INET6 = AF_INET6
};

class NetHelper
{


public:
    static SocketAddressPtr CreateIPV4FromString(const string &s);

    static UDPSocketPtr CreateUDPSocket(SocketAddressFamily addressFamily);

    static TCPSocketPtr CreateTCPSocket(SocketAddressFamily addressFamily);

    static fd_set *FillSetFromVector(fd_set &outSet, const vector<TCPSocketPtr> *inSockets);    //inSockets ==> outSet

    static void FillVectorFromSet(vector<TCPSocketPtr> *outSockets, const vector<TCPSocketPtr> *inSockets,
                                  const fd_set &inSet); //inSockets (which is in inSet) ==> outSockets

    static int Select(const vector<TCPSocketPtr> *inReadSet, vector<TCPSocketPtr> *outReadSet,
                      const vector<TCPSocketPtr> *inWriteSet, vector<TCPSocketPtr> *outWriteSet,
                      const vector<TCPSocketPtr> *inExceptSet, vector<TCPSocketPtr> *outExceptSet,
                      const timeval *_timeout);
};


#endif //RSYNCTOOL_NETHELPER_H
