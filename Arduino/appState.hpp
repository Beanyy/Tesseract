#ifndef APPSTATE_HPP
#define APPSTATE_HPP

#include "animation.hpp"
class AppState
{
private:
    static const int nModifiers = 10;
    struct Modifier
    {
        char cmd;
        char size;
        void *value;
    };
    Modifier modifiers[nModifiers];

public:
    AppState();
    CHSV color;
    int animation;
    int speed;
    char off;
    char pause;
    char colorLock;
    char motionLock;
    void Update(char *cmd);
    void Reset();
    void ApplyState(AnimationBase *ani, float defaultSpeed, CRGB defaultColor);
};
#endif