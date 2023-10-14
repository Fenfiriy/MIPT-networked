#include "entity.h"
#include "mathUtils.h"


MoveDirection opposite(MoveDirection dir)
{
	switch (dir)
	{
	case UP:
		return DOWN;
	case DOWN:
		return UP;
	case LEFT:
		return RIGHT;
	case RIGHT:
		return LEFT;
	default:
		break;
	}
}

void simulate_entity(Entity &e, std::pair<uint16_t, uint8_t> p)
{
	uint16_t eid = p.first;
	uint8_t last_visited = p.second;
	switch (eid)
	{
	case server_entity:
		e.length++;
	case invalid_entity:
		e.posHead = move(e.posHead, e.dir);
		break;
	default:
		respawn(e);
		break;
	}
}

vec2int move(vec2int pos, MoveDirection dir)
{
	vec2int newpos = pos;
	switch (dir)
	{
	case UP:
		newpos.y--;
		break;
	case DOWN:
		newpos.y++;
		break;
	case LEFT:
		newpos.x--;
		break;
	case RIGHT:
		newpos.x++;
		break;
	default:
		break;
	}

	return newpos;
}

void respawn(Entity& e)
{
	uint8_t x = (rand() % 4) * 4 + 10;
	uint8_t y = (rand() % 4) * 4 + 10;
	e.posHead = vec2int{ x, y };
	e.length = 1;
}