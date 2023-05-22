#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "socket_tools.h"
#include "message.h"

int main(int argc, const char **argv)
{
    const char* port_i = "2022";
    const char* port_o = "2023";

    uint32_t lastID = 1;

    addrinfo resAddrInfo;
    int sfd_o = create_dgram_socket("localhost", port_o, &resAddrInfo);
    int sfd_i = create_dgram_socket(nullptr, port_i, nullptr);

    if (sfd_o == -1 || sfd_i == -1)
    {
        printf("Cannot create a socket\n");
        return 1;
    }
  printf("listening!\n");

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd_i, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd_i + 1, &readSet, NULL, NULL, &timeout);

    if (FD_ISSET(sfd_i, &readSet))
    {
        Message msg = get_msg(sfd_i);
        switch (msg.msgType)
        {
        case KEEPALIVE:
        {
            std::cout << "Client " << msg.clientId << " is staying alive" << "\n";
            break; 
        }
        case INIT:
        {
            std::cout << "Connection request " << msg.clientId << "\n";
            Message init_resp{ INIT, lastID };
            ssize_t init_res = send_msg(sfd_o, resAddrInfo, &init_resp);
            std::cout << "Client " << lastID << " created" << "\n";
            lastID++;
            if (init_res == -1)
            {
                std::cout << strerror(errno) << std::endl;
            }
            break;
        }
        case DATA:
        {
            std::cout << "Client " << msg.clientId << ": " << msg.message << "\n";
            Message resp{ DATA, msg.clientId, msg.message + 1 };
            ssize_t res = send_msg(sfd_o, resAddrInfo, &resp);
            if (res == -1)
            {
                std::cout << strerror(errno) << std::endl;
            }
            break;
        }
        }

    }
  }
  return 0;
}
