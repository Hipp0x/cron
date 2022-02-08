#ifndef CREATE_TASK_H
#define CREATE_TASK_H

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "common-read.h"
#include "common-folder.h"
#include "write-reply.h"
#include "server-reply.h"

uint64_t create_new_task(struct timing *t, uint32_t length, string **s);

#endif // CREATE_TASK_H