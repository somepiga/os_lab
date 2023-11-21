#ifndef CHATROOM_H
#define CHATROOM_H
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERR_MSG 0
#define REG 1
#define TRANS 2

extern volatile int read_listen;

extern void signal_handler(int signum);

typedef struct message {
    int type;
    char from;
    char to[10];
    char data[88];
} message;

void toString(message msg, char* input) {
    int i;
    if (msg.type == REG) {
        input[0] = '1';
        input[1] = msg.from;
        input[2] = '\0';
    } else if (msg.type == TRANS) {
        input[0] = '2';
        input[1] = msg.from;
        for (i = 0; i < 10; ++i) {
            input[i + 2] = (msg.to[i] == '\0') ? '0' : msg.to[i];
        }
        i = 12;
        int k;
        for (k = 0; k < strlen(msg.data); ++k) {
            input[i++] = msg.data[k];
        }
        input[i] = '\0';
    }
}

message parse(char* output) {
    message msg;
    // to REG
    if (output[0] == '1') {
        msg.type = REG;
        msg.from = output[1];
        return msg;
    }

    // to TRANS
    if (output[0] == '2') {
        msg.type = TRANS;
        msg.from = output[1];
        memcpy(msg.to, output + 2, 10);
        memcpy(msg.data, output + 12, 88);
        return msg;
    }

    msg.type = ERR_MSG;
    return msg;
}

#endif  // CHATROOM_H
