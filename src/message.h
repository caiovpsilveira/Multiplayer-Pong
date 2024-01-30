#ifndef MESSAGE_H
#define MESSAGE_H

#include "game.h"

#include "player_input.h"

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    MessageType_PlayerInputMessage,
    MessageType_GameUpdateMessage,
    MessageType_ConsoleOutputMessage,
} MessageType;

const char* MessageType_toString(MessageType t);

typedef struct __attribute__((packed))
{
    MessageType type;
    uint32_t payload_size;
} MessageHeader;
typedef MessageHeader Message;

typedef struct __attribute__((packed))
{
    MessageHeader header;
    PlayerInput input;
} PlayerInputMessage;

typedef struct __attribute__((packed))
{
    MessageHeader header;
    Game game;
} GameUpdateMessage;

typedef struct __attribute__((packed))
{
    MessageHeader header;
    char str;
} ConsoleOutputMessage;

// Reads SIZE bytes from the socket stream, and stores into buffer
// Returns the number of bytes read, if any, or -1 if error.
int _receive_data(int socketFD, void* buf, size_t size);

// Returns a pointer to the received message.
// The first bytes correspond to the MessageHeader
// Returns NULL if any error occurred
Message* receive_message(int socketFD);

// Sends the message.
void _send_message(int socketFD, void* message);

void send_player_input(int socketFD, PlayerInput input);

void send_game_update(int socketFD, const Game* game);

void send_console_output(int socketFD, const char* str);

#endif // MESSAGE_H
