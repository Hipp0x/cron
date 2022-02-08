#ifndef RUN_TASK_H
#define RUN_TASK_H

#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "server-reply.h"
#include "common-folder.h"

typedef struct {
  uint64_t id;
  struct timing *t;
  commandline *command;
  bool is_removed;
} s_task;

void run_tasks();

#endif // RUN_TASK
