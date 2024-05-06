#include <enet/enet.h>
#include <iostream>
#include <string>
#include <vector>

const std::string LOBBY_NAME = "localhost";
const int LOBBY_PORT = 10887;
const int SERVER_PORT = 10888; // client should get this from the lobby, not from here

void broadcast_string(ENetHost* host, std::string msg, bool reliable = true)
{
    ENetPacket* packet = enet_packet_create(msg.c_str(), msg.size() + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(host, reliable, packet);
}

void send_string(ENetPeer* peer, std::string msg, bool reliable = true)
{
    ENetPacket* packet = enet_packet_create(msg.c_str(), msg.size() + 1, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, reliable, packet);
}