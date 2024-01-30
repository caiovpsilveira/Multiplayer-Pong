#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

typedef enum
{
    PlayerInput_UP,
    PlayerInput_DOWN,
} PlayerInput;

const char* PlayerInput_toString(PlayerInput i);

#endif // PLAYER_INPUT_H
