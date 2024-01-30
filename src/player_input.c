#include "player_input.h"

#include "assert.h"
#include "stddef.h"

const char* PlayerInput_toString(PlayerInput i)
{
    char* str = NULL;
    switch (i)
    {
        case PlayerInput_UP:
            str = "PlayerInput_UP";
            break;
        case PlayerInput_DOWN:
            str = "PlayerInput_DOWN";
            break;
    }
    assert(str);
    return str;
}
