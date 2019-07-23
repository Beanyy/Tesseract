#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "geometry.hpp"

#ifdef SIMULATOR
#include <stdint.h>
#include <cstring>
#endif

#define swap(x,y) do \
   { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
     memcpy(swap_temp,&y,sizeof(x)); \
     memcpy(&y,&x,       sizeof(x)); \
     memcpy(&x,swap_temp,sizeof(x)); \
    } while(0)

#ifdef SIMULATOR
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

template <class T>
T constrain(T in, T lower, T upper);

uint16_t lerp16by8(uint16_t from, uint16_t to, uint8_t frac);

#endif

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void rotateZ(float rad, float vec[3]);
float getDistance(Position a, Position b);
#endif
