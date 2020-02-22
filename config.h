#ifndef CONFIG_H
#define CONFIG_H

const float DISTANCE_TOL = 0.0001;

const unsigned char NULL_TYPECHAR = 0;

const unsigned char RESYNC_PACKET_CHAR = 'R';
const unsigned char FRAMECMDS_PACKET_CHAR = 'C';

const unsigned char GOLDPILE_TYPECHAR = 'g';
const unsigned char PRIME_TYPECHAR = 'P';
const unsigned char GATEWAY_TYPECHAR = 'G';

const float SEC_PER_FRAME = 1 / 30.0;

const float ENTITY_COLLIDE_RADIUS = 10;

const float GATEWAY_TRANSFER_RATE = 0.2; // credit per frame

const float PRIME_COST = 20;
const float PRIME_SPEED = 2;
const float PRIME_RANGE = 50;
const int PRIME_PICKUP_RATE = 5;

const float GATEWAY_COST = 100;
const float GATEWAY_RANGE = 50;

#endif // CONFIG_H