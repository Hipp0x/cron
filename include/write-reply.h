#ifndef SATURND_PRINT_H
#define SATURND_PRINT_H

#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server-reply.h"
#include "pipes.h"
#include "create-task.h"
#include "run_task.h"
#include "timing.h"

void write_reply_c (struct timing *t, uint32_t length, string **s);
void write_reply_std(uint64_t taskid, bool is_stdout);
void write_reply_terminate();
void write_reply_l(s_task **tasks, uint32_t nb_tasks);
void write_reply_t_ec(uint64_t taskid);
void write_reply_rm(uint64_t taskid);


#endif //SATURND_PRINT_H
