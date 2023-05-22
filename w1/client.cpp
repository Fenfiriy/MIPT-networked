#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include "socket_tools.h"
#include "message.h"

std::mutex m;
uint32_t thisClientID = 0;

uint32_t ClientID(uint32_t id = 0)
{
    m.lock();
    if (id == 0)
    {
        m.unlock();
        return thisClientID;
    }
    else
    {
        thisClientID = id;
        m.unlock();
        return thisClientID;
    }
}

void init(int sfd, addrinfo r_addr)
{
    std::cout << "Trying to connect" << std::endl;
    Message msg{ INIT, ClientID()};
    ssize_t res = send_msg(sfd, r_addr, &msg);

    if (res == -1)
    {
        std::cout << strerror(errno) << std::endl;
    }
}

void server_listen(int sfd)
{
    while (true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sfd, &readSet);

        timeval timeout = { 0, 100000 }; // 100 ms
        select(sfd + 1, &readSet, NULL, NULL, &timeout);

        if (FD_ISSET(sfd, &readSet))
        {
            Message msg = get_msg(sfd);
            std::cout << msg.msgType << " " << msg.clientId << "\n";
            switch (msg.msgType)
            {
            case INIT:
                std::cout << "Successful connection to server, ID " << msg.clientId << "\n";
                if (ClientID() == 0)
                {
                    ClientID(msg.clientId);
                }
                break;
            case DATA:
                std::cout << "Server: " << msg.message << "\n";
                break;
            }
        }
    }
}

void keep_alive(int sfd, addrinfo r_addr)
{
    Message msg{ KEEPALIVE, ClientID() };
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        ssize_t res = send_msg(sfd, r_addr, &msg);
        
        if (res == -1)
        {
            std::cout << strerror(errno) << std::endl;
        }

        std::cout << "Staying alive" << std::endl;

    }
}

int main(int argc, const char **argv)
{
    const char* port_o = "2022";
    const char* port_i = "2023";

  addrinfo resAddrInfo;
  int sfd_o = create_dgram_socket("localhost", port_o, &resAddrInfo);
  int sfd_i = create_dgram_socket(nullptr, port_i, nullptr);


  if (sfd_o == -1 || sfd_i == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }

  std::thread keepAliveThread(keep_alive, sfd_o, resAddrInfo);
  std::thread listenThread(server_listen, sfd_i);
  keepAliveThread.detach();
  listenThread.detach();
  
  while (true)
  {
    std::string input;
    printf(">");
    std::getline(std::cin, input);
    if (input == "init")
    {
        init(sfd_o, resAddrInfo);
    }
    else
    {
        Message msg{ DATA, thisClientID, std::stoi(input) };
        ssize_t res = send_msg(sfd_o, resAddrInfo, &msg);
        if (res == -1)
            std::cout << strerror(errno) << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  return 0;
}
