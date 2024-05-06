#include "raylib.h"
#include "common.h"

int main(int argc, const char **argv)
{
  int width = 800;
  int height = 600;
  InitWindow(width, height, "w6 AI MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
	width = std::min(scrWidth, width);
	height = std::min(scrHeight - 150, height);
	SetWindowSize(width, height);
  }

  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

  if (enet_initialize() != 0)
  {
	printf("Cannot init ENet");
	return 1;
  }

  ENetHost *client = enet_host_create(nullptr, 2, 3, 0, 0);
  if (!client)
  {
	printf("Cannot create ENet client\n");
	return 1;
  }

  ENetAddress lobbyAddress;
  enet_address_set_host(&lobbyAddress, LOBBY_NAME.c_str());
  lobbyAddress.port = LOBBY_PORT;

  ENetPeer *lobbyPeer = enet_host_connect(client, &lobbyAddress, 2, 0);
  if (!lobbyPeer)
  {
	printf("Cannot connect to lobby");
	return 1;
  }

  bool connected_lobby = false;
  bool connected_server = false;
  float posx = 0.f;
  float posy = 0.f;
  int player_id = -1;
  std::string player_name = "";


  std::vector<std::pair<std::string, int>> players = {};

  ENetAddress serverAddress;
  ENetPeer* serverPeer;

  std::string status = "Connecting to lobby...";

  while (!WindowShouldClose())
  {
	const float dt = GetFrameTime();
	ENetEvent event;
	while (enet_host_service(client, &event, 10) > 0)
	{
	  switch (event.type)
	  {
	  case ENET_EVENT_TYPE_CONNECT:
		printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
		if (event.peer->address.port == LOBBY_PORT)
		{
			status = "Connected to lobby, press ENTER to start";
			connected_lobby = true;
		}
		else
		{
			status = "Connected to server";
			connected_server = true;
		}
		break;
	  case ENET_EVENT_TYPE_RECEIVE:
	  {
		std::string msg((char*)event.packet->data);
		std::string code = msg.substr(0, msg.find(": "));
		printf("Received packet type %s\n", msg.c_str());
		msg = msg.substr(msg.find("\n") + 1);
		switch (atoi(code.c_str()))
		{
		case 0:
			enet_address_set_host(&serverAddress, msg.substr(0, msg.find(':')).c_str());
			serverAddress.port = atoi(msg.substr(msg.find(':') + 1).c_str());
			serverPeer = enet_host_connect(client, &serverAddress, 2, 0);

			if (!serverPeer)
			{
				printf("Cannot connect to server");
				status = "Cannot connect to server, try again later";
			}
			break;
		case 1:
			player_id = atoi(msg.substr(0, msg.find(' ')).c_str());
			player_name = msg.substr(msg.find(' ') + 1);
			break;
		case 2:
		{
			int players_count = atoi(msg.substr(0, msg.find('\n')).c_str());
			msg = msg.substr(msg.find('\n') + 1);
			for (int i = 0; i < players_count; i++)
			{
				int id = atoi(msg.substr(0, msg.find(' ')).c_str());
				msg = msg.substr(msg.find(' ') + 1);
				std::string name = msg.substr(0, msg.find('\n'));
				msg = msg.substr(msg.find('\n') + 1);
				
				players.push_back({name, 0});
				if (players.size() - 1 != id)
				{
					printf("We casually broke ids, no biggie\n");
				}
			}
			break;
		}
		case 3:
		{
			int id = atoi(msg.substr(0, msg.find(' ')).c_str());
			std::string name = msg.substr(msg.find(' ') + 1);

			
			if (id >= players.size())
			{
				players.push_back({ name, 0 });
			}
			break;
		}
		case 4:
		{
			for (int i = 0; i < players.size(); i++)
			{
				int id = atoi(msg.substr(0, msg.find(' ')).c_str());
				int ping = atoi(msg.substr(msg.find(' ') + 1).c_str());
				players[id].second = ping;
			}
			break;
		}
		default:
			break;
		}
		enet_packet_destroy(event.packet);
		break;
	  }
	  default:
		break;
	  };
	}
	if (connected_lobby && !connected_server && IsKeyDown(KEY_ENTER))
	{
		send_string(lobbyPeer, "0: Start game\n");
		status = "Connecting to server...";
	}

	if (connected_server)
	{	  
	  bool left = IsKeyDown(KEY_LEFT);
	  bool right = IsKeyDown(KEY_RIGHT);
	  bool up = IsKeyDown(KEY_UP);
	  bool down = IsKeyDown(KEY_DOWN);
	  constexpr float spd = 10.f;
	  posx += ((left ? -1.f : 0.f) + (right ? 1.f : 0.f)) * dt * spd;
	  posy += ((up ? -1.f : 0.f) + (down ? 1.f : 0.f)) * dt * spd;
	  send_string(serverPeer, TextFormat("1: Position\n%f %f", posx, posy));
	}

	int row = 0;

	BeginDrawing();
	  ClearBackground(BLACK);
	  DrawText(TextFormat("Current status: %s", status.c_str()), 20, 20 * ++row, 20, WHITE);
	  if (connected_server)
	  {
		  DrawText(TextFormat("My position: (%d, %d)", (int)posx, (int)posy), 20, 20 * ++row, 20, WHITE);
		  DrawText("List of players:", 20, 20 * ++row, 20, WHITE);
		  DrawText("ID", 20, 20 * ++row, 20, WHITE);
		  DrawText("Name", 100, 20 * row, 20, WHITE);
		  DrawText("Ping", 280, 20 * row, 20, WHITE);
		  for (int i = 0; i < players.size(); i++)
		  {
			  DrawText(TextFormat("%d", i), 20, 20 * ++row, 20, WHITE);
			  DrawText((players[i].first + (i == player_id ? " (you)" : "")).c_str(), 100, 20 * row, 20, WHITE);
			  DrawText(TextFormat("%d", players[i].second), 280, 20 * row, 20, WHITE);
		  }
	  }
	EndDrawing();
  }
  return 0;
}
