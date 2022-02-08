#include "common-read.h"


/* Terminates the program if p is NULL. */
void is_malloc_error(void *p) {
    if (p == NULL) {
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }
}

/* Terminates the program if there was a reading error.
- val : the return value of the `write` function call */
void is_read_error(int val) {
    if (val == -1) {
        perror("Reading error");
        exit(EXIT_FAILURE);
    }
}

/* Returns a timing read from fd. */
struct timing *read_timing(int fd) {
    uint64_t minutes; uint32_t hours; uint8_t daysofweek;

    struct timing *t = malloc(sizeof(struct timing));
    is_malloc_error(t);

    int val = read(fd, &minutes, sizeof(uint64_t));
    if (val == -1) {
        perror("Can't read the minutes from the timing");
        exit(EXIT_FAILURE);
    }
    val = read(fd, &hours, sizeof(uint32_t));
    if (val == -1) {
        perror("Can't read the hours from the timing");
        exit(EXIT_FAILURE);
    }
    val = read(fd, &daysofweek, sizeof(uint8_t));
    if (val == -1) {
        perror("Can't read the day of the week from the timing");
        exit(EXIT_FAILURE);
    }

    t->minutes = be64toh(minutes);
    t->hours = be32toh(hours);
    t->daysofweek = daysofweek; // no need to invert bytes, there is only 1

    return t;
}

/* Reads argc arguments from fd and formats them into string*.
Returns a string** that contains all the arguments. */
string **read_args(int fd, uint32_t argc) {
    string **argv = malloc(argc * sizeof(string));
    is_malloc_error(argv);
    for(uint32_t j = 0; j < argc; j++){
        argv[j] = read_string(fd);
    }
    return argv;
}



/* Returns a string* read from fd. */
string *read_string(int fd) {
    string *str = malloc(sizeof(string) +1);

    is_malloc_error(str);

    // read the length of the arg
    uint32_t strlength;
    is_read_error(read(fd, &strlength, sizeof(uint32_t)));
    str->length = be32toh(strlength);

    // malloc the BYTE array for the arg
    str->s = malloc(str->length * sizeof(BYTE)+1);
    is_malloc_error(str->s);

    // read the arg
    is_read_error(read(fd, str->s, str->length));
    str->s[str->length * sizeof(BYTE)] = 0;

    return str;
}

/* Reads and returns one task read from fd. */
task *parse_one_task(int fd) {
    // malloc the task
    task *t = malloc(sizeof(task));
    is_malloc_error(t);

    // read the id of the task
    uint64_t taskid;
    is_read_error(read(fd, &taskid, sizeof(uint64_t)));
    t->taskid = be64toh(taskid);

    // read the timing of the task
    t->t = read_timing(fd);

    // read the command and its args
    t->command = malloc(sizeof(commandline));
    is_malloc_error(t->command);

    // read the number of args
    uint32_t argc;
    is_read_error(read(fd, &argc, sizeof(uint32_t)));
    t->command->argc = be32toh(argc);

    if (argc != 0) { // read the args
        t->command->argv = read_args(fd, t->command->argc);
    }

    return t;
}

/* Reads nbTasks tasks from fd.
Returns the task** that contains all the info. */
task **parse_tasks(int fd, uint32_t nbTasks) {
    task **tasks = malloc(nbTasks * sizeof(task));
    is_malloc_error(tasks);

    for(uint32_t i = 0; i < nbTasks; i++){
        tasks[i] = parse_one_task(fd);
    }
    return tasks;
}

/* Reads a task_ID from fd.
Returns it as a uint64_t*/
uint64_t read_taskID(int fd){
    uint64_t taskid;
    is_read_error(read(fd, &taskid, sizeof(uint64_t)));
    taskid = be64toh(taskid);
    return taskid;
}
