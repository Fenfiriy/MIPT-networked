#include "protocol.h"
#include "bitstream.h"

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.writeRaw(E_CLIENT_TO_SERVER_JOIN);

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent, std::string &name)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity)
                                                   + sizeof(int)
                                                   + (int)name.size() * sizeof(char),
                                                   ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.writeRaw(ent);
  bs.writeRaw((int)name.size());

  for (int i = 0; i < (int)name.size(); ++i)
	bs.writeRaw(name[i]);

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

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   2 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data);
  bs.writeRaw(E_CLIENT_TO_SERVER_STATE);
  bs.writeRaw(eid);
  bs.writeRaw(x);
  bs.writeRaw(y);

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   2 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.writeRaw(eid);
  bs.writeRaw(x);
  bs.writeRaw(y);

  enet_peer_send(peer, 1, packet);
}

void send_update(ENetPeer* peer, uint16_t eid, float r, float points)
{
  ENetPacket* packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   2 * sizeof(float),
      	                                           ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_UPDATE);
  bs.writeRaw(eid);
  bs.writeRaw(r);
  bs.writeRaw(points);

  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent, std::string &name)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(ent);

  int len;
  bs.readRaw(len);
  for (int i = 0; i < len; ++i)
  {
    char c;
	bs.readRaw(c);
	name += c;
  }
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
  bs.readRaw(x);
  bs.readRaw(y);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
  bs.readRaw(x);
  bs.readRaw(y);
}

void deserialize_update(ENetPacket* packet, uint16_t& eid, float& r, float& points)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
  bs.readRaw(r);
  bs.readRaw(points);
}