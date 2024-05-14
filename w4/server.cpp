#include <enet/enet.h>
#include <iostream>
#include "entity.h"
#include "protocol.h"
#include <stdlib.h>
#include <map>

static std::map<uint16_t, Entity> entities;
static std::map<uint16_t, std::pair<std::string, int>> players;
static std::map<uint16_t, ENetPeer*> controlledMap;
static std::map<uint16_t, float> invulnerableTime;

std::string gen_name()
{
    std::string name = "";
    for (int i = 0; i < 5; i++)
    {
        name += ('a' + rand() % 26);
    }
    return name;
}

static uint16_t create_random_entity()
{
  uint16_t newEid = entities.size();
  uint32_t color = (int((rand() % 120 + 120)) << 24) + (int((rand() % 120 + 120)) << 16) + (int(rand() % 120 + 120) << 8) + 255;
  float x = (rand() % 40 - 20) * 15.f;
  float y = (rand() % 40 - 20) * 15.f;
  Entity ent = {color, x, y, newEid, false, 0.f, 0.f, 5.f};
  entities.emplace(newEid, ent);
  invulnerableTime[newEid] = 5.f;
  return newEid;
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  std::string name = gen_name();
  printf("New player %s\n", name.c_str());
  // send all entities
  for (const auto& [k, ent] : entities)
    send_new_entity(peer, ent, players[k].first);

  // find max eid
  uint16_t newEid = create_random_entity();
  const Entity& ent = entities[newEid];

  controlledMap[newEid] = peer;
  players[newEid] = std::make_pair(name, 0);

  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
  {
    send_new_entity(&host->peers[i], ent, name);
    send_update(&host->peers[i], newEid, ent.radius, 0.f);
  }
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

void on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f;
  float r = 0.f;
  deserialize_entity_state(packet, eid, x, y);

  auto &e = entities.find(eid)->second;
  e.x = x;
  e.y = y;
}

float dist2(Entity e1, Entity e2)
{
    return (e1.x - e2.x) * (e1.x - e2.x) + (e1.y - e2.y) * (e1.y - e2.y);
}

int main(int argc, const char **argv)
{
  srand(time(nullptr));

  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  constexpr int numAi = 10;

  for (int i = 0; i < numAi; ++i)
  {
    uint16_t eid = create_random_entity();
    entities[eid].serverControlled = true;
    controlledMap[eid] = nullptr;
    players[eid] = std::make_pair("AI", 0);
  }

  uint32_t lastTime = enet_time_get();
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
          case E_CLIENT_TO_SERVER_JOIN:
            on_join(event.packet, event.peer, server);
            break;
          case E_CLIENT_TO_SERVER_STATE:
            on_state(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }



    for (auto& [k, e] : entities)
    {
      for (auto& [k1, e1] : entities)
      {
          if (k1 != k && dist2(e, e1) < (e.radius + e1.radius) * (e.radius + e1.radius))
          {
              auto k_min = e.radius < e1.radius ? k : k1;
              auto k_max = e.radius < e1.radius ? k1 : k;

              if (invulnerableTime[k_min] > 0.f)
              {
				  continue;
			  }
              else
              {
                  invulnerableTime[k_min] = 5.f;
              }

              entities[k_max].radius += entities[k_min].radius / 2;
              players[k_max].second += entities[k_min].radius / 2;

              entities[k_min].radius /= 2;
              entities[k_min].x = (rand() % 40 - 20) * 5.f;
              entities[k_min].y = (rand() % 40 - 20) * 5.f;

              for (size_t i = 0; i < server->peerCount; ++i)
              {
                  ENetPeer* peer = &server->peers[i];
                  send_update(peer, entities[k_min].eid, entities[k_min].radius, players[k_min].second);
                  send_update(peer, entities[k_max].eid, entities[k_max].radius, players[k_max].second);
              }
		  }
      }

      if (e.serverControlled)
      {
        const float diffX = e.targetX - e.x;
        const float diffY = e.targetY - e.y;
        const float dirX = diffX > 0.f ? 1.f : -1.f;
        const float dirY = diffY > 0.f ? 1.f : -1.f;
        constexpr float spd = 50.f;
        e.x += dirX * spd * dt;
        e.y += dirY * spd * dt;
        if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
        {
          e.targetX = (rand() % 40 - 20) * 15.f;
          e.targetY = (rand() % 40 - 20) * 15.f;
        }
      }

      for (size_t i = 0; i < server->peerCount; ++i)
      {
        ENetPeer *peer = &server->peers[i];
        if (controlledMap[e.eid] != peer)
          send_snapshot(peer, e.eid, e.x, e.y);
      }

      if (invulnerableTime[e.eid] > 0.f)
      {
		  invulnerableTime[e.eid] -= dt;
          if (invulnerableTime[e.eid] <= 0.f)
          {
			  invulnerableTime[e.eid] = 0.f;
		  }
	  }
    }

    //usleep(400000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


