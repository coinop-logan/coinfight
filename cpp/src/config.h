#ifndef CONFIG_H
#define CONFIG_H

#include "myvectors.h"

using EntityRef = uint16_t;
using coinsInt = unsigned long;

const unsigned long MAX_COINS = UINT32_MAX;

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 800;
const vector2i HALF_SCREENDIM = vector2i(WINDOW_WIDTH, WINDOW_HEIGHT) / 2;

const uint NEEDED_PLAYERS = 1;

const float DISTANCE_TOL = 0.0001;

const EntityRef NULL_ENTITYREF = 0;

const unsigned char NULL_TYPECHAR = 0;

const unsigned char PACKET_RESYNC_CHAR = 'R';
const unsigned char PACKET_FRAMECMDS_CHAR = 'C';

const unsigned char EVENT_BALANCEUPDATE_CHAR = 1;
const unsigned char EVENT_GAMESTART_CHAR = 2;

const unsigned char GOLDPILE_TYPECHAR = 'g';
const unsigned char PRIME_TYPECHAR = 'P';
const unsigned char GATEWAY_TYPECHAR = 'G';

const float SEC_PER_FRAME = 1 / 60.0;

const float ENTITY_COLLIDE_RADIUS = 10;

const int CREDIT_PER_DOLLAR_EXPONENT = 3; // credit = dollar * 10^X
const int WEI_PER_DOLLAR_EXPONENT = 18; // using xDai, so wei = dollar * 10^18

const coinsInt PRIME_COST = 1000;
const float PRIME_SPEED = 4;
const float PRIME_RANGE = 150;
const coinsInt PRIME_PICKUP_RATE = 5;
const coinsInt PRIME_PUTDOWN_RATE = 8;
const coinsInt PRIME_MAX_GOLD_HELD = MAX_COINS;
const coinsInt PRIME_BUILD_RATE = 5;

const coinsInt GATEWAY_COST = 100;
const float GATEWAY_RANGE = 150;

const float SPACE_BETWEEN_SPAWNS = 100;

#endif // CONFIG_H