#include "common.h"

std::string gen_name()
{
    std::string name = "";
    for (int i = 0; i < 5; i++)
    {
        name += ('a' + rand() % 26);
    }
    return name;
}

int main(int argc, const char** argv)
{
    if (enet_initialize() != 0)
    {
        printf("Cannot init ENet");
        return 1;
    }
    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = SERVER_PORT;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

    if (!server)
    {
        printf("Cannot create ENet server\n");
        return 1;
    }

    uint32_t timeStart = enet_time_get();
    uint32_t lastPingTime = timeStart;

    std::vector<std::string> players = {};

    while (true)
    {
        ENetEvent event;
        while (enet_host_service(server, &event, 10) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
                players.push_back(gen_name());
                send_string(event.peer, "1: Your credentials\n" + std::to_string(players.size() - 1) + " " + players.back());
                std::string list = "2: List of players\n";
                list += std::to_string(players.size());
                for (int i = 0; i < players.size(); i++)
                {
                    list += "\n" + std::to_string(i) + " " + players[i];
                }
                send_string(event.peer, list);
                broadcast_string(server, "3: Player connected\n" + std::to_string(players.size() - 1) + " " + players.back());
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::string msg((char*)event.packet->data);
                std::string code = msg.substr(0, msg.find(": "));
                printf("Received packet type %s\n", msg.c_str());
                msg = msg.substr(msg.find("\n") + 1);
                switch (atoi(code.c_str()))
                {
                case 1:

                    break;
                default:
                    break;
                }
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Connection with %x:%u lost\n", event.peer->address.host, event.peer->address.port);
                break;
            default:
                break;
            };
        }

        uint32_t curTime = enet_time_get();
        if (curTime - lastPingTime > 1000)
        {
            lastPingTime = curTime;
            std::string ping_msg = "4: Pings";
            for (int i = 0; i < server->connectedPeers; i++)
            {
                ping_msg += "\n" + std::to_string(i) + " " + std::to_string(server->peers[i].roundTripTime);
            }
            broadcast_string(server, ping_msg, false);
        }
        
    }

    enet_host_destroy(server);

    atexit(enet_deinitialize);
    return 0;
}