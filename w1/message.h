#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "socket_tools.h"

enum MsgType : uint32_t
{
    INIT,
    DATA,
    KEEPALIVE
};

struct Message
{
    MsgType msgType;
    uint32_t clientId;
    uint32_t message;
};

ssize_t send_msg(int sfd, addrinfo addrInfo, Message* msg)
{
    size_t msgSize;
    switch (msg->msgType)
    {
    case INIT:
    case KEEPALIVE:
        msgSize = sizeof(uint32_t) * 2;
        break;
    default:
        msgSize = sizeof(uint32_t) * 3;
        break;
    }
    ssize_t res = sendto(sfd, msg, msgSize, 0, addrInfo.ai_addr, addrInfo.ai_addrlen);

    if (res == -1)
        std::cout << strerror(errno) << std::endl;

    return res;
}

Message get_msg(int sfd)
{
    Message msg;

    ssize_t numBytes = recvfrom(sfd, &msg, sizeof(Message), 0, nullptr, nullptr);

    return msg;
}