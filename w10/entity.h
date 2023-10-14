#pragma once
#include <cstdint>
#include <vector>
#include "quantisation.h"
#include <string>

enum MoveDirection : uint8_t
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

MoveDirection opposite(MoveDirection dir);

constexpr uint16_t invalid_entity = -1;
constexpr uint16_t server_entity = 0;
struct Entity
{
	uint32_t color = 0xff00ffff;
	uint16_t eid = invalid_entity;

	vec2int posHead;

	std::string name;
	MoveDirection dir = RIGHT;

	uint8_t length = 1;
};
void simulate_entity(Entity &e, std::pair<uint16_t, uint8_t> p);
vec2int move(vec2int pos, MoveDirection dir);
void respawn(Entity& e);