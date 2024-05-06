#include "common.h"

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = LOBBY_PORT;

  ENetHost *lobby = enet_host_create(&address, 32, 2, 0, 0);

  if (!lobby)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }
  bool game_started = false;
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(lobby, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
		if (game_started)
		{
			send_string(event.peer, "0: Game started\nlocalhost:" + std::to_string(SERVER_PORT));
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
		  {
			  if (!game_started)
			  {//tried to check is server was available, but it was not working, even when creating a new client here
			/*	  ENetAddress serverAddress;
				  ENetPeer* serverPeer;
				  enet_address_set_host(&serverAddress, "localhost");
				  serverAddress.port = SERVER_PORT;
				  serverPeer = enet_host_connect(lobby, &serverAddress, 2, 0);
				  if (!serverPeer)
				  {
					  printf("Cannot connect to server\n");
					  return 1;
				  }
				  enet_peer_disconnect(serverPeer, 0);*/
				  game_started = true;
				  printf("Game started\n");
				  broadcast_string(lobby, "0: Game started\nlocalhost:" + std::to_string(SERVER_PORT));
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
  }

  enet_host_destroy(lobby);

  atexit(enet_deinitialize);
  return 0;
}

