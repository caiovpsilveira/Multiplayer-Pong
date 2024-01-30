#include "debug.h"
#include "game.h"
#include "message.h"
#include "player_input.h"

#include <assert.h>
#include <asm-generic/socket.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>

#define SERVER_TICK_RATE 60
#define MS_PER_SERVER_TICK (1000 / SERVER_TICK_RATE)

int createSocket()
{
    int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!socketFD)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    int err = setsockopt(socketFD,
                         SOL_SOCKET,
                         SO_REUSEADDR | SO_REUSEPORT,
                         &opt, sizeof(opt));
    if (err)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    return socketFD;
}

struct sockaddr_in createAddress(uint16_t port)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    return address;
}

int initServerSocket(uint16_t port)
{
    int socketFD = createSocket();
    struct sockaddr_in address = createAddress(port);

    if (bind(socketFD, (struct sockaddr*) &address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int maxConnectionQueueSize = 3;
    if (listen(socketFD, maxConnectionQueueSize) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return socketFD;
}

void handlePlayerInputMessage(const PlayerInputMessage* message, Player* p);

void handlePlayerMessage(const void* message, Player* p)
{
    MessageHeader* header = (MessageHeader*) message;

    switch (header->type)
    {
        case MessageType_PlayerInputMessage:
            handlePlayerInputMessage((const PlayerInputMessage*) message, p);
            break;
        default:
            assert("Invalid message type" && 0);
            break;
    }
}

void handlePlayerInputMessage(const PlayerInputMessage* message, Player* p)
{
    switch (message->input)
    {
        case PlayerInput_UP:
            p->box.pos.y += p->yvel;
            if (p->box.pos.y + p->box.size.y > WORLD_BOX_HEIGHT)
            {
                p->box.pos.y = WORLD_BOX_HEIGHT - p->box.size.y;
            }
            break;
        case PlayerInput_DOWN:
            p->box.pos.y -= p->yvel;
            if (p->box.pos.y < 0)
            {
                p->box.pos.y = 0;
            }
            break;
        default:
            assert("Invalid PlayerInputMessage" && 0);
            break;
    }
}

void centerPlayers(Player players[])
{
    players[0].box.pos.x = WALL_SPACING;
    players[0].box.pos.y = (WORLD_BOX_HEIGHT - players[0].box.size.y) / 2;

    players[1].box.pos.x = (WORLD_BOX_WIDTH - WALL_SPACING - players[1].box.size.x);
    players[1].box.pos.y = (WORLD_BOX_HEIGHT - players[1].box.size.y) / 2;
}

void initPlayers(Player players[])
{
    // Center the players in the world box, in the y axis.
    // Position each player WALL_SPACING far from the left and right wall
    // Player[0] is the left player. Player[1] the right

    players[0].box.size.x = 50;
    players[0].box.size.y = 100;

    players[0].score = 0;
    players[0].yvel = 10;

    players[1].box.size.x = 50;
    players[1].box.size.y = 100;

    players[1].score = 0;
    players[1].yvel = 10;

    centerPlayers(players);
}

void centerBall(Ball* ball)
{
    ball->box.pos.x = (WORLD_BOX_WIDTH - ball->box.size.x) / 2;
    ball->box.pos.y = (WORLD_BOX_HEIGHT - ball->box.size.y) / 2;
}

void randBallSpeed(Ball* ball)
{
    // gen two random floats:
    // x in [-1.0, -0.3] U [0.3, 1.0]
    // y in [-1.0, 1.0]
    float x;
    do
    {
        x = 2 * (float)rand()/RAND_MAX - 1;
    }
    while(x >= -0.3f && x <= 0.3f);
    float y = 2* (float)rand()/RAND_MAX - 1;

    // Normalize
    float length = sqrt(x*x + y*y);
    if (length != 0) {
        x /= length;
        y /= length;
    }

    // multiply by abs speed
    x *= ball->absVel;
    y *= ball->absVel;

    ball->vel.x = x;
    ball->vel.y = y;
}

void initBall(Ball* ball)
{
    // Center the ball in the middle of the world box

    ball->box.size.x = 10;
    ball->box.size.y = 10;
    ball->absVel = 5;

    centerBall(ball);
    randBallSpeed(ball);
}

void updateBall(Ball* ball)
{
    ball->box.pos.x += ball->vel.x;
    ball->box.pos.y += ball->vel.y;
}

int checkRectangleColision(const Rect* a, const Rect* b)
{
    return (a->pos.x + a->size.x >= b->pos.x &&
            a->pos.x <= b->pos.x + b->size.x &&
            a->pos.y + a->size.y >= b->pos.y &&
            a->pos.y <= b->pos.y + b->size.y);
}

void checkColisions(const Player players[], Ball* ball)
{
    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        if (checkRectangleColision(&players[i].box, &ball->box))
        {
            ball->vel.x = -ball->vel.x;
        }
    }
    if (ball->box.pos.y <= 0 || ball->box.pos.y + ball->box.size.y >= WORLD_BOX_HEIGHT)
    {
        ball->vel.y = -ball->vel.y;
    }
}

int checkScore(Player players[], const Ball* ball)
{
    if (ball->box.pos.x <= 0)
    {
        players[1].score += 1;
        return 1;
    }
    else if (ball->box.pos.x + ball->box.size.x >= WORLD_BOX_WIDTH)
    {
        players[0].score += 1;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    uint16_t port = 20000;
    int serverSocketFD = initServerSocket(port);

    struct sockaddr_storage p1_addr;
    socklen_t p1_addr_size = sizeof(p1_addr);
    int p1_socketFD = accept(serverSocketFD, (struct sockaddr*)& p1_addr, &p1_addr_size);
    if (p1_socketFD == -1)
    {
        perror("accept p1");
        exit(EXIT_FAILURE);
    }
    send_console_output(p1_socketFD, "Waiting for second player to connect.\n");

    struct sockaddr_storage p2_addr;
    socklen_t p2_addr_size = sizeof(p2_addr);
    int p2_socketFD = accept(serverSocketFD, (struct sockaddr*)& p2_addr, &p2_addr_size);
    if (p2_socketFD == -1)
    {
        perror("accept p2");
        exit(EXIT_FAILURE);
    }

    struct pollfd pollClientFD[NUM_PLAYERS];
    pollClientFD[0].fd = p1_socketFD;
    pollClientFD[0].events = POLLIN;

    pollClientFD[1].fd = p2_socketFD;
    pollClientFD[1].events = POLLIN;

    Game game;
    initPlayers(game.players);
    initBall(&game.ball);

    while (pollClientFD[0].fd && pollClientFD[1].fd)
    {
        struct timespec startTime;
        clock_gettime(CLOCK_MONOTONIC, &startTime);

        int num_events = poll(pollClientFD, NUM_PLAYERS, 0);
        if (num_events > 0)
        {
            for (int i = 0; i < NUM_PLAYERS && num_events; ++i)
            {
                int pollin_happened = pollClientFD[i].revents & POLLIN;
                if (pollin_happened)
                {
                    DEBUG_PRINT("Received message from player %d.\n", i);
                    int clientFD = pollClientFD[i].fd;
                    Message* message = receive_message(clientFD);

                    if (message)
                    {
                        handlePlayerMessage(message, &game.players[i]);
                        free(message);
                    }
                    else
                    {
                        pollClientFD[i].fd = 0;
                    }
                    
                    --num_events;
                }
            }
        }

        updateBall(&game.ball);
        checkColisions(game.players, &game.ball);
        if (checkScore(game.players, &game.ball))
        {
            centerPlayers(game.players);
            centerBall(&game.ball);
            randBallSpeed(&game.ball);
        }

        send_game_update(p1_socketFD, &game);
        send_game_update(p2_socketFD, &game);

        struct timespec endTime;
        clock_gettime(CLOCK_MONOTONIC, &endTime);

        long elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000 +
                           (endTime.tv_nsec - startTime.tv_nsec) / 1000000;

        long sleepTime = MS_PER_SERVER_TICK - elapsedTime;
        if (sleepTime > 0) {
            struct timespec sleepDuration = {.tv_sec = sleepTime / 1000, .tv_nsec = (sleepTime % 1000) * 1000000};
            nanosleep(&sleepDuration, NULL);
        }
        else
        {
            perror("Server is late...");
        }
    }

    close(p1_socketFD);
    close(p2_socketFD);
    close(serverSocketFD);
    return 0;
}
