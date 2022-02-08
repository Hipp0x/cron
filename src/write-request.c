#include "write-request.h"

/* Writes a request for CREATE_TASK. */
void write_request_c(int fd, uint16_t operation, commandline *command, struct timing *t) {
    // compute the total length of the request
    int length = sizeof(operation)
                + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t)  // timing fields
                + sizeof(uint32_t); // argc
    for (uint32_t i = 0; i < command->argc; ++i) {
        // add the length of the string + 32 (storing the length)
        length += sizeof(uint32_t) + command->argv[i]->length;
    }

    // create the request
    int current = 0;
    BYTE buff[length];
    operation = htobe16(operation);
    memcpy(buff, &operation, sizeof(uint16_t));
    current += sizeof(uint16_t);

    // copy the timing
    t->minutes = htobe64(t->minutes);
    memcpy(buff+current, &t->minutes, sizeof(uint64_t));
    current += sizeof(uint64_t);
    t->hours = htobe32(t->hours);
    memcpy(buff+current, &t->hours, sizeof(uint32_t));
    current += sizeof(uint32_t);
    memcpy(buff+current, &t->daysofweek, sizeof(uint8_t)); // no need to convert endian for days : there is only 1 byte
    current += sizeof(uint8_t);

    uint32_t argc_tmp = htobe32(command->argc);
    memcpy(buff+current, &argc_tmp, sizeof(uint32_t));
    current += sizeof(uint32_t);

    // copy command->argv
    for (int i = 0; i < command->argc; i++) {
        string *str = command->argv[i];
        uint32_t length_tmp = htobe32(str->length);
        memcpy(buff+current, &length_tmp, sizeof(uint32_t));
        current += sizeof(uint32_t);
        memcpy(buff+current, str->s, str->length);
        current += str->length;
        free(command->argv[i]->s);
        free(command->argv[i]);
    }

    // write the request
    write(fd, buff, length);
}

/* Writes a request that contains the operation code and the taskid.
This is for REMOVE, GET_TIMES_AND_EXIT_CODES, STDOUT and STDERR requests. */
void write_request_taskid(int fd, uint16_t operation, uint64_t taskid) {
    size_t length = sizeof(uint16_t) + sizeof(uint64_t);
    // copy the info into the BYTE buffer
    BYTE buff[length];
    operation = htobe16(operation);
    memcpy(buff, &operation, sizeof(uint16_t));
    taskid = htobe64(taskid);
    memcpy(buff+sizeof(uint16_t), &taskid, sizeof(uint64_t));
    // write the request to the pipe
    write(fd, buff, length);
}

/* Writes a request that only contains the operation code.
This is for LIST_TASKS and TEERMINATE requests. */
void write_request_operation_code(int fd, uint16_t operation) {
    operation = htobe16(operation);
    write(fd, &operation, sizeof(operation));
}


/* Main method to write the request.
Takes all the possible arguments (see protocol.md) and writes a request to fd
depending on the operation code. */
void write_request(int fd, uint16_t operation, commandline *command, struct timing *t, uint64_t taskid) {
    switch (operation) {
        case CLIENT_REQUEST_CREATE_TASK:
            write_request_c(fd, operation, command, t);
            free(t);
            free(command->argv);
            free(command);
            break;
        // below are the requests with only the operation code
        case CLIENT_REQUEST_LIST_TASKS:
        case CLIENT_REQUEST_TERMINATE:
            write_request_operation_code(fd, operation);
            break;
        // below are all the requests with only the operation code and the taskid
        case CLIENT_REQUEST_REMOVE_TASK:
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:
        case CLIENT_REQUEST_GET_STDOUT:
        case CLIENT_REQUEST_GET_STDERR:
            write_request_taskid(fd, operation, taskid);
            break;
    }
}
