#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "glib.h"
#include "server.h"

typedef enum {
    PROCESS_CMD_REGISTER = 0x01,
    PROCESS_CMD_SEND,
    PROCESS_CMD_LIST,
    PROCESS_CMD_RET,
}PROCESS_CMD_TYPE;

void process_dispatcher(GIOChannel *io, char *data);




















#endif

