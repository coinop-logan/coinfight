#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>
#include "myvectors.h"

using EntityRef = uint16_t;
using coinsInt = unsigned long;

const std::string SERVER_IP_DEFAULT = "18.206.114.77";
const unsigned int MAIN_PORT = 8473;

const unsigned int ESCAPE_TO_QUIT_TICKS = 70;
const unsigned int ESCAPE_TO_QUIT_TEXT_LIFE = 30;

const unsigned long MAX_COINS = UINT32_MAX;

const float EPSILON = 0.00001;

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

const float PARTICLE_MAGNET_STRENGTH = 1;
const float PARTICLE_FRICTION_CONSTANT = 0.9;

const int CURSOR_SIZE = 26;

const float COLLISION_CORRECTION_BROADPHASE_FILTERBOX_HALFWIDTH = 50;

#endif // CONFIG_H
