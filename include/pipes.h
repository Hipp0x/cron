#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h> // open()
#include <string.h> // strcat()

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h> 

int open_pipe(char *name, int flags);
void close_pipe(int);
void create_pipes();

char* write_default_pipes_directory();
char *get_pipe_name(char*, char*);

int open_reply_pipe_saturnd();