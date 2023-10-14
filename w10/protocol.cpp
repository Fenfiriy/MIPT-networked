#include "protocol.h"
#include "quantisation.h"
#include "bitstream.h"
#include <cstring> // memcpy
#include <iostream>
#include <stdlib.h>
#include "server.h"

static uint32_t xorCipherKey = 0;

void send_join(ENetPeer *peer, uint8_t name_len, const char* name)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint8_t) + sizeof(char) * name_len,
      ENET_PACKET_FLAG_RELIABLE);
  Bitstream bs(packet->data);
  bs.writeRaw(E_CLIENT_TO_SERVER_JOIN);
  bs.writeRaw(name_len);
  for (int i = 0; i < name_len; i++)
      bs.writeRaw(name[i]);

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(ent),
                                                   ENET_PACKET_FLAG_RELIABLE);
  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.writeRaw(ent.eid);
  bs.writeRaw(ent.color);
  uint8_t len = ent.name.length();
  bs.writeRaw(len);

  for (int i = 0; i < len; i++)
      bs.writeRaw(ent.name[i]);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                                   ENET_PACKET_FLAG_RELIABLE);
  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY);
  bs.writeRaw(eid);

  enet_peer_send(peer, 0, packet);
}

void send_cipher_key(ENetPeer *peer, uint32_t key)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint32_t),
                                                   ENET_PACKET_FLAG_RELIABLE);
  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_KEY);
  bs.writeRaw(key);

  enet_peer_send(peer, 0, packet);
}

void fuzz_packet_data(ENetPacket *packet)
{
  packet->data[rand() % packet->dataLength] = (uint8_t)rand();
}

void send_entity_input(ENetPeer* peer, uint16_t eid, MoveDirection dir)
{
    ENetPacket* packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
        sizeof(MoveDirection),
        //sizeof(uint8_t),
        ENET_PACKET_FLAG_UNSEQUENCED);
    Bitstream bs(packet->data);
    bs.writeRaw(E_CLIENT_TO_SERVER_INPUT);
    bs.writeRaw(eid);
    bs.writeRaw(dir);

    //fuzz_packet_data(packet);
    cipher_data(packet);

    enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer* peer, Entity& ent)
{
    ENetPacket* packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
        sizeof(uint8_t) + sizeof(vec2int) + sizeof(MoveDirection),
        ENET_PACKET_FLAG_UNSEQUENCED);
    Bitstream bs(packet->data);
    bs.writeRaw(E_SERVER_TO_CLIENT_SNAPSHOT);
    bs.writeRaw(ent.eid);
    bs.writeRaw(ent.length);
    bs.writeRaw(ent.posHead);
    bs.writeRaw(ent.dir);

    enet_peer_send(peer, 1, packet);
}

void send_state(ENetPeer* peer, std::vector<std::vector<std::pair<uint16_t, uint8_t>>>& fs)
{
    ENetPacket* packet = enet_packet_create(nullptr, sizeof(uint8_t) +
        sizeof(std::pair<uint16_t, uint8_t>) * fs.size() * fs[0].size(),
        ENET_PACKET_FLAG_UNSEQUENCED);
    Bitstream bs(packet->data);
    bs.writeRaw(E_SERVER_TO_CLIENT_STATE);
    for (int i = 0; i < fs.size(); i++)
        for (int j = 0; j < fs[i].size(); j++)
            bs.writeRaw(fs[i][j]);


    enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  Bitstream bs(packet->data);
  MessageType mt;
  bs.readRaw(mt);
  bs.readRaw(ent.eid);
  bs.readRaw(ent.color);
  uint8_t len;
  bs.readRaw(len);
  std::string name = "";
  char c;
  for (int i = 0; i < len; i++)
  {
      bs.readRaw(c);
      name.push_back(c);
  }
  ent.name = name;
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
    Bitstream bs(packet->data);
    MessageType mt;
    bs.readRaw(mt);
    bs.readRaw(eid);
}

void xor_packet_data(ENetPacket *packet, uint8_t *key_ptr)
{
  uint8_t *ptr = packet->data; ptr += sizeof(uint8_t);
  uint8_t *end = packet->data + packet->dataLength;
  for (int i = 0; ptr < end; ++ptr, ++i)
  {
    i = i % 4;
    *ptr ^= key_ptr[i];
  }
}

void cipher_data(ENetPacket *packet)
{
  xor_packet_data(packet, (uint8_t*)&xorCipherKey);
}

void decipher_data(ENetPacket *packet, ENetPeer *peer)
{
  xor_packet_data(packet, (uint8_t*)peer->data);
}

void deserialize_entity_input(ENetPacket* packet, uint16_t& eid, MoveDirection& dir)
{
    Bitstream bs(packet->data);
    MessageType mt;
    bs.readRaw(mt);
    bs.readRaw(eid);
    bs.readRaw(dir);
}

void deserialize_snapshot(ENetPacket* packet, uint16_t& eid, uint8_t& len, vec2int&pos, MoveDirection& dir)
{
    Bitstream bs(packet->data);
    MessageType mt;
    bs.readRaw(mt);
    bs.readRaw(eid);
    bs.readRaw(len);
    bs.readRaw(pos);
    bs.readRaw(dir);
}
void deserialize_state(ENetPacket* packet, std::vector<std::vector<std::pair<uint16_t, uint8_t>>>& fs)
{
    Bitstream bs(packet->data);
    MessageType mt;
    bs.readRaw(mt);
    for (int x = 0; x < fs.size(); x++)
    {
        for (int y = 0; y < fs[x].size(); y++)
        {
            bs.readRaw(fs[x][y]);
        }
    }
}

void deserialize_and_set_key(ENetPacket *packet)
{
    Bitstream bs(packet->data);
    MessageType mt;
    bs.readRaw(mt);
    bs.readRaw(xorCipherKey);
}

