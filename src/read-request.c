#include "read-request.h"

/*Parses a CREATE request from the request pipe fd.
Then launches write_reply_c which will :
- create the task on the disk
- write the reply to the reply pipe. */
void read_request_c (int fd){
    // read the timing
    struct timing *t = read_timing(fd);

    // read the args of the command line
    uint32_t argc;
    is_read_error(read(fd, &argc, sizeof(uint32_t)));
    argc = be32toh(argc);
    string **argv = read_args(fd, argc);
    close_pipe(fd);
    write_reply_c(t, argc, argv);
}

/* Reads the request for stdout (if is_stdout = 1) or for stderr (if is_stdout = 0).
 * Then launches the methods to write the reply to the reply pipe. */
void read_request_std(int fd, int is_stdout) {
    // read the id of the task
    uint64_t taskid;
    read(fd, &taskid, sizeof(uint64_t));
    taskid = htobe64(taskid);
    // write the reply
    write_reply_std(taskid, is_stdout);
}

/*  Reads the request for remove then launches the methods
to remove a task and write the reply.  */
void read_request_rm(int fd, uint64_t task_id){
    write_reply_rm(task_id);
}

/* Reads the request for Time_Exit_Codes and launches
the method to reply */
void read_request_t_ec(int fd){
    uint64_t task_id = read_taskID(fd);
    write_reply_t_ec(task_id);
}

