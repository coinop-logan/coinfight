#include <chrono>

#ifndef CONFIG_H
#define CONFIG_H

const std::chrono::duration<double, std::ratio<1,60>> ONE_FRAME(1);

#endif // CONFIG_H