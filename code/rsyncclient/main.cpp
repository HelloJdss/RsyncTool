//
// Created by carrot on 18-3-15.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <cstdio>   //perror
#include <cstdlib> //exitsend sendto
#include <unistd.h> //write read
#include <iostream>
#include <cstring>
#include "MsgHelper.h"
#include "MD5.h"

int main(int argc, char *crgv[])
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;

    std::string str = "123456";
    MD5 md5 = MD5(str);
    std::string hashStr = md5.hexdigest();
    std::cout << hashStr << std::endl;

    /*sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
    address.sin_port = htons(52077);
    len = sizeof(address);
    result = connect(sockfd, reinterpret_cast<const sockaddr *>(&address), len);
    if (result == -1)
    {
        perror("oops: client");
        exit(1);
    }
    char g[100];
    bzero(g, 100);
    while (std::cin >> g)
    {
        auto bytes = MsgHelper::CreateBytes(g, strlen(g) + 1);
        ST_PackageHeader a;
        a.op = 1;
        auto data = MsgHelper::PackageData(a, bytes);
        send(sockfd, data->ToChars(), data->Size(), 0);
        char buff[100];
        recv(sockfd, &buff, 100, 0);
        printf("char from server %s", buff);
        bzero(g, 100);
    }
    close(sockfd);*/
    return 0;
}