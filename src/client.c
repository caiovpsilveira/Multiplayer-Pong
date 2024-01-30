#include "debug.h"
#include "game.h"
#include "message.h"
#include "player_input.h"
#include "render.h"

#include <assert.h>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <pthread.h> 

struct sockaddr_in createAddress(const char* ip_addr, uint16_t port)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_addr, &address.sin_addr) <= 0) {
        perror("Invalid address. Address not supported.\n");
        exit(EXIT_FAILURE);
    }

    return address;
}

int initClientSocket()
{
    int clientFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!clientFD)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    return clientFD;
}

void handleGameUpdateMessage(const GameUpdateMessage* message, Game* game);

void handleConsoleOutputMessage(const ConsoleOutputMessage* message);

void handleServerMessage(const void* message, Game* game)
{
    MessageHeader* header = (MessageHeader*) message;

    switch (header->type)
    {
        case MessageType_GameUpdateMessage:
            handleGameUpdateMessage((const GameUpdateMessage*) message, game);
            break;
        case MessageType_ConsoleOutputMessage:
            handleConsoleOutputMessage((const ConsoleOutputMessage*) message);
            break;
        default:
            assert("Invalid message type" && 0);
            break;
    }
}

void handleGameUpdateMessage(const GameUpdateMessage* message, Game* game)
{
    memcpy(game, &message->game, sizeof(Game));
}

void handleConsoleOutputMessage(const ConsoleOutputMessage* message)
{
    printf("%s", &message->str);
}

int main(int argc, char** argv)
{
    const char* ip_addr = "127.0.0.1";
    uint16_t port = 20000;

    int clientFD = initClientSocket();
    struct sockaddr_in server_addr = createAddress(ip_addr, port);

    if (connect(clientFD, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed,\n");
        exit(EXIT_FAILURE);
    }

    Game game;
    int* keys = (int*) malloc (sizeof(int) * GLFW_KEY_LAST);
    for (int i = 0; i < GLFW_KEY_LAST; ++i)
        keys[i] = 0;
    pthread_mutex_t game_lock;
    pthread_mutex_t keys_lock;

    if (pthread_mutex_init(&game_lock, NULL) != 0) { 
        perror("Game Mutex init has failed\n"); 
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&keys_lock, NULL) != 0) { 
        perror("Keys Mutex init has failed\n"); 
        exit(EXIT_FAILURE);
    }

    int err;
    pthread_t render_tid;
    RenderData rd = { 1 /*isAlive*/, keys, &game, &game_lock, &keys_lock };
    err = pthread_create(&render_tid, 
                         NULL, 
                         &initRenderThread, (void*) &rd);
    if (err != 0)
    {
        perror("Could not init render thread.");
        exit(EXIT_FAILURE);
    }
    DEBUG_PRINT("Created render thread tid %lu.\n", render_tid);

    while (rd.isAlive)
    {
        MessageHeader* message = receive_message(clientFD);
        if (message)
        {
            pthread_mutex_lock(&game_lock);
            handleServerMessage(message, &game);
            pthread_mutex_unlock(&game_lock);
            free(message);
        }
        else
        {
            rd.isAlive = 0;
        }

        pthread_mutex_lock(&keys_lock);
        if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_DOWN])
        {
            PlayerInput input = keys[GLFW_KEY_UP] ? PlayerInput_UP : PlayerInput_DOWN;
            send_player_input(clientFD, input);
        }
        pthread_mutex_unlock(&keys_lock);
    }

    pthread_join(render_tid, NULL); 
    pthread_mutex_destroy(&game_lock);
    pthread_mutex_destroy(&keys_lock); 

    free(keys);
    close(clientFD);
    return 0;
}
