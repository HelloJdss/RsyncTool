//
// Created by carrot on 18-3-15.
//
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include "NetHelper.h"

SocketAddress::SocketAddress(uint32_t inAddr, uint16_t inPort)
{
    bzero(GetAsSockAddrIn(), sizeof(*GetAsSockAddrIn()));
    GetAsSockAddrIn()->sin_family = AF_INET;
    GetAsSockAddrIn()->sin_addr.s_addr = htonl(inAddr);
    GetAsSockAddrIn()->sin_port = htons(inPort);
}

SocketAddress::SocketAddress(const char *inAddr, uint16_t inPort)
{
    bzero(GetAsSockAddrIn(), sizeof(*GetAsSockAddrIn()));
    GetAsSockAddrIn()->sin_family = AF_INET;
    GetAsSockAddrIn()->sin_addr.s_addr = inet_addr(inAddr);
    GetAsSockAddrIn()->sin_port = htons(inPort);
}


SocketAddressPtr NetHelper::CreateIPV4FromString(const string &s)
{
    auto pos = s.find_last_of(':');
    string host, port;
    if (pos != string::npos)
    {
        host = s.substr(0, pos);
        port = s.substr(pos + 1);
    }
    else
    {
        host = s;
        port = "0";
    }

    addrinfo hint;
    memset(&hint, 0, sizeof(hint));

    hint.ai_family = AF_INET;

    addrinfo *result;
    int error = getaddrinfo(host.c_str(), port.c_str(), &hint, &result);
    if (error != 0 && result != nullptr)
    {
        freeaddrinfo(result);
        return nullptr;
    }

    while (!result->ai_addr && result->ai_next)
    {
        result = result->ai_next;
    }

    if (!result->ai_addr)
    {
        freeaddrinfo(result);
        return nullptr;
    }

    auto toRet = std::make_shared<SocketAddress>(*result->ai_addr);

    freeaddrinfo(result);
    return SocketAddressPtr();
}

UDPSocketPtr NetHelper::CreateUDPSocket(SocketAddressFamily addressFamily)
{
    int s = socket(addressFamily, SOCK_DGRAM, IPPROTO_UDP);
    if (s != -1)
    {
        return UDPSocketPtr(new UDPSocket(s));
    }
    else
    {
        return nullptr;
    }
}

TCPSocketPtr NetHelper::CreateTCPSocket(SocketAddressFamily addressFamily)
{
    int s = socket(addressFamily, SOCK_STREAM, IPPROTO_TCP);
    if (s != -1)
    {
        return TCPSocketPtr(new TCPSocket(s));
    }
    else
    {
        return nullptr;
    }
}

fd_set *NetHelper::FillSetFromVector(fd_set &outSet, const vector<TCPSocketPtr> *inSockets)
{
    if (inSockets)
    {
        FD_ZERO(&outSet);
        for (const TCPSocketPtr &pSocket : *inSockets)
        {
            FD_SET(pSocket->m_socket, &outSet);
        }
        return &outSet;
    }
    return nullptr;
}

void NetHelper::FillVectorFromSet(vector<TCPSocketPtr> *outSockets, const vector<TCPSocketPtr> *inSockets,
                                  const fd_set &inSet)
{
    if (inSockets && outSockets)
    {
        outSockets->clear();
        for (const TCPSocketPtr &socket : *inSockets)
        {
            if (FD_ISSET(socket->m_socket, &inSet))
            {
                outSockets->push_back(socket);
            }
        }
    }
}

int NetHelper::Select(const vector<TCPSocketPtr> *inReadSet, vector<TCPSocketPtr> *outReadSet,
                      const vector<TCPSocketPtr> *inWriteSet, vector<TCPSocketPtr> *outWriteSet,
                      const vector<TCPSocketPtr> *inExceptSet, vector<TCPSocketPtr> *outExceptSet,
                      const timeval *_timeout)
{
    fd_set _read, _write, _except;

    fd_set *read_ptr = FillSetFromVector(_read, inReadSet);
    fd_set *write_ptr = FillSetFromVector(_write, inWriteSet);
    fd_set *except_ptr = FillSetFromVector(_except, inExceptSet);

    timeval *tm_ptr = nullptr;
    timeval _tm;
    if (_timeout)
    {
        _tm = *_timeout;
        tm_ptr = &_tm;
    }

    int toRet = select(FD_SETSIZE, read_ptr, write_ptr, except_ptr, tm_ptr); //return 0 if timeout

    if (toRet > 0)
    {
        FillVectorFromSet(outReadSet, inReadSet, _read);
        FillVectorFromSet(outWriteSet, inWriteSet, _write);
        FillVectorFromSet(outExceptSet, inExceptSet, _except);
    }
    else if (toRet < 0)
    {
#ifdef LOG_ERROR
        LOG_ERROR("%s", strerror(errno));
#endif
    }
    return toRet;
}


