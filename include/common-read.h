#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "timing.h"
#include "client-request.h"
#include "server-reply.h"

void is_malloc_error(void *p);
void is_read_error(int val);
string **read_args(int fd, uint32_t argc);
string *read_string(int fd);


struct timing *read_timing(int fd);
/* Reads a timing from the open fd and returns it. */

task **parse_tasks(int fd, uint32_t nbTasks);
task *parse_one_task(int fd);

uint64_t read_taskID(int fd);