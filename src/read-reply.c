#include "read-reply.h"

/* Reads the reply to the LIST_TASK request. */
void read_reply_l(int fd) {
    // read the number of tasks
    uint32_t nbTasks;
    read(fd, &nbTasks, sizeof(uint32_t));
    nbTasks = be32toh(nbTasks);

    // parse the list of tasks and print them
    if (nbTasks != 0) {
        task **tasks = parse_tasks(fd, nbTasks);
        print_reply_l(nbTasks, tasks);
    }  // If there are no tasks, there is nothing to print

}

/* Reads the reply to the CREATE_TASK request. */
void read_reply_c(int fd) {
    uint64_t taskid;
    read(fd, &taskid, sizeof(uint64_t));
    taskid = be64toh(taskid);
    print_reply_c(taskid);
}

/* Reads the error code. */
void read_reply_error(int fd) {
    // get the error code
    uint16_t errcode;
    read(fd, &errcode, sizeof(uint16_t));
    errcode = htobe16(errcode);

    print_error(errcode);
}

/* Read the reply to the GET_TIMES_AND_EXIT_CODES request. */
void read_reply_x(int fd, uint16_t repcode) {
    if (repcode == SERVER_REPLY_ERROR) {
        read_reply_error(fd);
    } else {
        uint32_t nbRuns;
        read(fd, &nbRuns, sizeof(uint32_t));
        nbRuns = htobe32(nbRuns);
        run **runs = malloc(nbRuns * sizeof(run));
        is_malloc_error(runs);

        int64_t time;
        uint16_t exitcode;

        for (uint32_t i = 0; i < nbRuns; i++) {
            runs[i] = malloc(sizeof(run));
            is_malloc_error(runs);

            read(fd, &time, sizeof(time));
            time = htobe64(time);
            runs[i]->time = time;

            read(fd, &exitcode, sizeof(exitcode));
            exitcode = htobe16(exitcode);
            runs[i]->exitcode = exitcode;
        }
        print_times_and_exit_codes(nbRuns, runs);
    }
}

/* Reads the reply to the STDERR or STDOUT requests. */
void read_reply_std(int fd, uint16_t repcode) {
    if (repcode == SERVER_REPLY_ERROR) {
        read_reply_error(fd); // printing call is inside read_reply_error
    } else {
        string *output = read_string(fd);
        print_output(output);
        free(output->s);
        free(output);
    }
}


/* Main method to read the reply of the daemon.
- fd : the file descriptor of the reply pipe
- operation : the operation code of the original request

The reply gets parsed and then the reply is formatted and printed
to stdout. The print statement is often inside one of the auxiliary
functions.
*/
void read_reply(int fd, uint16_t operation) {
    // read the repcode
    uint16_t repcode;
    read(fd, &repcode, sizeof(uint16_t));
    repcode = htobe16(repcode);

    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS:
            read_reply_l(fd);
            break;
        case CLIENT_REQUEST_CREATE_TASK:
            read_reply_c(fd);
            break;
        case CLIENT_REQUEST_REMOVE_TASK:
            if (repcode == SERVER_REPLY_ERROR) {
                read_reply_error(fd);
            } // otherwise, nothing to do (and nothing to print)
            break;
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:
            read_reply_x(fd, repcode);
            break;
        case CLIENT_REQUEST_GET_STDERR:
        case CLIENT_REQUEST_GET_STDOUT:
            read_reply_std(fd, repcode);
        case CLIENT_REQUEST_TERMINATE: break; // nothing to do

    }
}
