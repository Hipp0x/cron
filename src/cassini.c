#include "cassini.h"

const char usage_info[] = "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";


/*
Parses a char* into a string* :
- copies the char* into a BYTE* and put the data into it (without the trailing \0)
- computes the length of the string (still without the \0) into a uint32_t
 */
string* argToString (char* c) {
    string *str = malloc(sizeof(string));

    // length without the \0
    str->length = strlen(c);

    // copy the string into the array WITHOUT the trailing \0
    str->s = malloc(str->length);
    if (str->s == NULL) {
      perror("Malloc failure");
      exit(EXIT_FAILURE);
    }
    memcpy(str->s, c, str->length);
    return str;
}


/*
Parses the arguments that were passed to cassini.
The arguments are copied from char* into string*.
Returns a commandline struct that contains all the args, with the name of the
command as the first element of the array of string*.

Exits the program with return code 1 if there isn't at least one argument
to parse.
 */
commandline* get_commandline_arguments (int argc, char *argv[], int optind) {
    if (optind < argc) {
        // malloc the struct and the array
        commandline *c = malloc(sizeof(commandline));
        if (c == NULL) goto malloc_error;
        c->argv = malloc(argc * sizeof(string));
        if (argv == NULL) goto malloc_error;

        // find the number of arguments
        c->argc = argc - optind;

        // put the arguments into strings and into the array
        for(int i = 0; i < argc-optind; i++) {
            c->argv[i] = argToString(argv[optind+i]);
        }
    return c;
    } else { // the user didn't give a command to execute
        fprintf(stderr, "Missing a command name.\n %s", usage_info);
        exit(1);
    }

    malloc_error:
    fprintf(stderr, "Malloc failure\n");
    exit(1);
}


int main(int argc, char * argv[]) {
    errno = 0;
  
    char * minutes_str = "*";
    char * hours_str = "*";
    char * daysofweek_str = "*";
    char * pipes_directory = NULL;
  
    uint16_t operation = CLIENT_REQUEST_LIST_TASKS;
    uint64_t taskid;
    commandline *command;
    struct timing *t;
    int pipes_fd[2];

    int opt;
    char * strtoull_endp;
    while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
      switch (opt) {
      case 'm':
        minutes_str = optarg;
        break;
      case 'H':
        hours_str = optarg;
        break;
      case 'd':
        daysofweek_str = optarg;
        break;
      case 'p':
        pipes_directory = strdup(optarg);
        if (pipes_directory == NULL) goto error;
        break;
      case 'l':
        operation = CLIENT_REQUEST_LIST_TASKS;
        break;
      case 'c':
        operation = CLIENT_REQUEST_CREATE_TASK;
        break;
      case 'q':
        operation = CLIENT_REQUEST_TERMINATE;
        break;
      case 'r':
        operation = CLIENT_REQUEST_REMOVE_TASK;
        taskid = strtoull(optarg, &strtoull_endp, 10);
        if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
        break;
      case 'x':
        operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
        taskid = strtoull(optarg, &strtoull_endp, 10);
        if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
        break;
      case 'o':
        operation = CLIENT_REQUEST_GET_STDOUT;
        taskid = strtoull(optarg, &strtoull_endp, 10);
        if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
        break;
      case 'e':
        operation = CLIENT_REQUEST_GET_STDERR;
        taskid = strtoull(optarg, &strtoull_endp, 10);
        if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
        break;
      case 'h':
        printf("%s", usage_info);
        return 0;
      }
    }
    
    // if creating a task, fill the struct with the data
    // and get all the command arguments
    if (operation == CLIENT_REQUEST_CREATE_TASK) {
      t = malloc(sizeof(struct timing));
      if (t == NULL) goto error;

      int ret = timing_from_strings(t, minutes_str, hours_str, daysofweek_str);
      if (ret == -1) goto error;
      command = get_commandline_arguments(argc, argv, optind);
    }

    // find out the complete filenames of the pipes
    if (pipes_directory == NULL) {
        pipes_directory = write_default_pipes_directory();
    }
    char *request_pipe_name = get_pipe_name(pipes_directory, "saturnd-request-pipe");
    char *reply_pipe_name = get_pipe_name(pipes_directory, "saturnd-reply-pipe");
    
    //ouverture des pipes
    pipes_fd[1] = open_pipe(request_pipe_name, O_WRONLY | O_NONBLOCK);

    // write the request
    write_request(pipes_fd[1], operation, command, t, taskid);
    close_pipe(pipes_fd[1]);

    // read the reply
    pipes_fd[0] = open_pipe(reply_pipe_name, O_RDONLY);
    read_reply(pipes_fd[0], operation);
    close_pipe(pipes_fd[0]);

    free(request_pipe_name);
    free(reply_pipe_name);
    free(pipes_directory);
    return EXIT_SUCCESS;

    error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    free(request_pipe_name);
    free(reply_pipe_name);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}

