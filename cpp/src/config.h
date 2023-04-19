#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>
#include <string>
#include "myvectors.h"

using EntityRef = uint16_t;
using coinsInt = uint32_t;

const std::string SERVER_IP_DEFAULT = "18.206.114.77";
const unsigned int MAIN_PORT = 8473;

const coinsInt MAX_COINS = UINT32_MAX;

const fixed32 EPSILON(0.001);

const uint8_t NULL_TYPECHAR = 0;

const uint8_t PACKET_RESYNC_CHAR = 1;
const uint8_t PACKET_FRAMECMDS_CHAR = 2;

const uint8_t GOLDPILE_TYPECHAR = 1;
const uint8_t BEACON_TYPECHAR = 2;
const uint8_t FIGHTER_TYPECHAR = 3;
const uint8_t PRIME_TYPECHAR = 4;
const uint8_t GATEWAY_TYPECHAR = 5;
const uint8_t TURRET_TYPECHAR = 6;

const std::chrono::duration<double, std::ratio<1,60>> ONE_FRAME(1);

const fixed32 ENTITY_COLLIDE_RADIUS(15);

const int BLOCKCHAIN_WEI_EXPONENT = 18; // weiAmount / 10^? = humanReadbleAmount
const int LOCAL_CREDIT_EXPONENT = 3; // humanReadableAmount * 10^? = coinsIntAmount

const float PARTICLE_MAGNET_STRENGTH = 1;
const float PARTICLE_FRICTION_CONSTANT = 0.9;

const int CURSOR_SIZE = 26;

const fixed32 COLLISION_CORRECTION_BROADPHASE_FILTERBOX_HALFWIDTH(50);

const unsigned int DEPOSIT_TO_WALLET_PERCENT = 50;
const unsigned int DEPOSIT_TO_HONEYPOT_PERCENT = 40;

#endif // CONFIG_H
