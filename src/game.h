#ifndef GAME_H
#define GAME_H

// TODO: Server and Client should not have
// the same "view" of the game components,
// it's unecessary.

#define NUM_PLAYERS 2
#define WORLD_BOX_WIDTH 1000
#define WORLD_BOX_HEIGHT 1000
#define WALL_SPACING 50

typedef struct
{
    float x, y;
} Vec2f;

typedef struct
{
    Vec2f pos;
    Vec2f size;
} Rect;

typedef struct
{
    int score;
    Rect box;
    float yvel;
} Player;

typedef struct
{
    Rect box;
    Vec2f vel;
    float absVel;
} Ball;

typedef struct
{
    Player players[NUM_PLAYERS];
    Ball ball;
} Game;

#endif // GAME_H
