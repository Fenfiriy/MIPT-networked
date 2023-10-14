#include <iostream>
#include "entity.h"
#include "protocol.h"
#include "mathUtils.h"
#include "server.h"
#include <random>

#include <chrono>
#include <thread>

#define FIELD_SIZE 32

#ifdef WIN32
void usleep(__int64 usec)
{
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
#endif

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t maxEid = entities.empty() ? 0 : entities[0].eid;
  for (const Entity &e : entities)
    maxEid = std::max(maxEid, e.eid);
  uint16_t newEid = maxEid + 1;
  uint32_t color = 0x440000ff * (rand() % 5) +
                   0x004400ff * (rand() % 5) +
                   0x000044ff * (rand() % 5) +
                   0x000044ff * (rand() % 5) +
                   0x000000ff;
  uint8_t x = (rand() % 4) * 4 + 10;
  uint8_t y = (rand() % 4) * 4 + 10;
  Entity ent = { color, newEid, vec2int{x, y} };
  entities.push_back(ent);

  controlledMap[newEid] = peer;

  printf("Assigned eid %u\n", newEid);

  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
    send_new_entity(&host->peers[i], ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
  uint32_t *keyPtr = (uint32_t*)peer->data;
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32_t> distrib(0);
  *keyPtr = distrib(gen);
  send_cipher_key(peer, *keyPtr);
}

void on_input(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  MoveDirection dir;
  deserialize_entity_input(packet, eid, dir);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
        printf("Input %d %d\n", e.dir, opposite(dir));
        if (e.dir != opposite(dir))
        {
            e.dir = dir;
        }
    }
}

int main(int argc, const char **argv)
{
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

  for (int i = 0; i < FIELD_SIZE; i++)
  {
      fieldState.push_back({});
      for (int j = 0; j < FIELD_SIZE; j++)
      {
          fieldState[i].push_back({ invalid_entity, 0 });
          printf("0 ");
      }

      printf("\n");
  }

  printf("Field %d x %d created\n", fieldState.size(), fieldState[0].size());

  uint32_t lastTime = enet_time_get();
  float tick = 0.f;
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    tick += dt;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        event.peer->data = new uint32_t;
        *(uint32_t*)event.peer->data = 0;
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        printf("Disconnected %x:%u \n", event.peer->address.host, event.peer->address.port);
        delete event.peer->data;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
          case E_CLIENT_TO_SERVER_JOIN:
            on_join(event.packet, event.peer, server);
            break;
          case E_CLIENT_TO_SERVER_INPUT:
            decipher_data(event.packet, event.peer);
            on_input(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    static int t = 0;
    std::vector<vec2int> free_spaces{};

    bool apple_exists = false;
    for (uint8_t x = 0; x < fieldState.size(); x++)
    {
        for (uint8_t y = 0; y < fieldState[x].size(); y++)
        {

            if (fieldState[x][y].first == invalid_entity)
            {
                free_spaces.push_back(vec2int{ x, y });
            }
            if (fieldState[x][y].first == server_entity)
            {
                apple_exists = true;
            }
        }
    }
    
    if (!apple_exists)
    {
        int sz = free_spaces.size();
        vec2int rand_pos = free_spaces[rand() % sz];
        fieldState[rand_pos.x][rand_pos.y].first = server_entity;
    }

    for (Entity& e : entities)
    {
        for (int x = 0; x < fieldState.size(); x++)
        {
            for (int y = 0; y < fieldState[x].size(); y++)
            {

                if (fieldState[x][y].first == e.eid)
                {
                    fieldState[x][y].second++;
                    if (fieldState[x][y].second >= e.length)
                    {
                        fieldState[x][y].first = invalid_entity;
                    }
                }
            }
        }



        //printf("Entity %i in pos %i %i\n", e.eid, e.posHead.x, e.posHead.y);
        vec2int p = move(e.posHead, e.dir);
        if (p.x >= FIELD_SIZE || p.y >= FIELD_SIZE)
            respawn(e);
        else
        {
            simulate_entity(e, fieldState[p.x][p.y]);
            fieldState[p.x][p.y].first = e.eid;
            fieldState[p.x][p.y].second = 0;
        }

        // send
        for (size_t i = 0; i < server->peerCount; ++i)
        {
            ENetPeer* peer = &server->peers[i];
            // skip this here in this implementation
            //if (controlledMap[e.eid] != peer)
            send_snapshot(peer, e);
        }
    }



    // send
    for (size_t i = 0; i < server->peerCount; ++i)
    {
        ENetPeer* peer = &server->peers[i];
        // skip this here in this implementation
        //if (controlledMap[e.eid] != peer)
        send_state(peer, fieldState);
    }
    usleep(500000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


