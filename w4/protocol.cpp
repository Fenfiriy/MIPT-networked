#include "protocol.h"
#include "bitstream.h"

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.writeRaw(E_CLIENT_TO_SERVER_JOIN);

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity),
                                                   ENET_PACKET_FLAG_RELIABLE);

  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.writeRaw(ent);

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

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y, float r)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   3 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data);
  bs.writeRaw(E_CLIENT_TO_SERVER_STATE);
  bs.writeRaw(eid);
  bs.writeRaw(x);
  bs.writeRaw(y);
  bs.writeRaw(r);

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float r)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   3 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);

  Bitstream bs(packet->data);
  bs.writeRaw(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.writeRaw(eid);
  bs.writeRaw(x);
  bs.writeRaw(y);
  bs.writeRaw(r);

  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(ent);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
  bs.readRaw(x);
  bs.readRaw(y);
  bs.readRaw(r);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r)
{
  Bitstream bs(packet->data);
  bs.skip<uint8_t>();
  bs.readRaw(eid);
  bs.readRaw(x);
  bs.readRaw(y);
  bs.readRaw(r);
}

