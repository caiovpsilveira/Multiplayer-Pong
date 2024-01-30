#ifndef RENDER_H
#define RENDER_H

#include "game.h"

#include <pthread.h>
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resize_callback(GLFWwindow* window, int width, int height);

typedef struct
{
    int isAlive; // unguarded
    int* keys;
    const Game* game;
    pthread_mutex_t* game_lock;
    pthread_mutex_t* keys_lock;
} RenderData;

void drawRect(const Rect* rect);
void drawGame(const Game* game);

void* initRenderThread(void* renderData);

#endif // RENDER_H
