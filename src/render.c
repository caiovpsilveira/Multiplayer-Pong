#include "render.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define FPS_LIMIT 30

void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    RenderData* renderData = (RenderData*) glfwGetWindowUserPointer(window);
    int* keys = renderData->keys;
    pthread_mutex_t* keys_lock = renderData->keys_lock;

    pthread_mutex_lock(keys_lock);
    if (action == GLFW_PRESS)
    {
        keys[key] = 1;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = 0;
    }
    pthread_mutex_unlock(keys_lock);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void drawRect(const Rect* rect)
{
    glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f(rect->pos.x, rect->pos.y, 0.0);
    glVertex3f(rect->pos.x + rect->size.x, rect->pos.y, 0.0);
    glVertex3f(rect->pos.x + rect->size.x, rect->pos.y + rect->size.y, 0.0);
    glVertex3f(rect->pos.x, rect->pos.y + rect->size.y, 0.0);
    glEnd();
}

void drawGame(const Game* game)
{
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT);
    drawRect(&game->ball.box);
    drawRect(&game->players[0].box);
    drawRect(&game->players[1].box);
}

void* initRenderThread(void* renderData)
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    int width = 600;
    int height = 600;
    GLFWwindow* window = glfwCreateWindow(width, height, "Pong", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize_callback);
    glViewport(0, 0, width, height);

    // Set initial coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WORLD_BOX_WIDTH, 0, WORLD_BOX_HEIGHT, -1, 1);

    glfwSetWindowUserPointer(window, renderData);
    const Game* game = ((RenderData*) renderData)->game;
    pthread_mutex_t* game_lock = ((RenderData*) renderData)->game_lock;

    double lastFrameTime = 0;
    double fpsLimit = 1.0 / FPS_LIMIT;
    while (!glfwWindowShouldClose(window) && ((RenderData*) renderData)->isAlive)
    {
        glfwPollEvents();
        double now = glfwGetTime();
        if (now - lastFrameTime >= fpsLimit)
        {
            pthread_mutex_lock(game_lock);
            drawGame(game);
            pthread_mutex_unlock(game_lock);

            glfwSwapBuffers(window);

            lastFrameTime = now;
        }
    }

    ((RenderData*) renderData)->isAlive = 0;

    glfwDestroyWindow(window);
    glfwTerminate();
    return NULL;
}
