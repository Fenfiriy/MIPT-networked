// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include <functional>
#include "raylib.h"
#include <enet/enet.h>
#include <math.h>
#include <map>
#include <string>
#include <vector>
#include "entity.h"
#include "protocol.h"

#define FIELD_SIZE 32

static std::vector<Entity> entities;
static std::vector<std::vector<std::pair<uint16_t, uint8_t>>> fieldState{};
static uint16_t my_entity = invalid_entity;

void on_new_entity_packet(ENetPacket *packet)
{
  Entity newEntity;
  deserialize_new_entity(packet, newEntity);
  // TODO: Direct adressing, of course!
  for (const Entity &e : entities)
    if (e.eid == newEntity.eid)
      return; // don't need to do anything, we already have entity
  entities.push_back(newEntity);
}

void on_set_controlled_entity(ENetPacket *packet)
{
  deserialize_set_controlled_entity(packet, my_entity);
}

void on_snapshot(ENetPacket* packet)
{
    uint16_t eid = invalid_entity;
    uint8_t len = 0;
    MoveDirection dir;
    vec2int pos;
    deserialize_snapshot(packet, eid, len, pos, dir);
    // TODO: Direct adressing, of course!
    for (Entity& e : entities)
        if (e.eid == eid)
        {
            e.dir = dir;
            e.posHead = pos;
            e.length = len;
        }
}

void on_key(ENetPacket *packet)
{
  deserialize_and_set_key(packet);
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  printf("Insert name:\n");
  char tmp[201];
  scanf("%200s", tmp);
  std::string name = tmp;

  ENetHost *client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10131;

  ENetPeer *serverPeer = enet_host_connect(client, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }

  int width = 600;
  int height = 600;

  for (int i = 0; i < FIELD_SIZE; i++)
  {
      fieldState.push_back({});
      for (int j = 0; j < FIELD_SIZE; j++)
      {
          fieldState[i].push_back({ invalid_entity, 0 });
      }
  }


  InitWindow(width, height, "w10 networked MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  Camera2D camera = { {0, 0}, {0, 0}, 0.f, 1.f };
  camera.target = Vector2{ 0.f, 0.f };
  camera.offset = Vector2{ 0.f, 0.f };
  camera.rotation = 0.f;
  camera.zoom = 10.f;


  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

  bool connected = false;
  while (!WindowShouldClose())
  {
    float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        send_join(serverPeer, name.length(), name.c_str());
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
        case E_SERVER_TO_CLIENT_NEW_ENTITY:
          on_new_entity_packet(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
          on_set_controlled_entity(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SNAPSHOT:
            on_snapshot(event.packet);
            break;
        case E_SERVER_TO_CLIENT_STATE:
            deserialize_state(event.packet, fieldState);
            break;
        case E_SERVER_TO_CLIENT_KEY:
          on_key(event.packet);
          break;
        };
        break;
      default:
        break;
      };
    }
    if (my_entity != invalid_entity)
    {
      bool left = IsKeyDown(KEY_LEFT);
      bool right = IsKeyDown(KEY_RIGHT);
      bool up = IsKeyDown(KEY_UP);
      bool down = IsKeyDown(KEY_DOWN);
      // TODO: Direct adressing, of course!
      for (Entity &e : entities)
        if (e.eid == my_entity)
        {
            // Update
            int y = (up ? 1 : 0) + (down ? -1 : 0);
            int x = (left ? -1 : 0) + (right ? 1 : 0);
            // Send
            if (e.dir == UP || e.dir == DOWN)
            {
                if (x > 0)
                {
                    send_entity_input(serverPeer, my_entity, RIGHT);
                }
                if (x < 0)
                {
                    send_entity_input(serverPeer, my_entity, LEFT);
                }
            }                
            else
            {
                if (y > 0)
                {
                    send_entity_input(serverPeer, my_entity, UP);
                }
                if (y < 0)
                {
                    send_entity_input(serverPeer, my_entity, DOWN);
                }
            }
        }
    }

    BeginDrawing();
      ClearBackground(GRAY);
      BeginMode2D(camera);
        DrawRectangleLines(0, 0, FIELD_SIZE, FIELD_SIZE, GetColor(0xff00ffff));

        std::map<uint16_t, Color> colors{};
        int k = 0;
        colors[server_entity] = GREEN;
        for (const Entity& e : entities)
        {
            colors[e.eid] = GetColor(e.color);
            DrawText((e.name + " " + std::to_string(e.length)).c_str(), FIELD_SIZE + 2, 8 * k + 2, 1, GetColor(e.color));
            k++;
        }

        for (int x = 0; x < fieldState.size(); x++)
        {
            for (int y = 0; y < fieldState[x].size(); y++)
            {
                if (fieldState[x][y].first != invalid_entity)
                {
                    DrawCircle(x, y, 0.5f, colors[fieldState[x][y].first]);
                }
            }
        }

      EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
