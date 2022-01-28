#include "coins.h"
#include <glm/glm.hpp>

#ifndef CONFIG_H
#define CONFIG_H

using EntityRef = uint16_t;
using coinsInt = unsigned long;

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 400;

const float DISTANCE_TOL = 0.0001;

const EntityRef NULL_ENTITYREF = 0;

const unsigned char NULL_TYPECHAR = 0;

const unsigned char RESYNC_PACKET_CHAR = 'R';
const unsigned char FRAMECMDS_PACKET_CHAR = 'C';

const unsigned char GOLDPILE_TYPECHAR = 'g';
const unsigned char PRIME_TYPECHAR = 'P';
const unsigned char GATEWAY_TYPECHAR = 'G';

const float SEC_PER_FRAME = 1 / 30.0;

const float ENTITY_COLLIDE_RADIUS = 10;

const coinsInt PRIME_COST = 2000;
const float PRIME_SPEED = 2;
const float PRIME_RANGE = 50;
const coinsInt PRIME_PICKUP_RATE = 5;
const coinsInt PRIME_PUTDOWN_RATE = 8;
const coinsInt PRIME_MAX_GOLD_HELD = MAX_COINS;
const coinsInt PRIME_BUILD_RATE = 100;

const coinsInt GATEWAY_COST = 10000;
const float GATEWAY_RANGE = 50;

#endif // CONFIG_H