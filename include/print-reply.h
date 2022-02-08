#include <stdio.h>          // stdout, fprintf()
#include <stdlib.h>         // free()
#include <time.h>           // localtime()
#include <stdint.h>         // uint types
#include "server-reply.h"
#include "timing-text-io.h"

void print_reply_l(uint32_t, task**);

void print_reply_c(uint64_t);

void print_error(uint16_t);

void print_times_and_exit_codes(uint32_t, run**);

void print_output(string*);


