#include "helpers.hpp"

#ifdef SIMULATOR
template <class T>
T constrain(T in, T lower, T upper)
{
    if (in < lower)
        return lower;
    else if (in > upper)
        return upper;
    else
        return in;
}

template float constrain<float>(float in, float lower, float upper);

uint16_t lerp16by8(uint16_t from, uint16_t to, uint8_t frac)
{
    float diff = to - from;
    float fracDiff = diff * frac / 255;
    return (uint16_t)((float)from + fracDiff);
}
#endif
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void rotateZ(float rad, float vec[3])
{
    float x = vec[0];
    float y = vec[1];
    float z = vec[2];
    vec[0] = x * cos(rad) - y * sin(rad);
    vec[1] = y * cos(rad) - x * sin(rad);
}

float getDistance(Position a, Position b)
{
    float xyzA[3], xyzB[3];
    Position::ToXYZ(a, xyzA);
    Position::ToXYZ(b, xyzB);
    return sqrt(sq(xyzA[0] - xyzB[0]) + sq(xyzA[1] - xyzB[1]) + sq(xyzA[2] - xyzB[2]));
}