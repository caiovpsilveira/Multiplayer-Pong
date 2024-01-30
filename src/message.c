#include "message.h"
#include "debug.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

const char* MessageType_toString(MessageType t)
{
    char* str = NULL;
    switch (t)
    {
        case MessageType_PlayerInputMessage:
            str = "MessageType_PlayerInputMessage";
            break;
        case MessageType_GameUpdateMessage:
            str = "MessageType_GameUpdateMessage";
            break;
        case MessageType_ConsoleOutputMessage:
            str = "MessageType_ConsoleOutputMessage";
            break;
    }
    assert(str);
    return str;
}

int _receive_data(int socketFD, void* buf, size_t size)
{
    size_t total_received = 0;
    while (total_received < size) {
        ssize_t received = recv(socketFD, buf + total_received, size - total_received, 0);
        if (received <= 0) return -1;
        total_received += received;
    }
    return total_received;
}

Message* receive_message(int socketFD)
{
    // Receive header, common to all messages
    void* header = malloc(sizeof(MessageHeader));
    if (_receive_data(socketFD, header, sizeof(MessageHeader)) < 0) {
        return NULL;
    }

    // Allocate more space accordingly to the message size
    void* tmp = realloc(header, sizeof(MessageHeader) + ((MessageHeader*)header)->payload_size);
    if (tmp)
    {
        header = tmp;
    }
    else
    {
        perror("Error reallocating memory for header payload.");
        free(header);
        return NULL;
    }

    // Receive the rest of the Message
    if (_receive_data(socketFD, header + sizeof(MessageHeader), ((MessageHeader*)header)->payload_size) < 0) {
        return NULL;
    }

    return header;
}

void _send_message(int socketFD, void* message)
{
    uint32_t psize = ((MessageHeader*) message)->payload_size;
    send(socketFD, message, sizeof(MessageHeader) + psize, 0);
    DEBUG_PRINT("Sent %lu bytes.\n", sizeof(MessageHeader) + psize);
}

void send_game_update(int socketFD, const Game* game)
{
    GameUpdateMessage msg;
    msg.header.type = MessageType_GameUpdateMessage;
    msg.header.payload_size = sizeof(GameUpdateMessage) - sizeof(MessageHeader);
    memcpy(&msg.game, game, sizeof(Game));
    _send_message(socketFD, &msg);
}

void send_player_input(int socketFD, PlayerInput input)
{
    PlayerInputMessage msg;
    msg.header.type = MessageType_PlayerInputMessage;
    msg.header.payload_size = sizeof(PlayerInputMessage) - sizeof(MessageHeader);
    msg.input = input;
    _send_message(socketFD, &msg);
}

void send_console_output(int socketFD, const char* str)
{
    uint32_t size = strlen(str);
    // '\0' slot not needed because ConsoleOutputMessage has a char member
    ConsoleOutputMessage* msg = malloc(sizeof(ConsoleOutputMessage) + size);
    msg->header.type = MessageType_ConsoleOutputMessage;
    msg->header.payload_size = size;
    strcpy(&msg->str, str);
    _send_message(socketFD, msg);
    free(msg);
}
