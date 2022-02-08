#ifndef SERVER_REPLY_H
#define SERVER_REPLY_H

#define SERVER_REPLY_OK 0x4f4b     // 'OK'
#define SERVER_REPLY_ERROR 0x4552  // 'ER'

#define SERVER_REPLY_ERROR_NOT_FOUND 0x4e46  // 'NF'
#define SERVER_REPLY_ERROR_NEVER_RUN 0x4e52  // 'NR'

typedef unsigned char BYTE;

/* string is a uint32 field that contains :
- the length `L` of the string (without the `0` at the end)
- then the `L` bytes that are the contents of the string */
typedef struct {
  uint32_t length;
  BYTE *s;
} string;

/* commandline contains :
- argc = the length of the argc array
- argv = an array of strings. argv[0] must contain the name of
  the command and not be empty, the rest of the array contains the
  arguments of the command. */
typedef struct {
  uint32_t argc;
  string **argv;
} commandline;

/* This struct contains all the info about a task :
- taskid : its id (they are unique, and not reused)
- t : the timing at which it must be run
- command : the command that should be run and all its arguments
            (see the commandline struct for more info)
*/

typedef struct {
  uint64_t taskid;
  struct timing *t;
  commandline *command;
} task;


/* This struct contains all the info about a process that has been run :
- the time at which it was run for the last time
- its exit code the last time it was run
*/
typedef struct {
  int64_t time;
  uint16_t exitcode;
} run;


#endif // SERVER_REPLY_H
