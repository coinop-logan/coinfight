#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>
#include "myvectors.h"

using EntityRef = uint16_t;
using coinsInt = unsigned long;

const uint ESCAPE_TO_QUIT_TICKS = 70;
const uint ESCAPE_TO_QUIT_TEXT_LIFE = 30;

const unsigned long MAX_COINS = UINT32_MAX;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const vector2i HALF_SCREENDIM = vector2i(WINDOW_WIDTH, WINDOW_HEIGHT) / 2;

const float DISTANCE_TOL = 0.0001;

const EntityRef NULL_ENTITYREF = 0;

const unsigned char NULL_TYPECHAR = 0;

const unsigned char PACKET_RESYNC_CHAR = 1;
const unsigned char PACKET_FRAMECMDS_CHAR = 2;

const unsigned char GOLDPILE_TYPECHAR = 1;
const unsigned char BEACON_TYPECHAR = 2;
const unsigned char FIGHTER_TYPECHAR = 3;
const unsigned char PRIME_TYPECHAR = 4;
const unsigned char GATEWAY_TYPECHAR = 5;

const std::chrono::duration<double, std::ratio<1,60>> ONE_FRAME(1);

const float ENTITY_COLLIDE_RADIUS = 15;

const int CREDIT_PER_DOLLAR_EXPONENT = 3; // credit = dollar * 10^X
const int WEI_PER_DOLLAR_EXPONENT = 18; // using xDai, so wei = dollar * 10^18

const coinsInt SCUTTLE_RATE = 5;

const coinsInt GATEWAY_COST = 4000;
const uint16_t GATEWAY_HEALTH = 1500;
const float GATEWAY_RANGE = 150;
const coinsInt GATEWAY_BUILD_RATE = 8;

const coinsInt BEACON_BUILD_RATE = 10;
const coinsInt BEACON_COST = GATEWAY_COST;
const uint16_t BEACON_HEALTH = 100;

const coinsInt PRIME_COST = 500;
const uint16_t PRIME_HEALTH = 100;
const float PRIME_SPEED = 2;
const float PRIME_RANGE = 150;
const coinsInt PRIME_PICKUP_RATE = 5;
const coinsInt PRIME_PUTDOWN_RATE = 8;
const coinsInt PRIME_MAX_GOLD_HELD = MAX_COINS;

const coinsInt FIGHTER_COST = 1500;
const uint16_t FIGHTER_HEALTH = 300;
const float FIGHTER_SPEED = 3;
const float FIGHTER_RANGE = 200;
const int FIGHTER_SHOOT_COOLDOWN = 20;
const int FIGHTER_DAMAGE = 10;

const vector2f FIGHTER_SHOT_OFFSET(20, 10);

const float SPACE_BETWEEN_SPAWNS = 500;

const float PARTICLE_MAGNET_STRENGTH = 1;
const float PARTICLE_FRICTION_CONSTANT = 0.9;

const int CURSOR_SIZE = 26;

#endif // CONFIG_H