UDPSocket::~UDPSocket()
{
#ifdef LOG_TRACE
    LOG_TRACE("~UDPSocket");
#endif
    Close();
}

int UDPSocket::Bind(SocketAddress &inSrcAddr)
{
    int err = bind(m_socket, &inSrcAddr.m_sockaddr, static_cast<socklen_t>(inSrcAddr.GetSize()));
    if (err != 0)
    {
        return errno;
    }

    if (inSrcAddr.m_sockaddr.sa_family == INET)
    {
        m_ip = inet_ntoa(inSrcAddr.GetAsSockAddrIn()->sin_addr);
        m_port = ntohs(inSrcAddr.GetAsSockAddrIn()->sin_port);
    }
    return NO_ERROR;
}

int UDPSocket::SendTo(const void *inData, int inLen, const SocketAddress &inTo)
{
    auto bytes = sendto(m_socket, inData, inLen, 0, &inTo.m_sockaddr, static_cast<socklen_t>(inTo.GetSize()));

    if (bytes > 0)
    {
        return bytes;
    }
    else
    {
        throw errno; //throw exception
    }
}

int UDPSocket::RecvFrom(void *inBuffer, int inLen, SocketAddress &outFrom)
{
    auto fLen = outFrom.GetSize();
    auto bytes = recvfrom(m_socket, inBuffer, inLen, 0, &outFrom.m_sockaddr, reinterpret_cast<socklen_t *>(&fLen));
    if (bytes >= 0)
    {
        return bytes;
    }
    else
    {
        throw errno; //throw exception
    }
}

string UDPSocket::GetEndPoint() const
{
    string s(m_ip);
    std::stringstream stream;
    stream << m_port;
    return s.append(":").append(stream.str());
}

inline void UDPSocket::Close()
{
    if (m_socket != -1)
    {
        close(m_socket);
        m_socket = -1;
    }
}

TCPSocket::~TCPSocket()
{
#ifdef LOG_TRACE
    LOG_TRACE("~TCPSocket");
#endif
    Close();
}

void TCPSocket::Connect(const SocketAddress &inAddr)
{
    int err = connect(m_socket, &inAddr.m_sockaddr, inAddr.GetSize());
    if (err < 0)
    {
        throw errno; //throw exception
    }
}

int TCPSocket::Bind(SocketAddress &inSrcAddr)
{
    int err = bind(m_socket, &inSrcAddr.m_sockaddr, static_cast<socklen_t>(inSrcAddr.GetSize()));
    if (err != 0)
    {
        return errno;
    }

    if (inSrcAddr.m_sockaddr.sa_family == INET)
    {
        m_ip = inet_ntoa(inSrcAddr.GetAsSockAddrIn()->sin_addr);
        m_port = ntohs(inSrcAddr.GetAsSockAddrIn()->sin_port);
    }
    return NO_ERROR;
}

void TCPSocket::Listen(int inBackLog)
{
    int err = listen(m_socket, inBackLog);
    if (err < 0)
    {
        throw errno; //throw exception
    }
}

std::shared_ptr<TCPSocket> TCPSocket::Accept(SocketAddress &inFromAddr)
{
    socklen_t Length = inFromAddr.GetSize();
    int newSocket = accept(m_socket, &inFromAddr.m_sockaddr, &Length);
    if (newSocket != -1)
    {
        TCPSocketPtr socketPtr = TCPSocketPtr(new TCPSocket(newSocket));
        if (inFromAddr.m_sockaddr.sa_family == INET)
        {
            sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(&inFromAddr.m_sockaddr);
            socketPtr->m_ip = inet_ntoa(addr->sin_addr);
            socketPtr->m_port = ntohs(addr->sin_port);
        }
        return socketPtr;
    }
    else
    {
        return nullptr;
    }
}

int TCPSocket::Send(const void *inData, int inLen)
{
    auto bytes = send(m_socket, inData, inLen, 0);
    if (bytes < 0)
    {
        throw errno; //throw exception
    }
    else
    {
        return bytes;
    }
}

int TCPSocket::Receive(void *inBuffer, int inLen)
{
    auto bytes = recv(m_socket, inBuffer, inLen, 0);
    if (bytes >= 0)
    {
        return bytes;
    }
    else
    {
        throw errno; //throw exception
    }
}

string TCPSocket::GetEndPoint() const
{
    string s;
    std::stringstream stream;
    stream << m_port;
    return s.append(m_ip).append(":").append(stream.str());
}

inline void TCPSocket::Close()
{
    if (m_socket != -1)
    {
        close(m_socket);
        m_socket = -1;
    }
}
