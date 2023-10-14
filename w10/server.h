#include <enet/enet.h>
#include <stdlib.h>
#include <map>

static std::vector<Entity> entities;
static std::vector<std::vector<std::pair<uint16_t, uint8_t>>> fieldState{};
static std::map<uint16_t, ENetPeer*> controlledMap;


void on_join(ENetPacket* packet, ENetPeer* peer, ENetHost* host);
void on_input(ENetPacket* packet);