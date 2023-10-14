#pragma once
#include <enet/enet.h>
#include <cstdint>
#include "entity.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_INPUT,
  E_SERVER_TO_CLIENT_SNAPSHOT,
  E_SERVER_TO_CLIENT_STATE,
  E_SERVER_TO_CLIENT_KEY
};

void send_join(ENetPeer *peer);
void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_set_controlled_entity(ENetPeer *peer, uint16_t eid);
void send_cipher_key(ENetPeer *peer, uint32_t key);
void send_entity_input(ENetPeer* peer, uint16_t eid, MoveDirection dir);
void send_snapshot(ENetPeer* peer, Entity& ent);
void send_state(ENetPeer* peer, std::vector<std::vector<std::pair<uint16_t, uint8_t>>>& fs);

MessageType get_packet_type(ENetPacket *packet);

void deserialize_new_entity(ENetPacket *packet, Entity &ent);
void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid);
void deserialize_entity_input(ENetPacket* packet, uint16_t& eid, MoveDirection& dir);
void deserialize_snapshot(ENetPacket* packet, uint16_t& eid, uint8_t& len, vec2int& pos, MoveDirection& dir);
void deserialize_state(ENetPacket* packet, std::vector<std::vector<std::pair<uint16_t, uint8_t>>>& fs);
void deserialize_and_set_key(ENetPacket *packet);

void cipher_data(ENetPacket *packet);
void decipher_data(ENetPacket *packet, ENetPeer *peer);

