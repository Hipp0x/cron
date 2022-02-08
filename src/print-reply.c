#include "print-reply.h"

/* Prints the reply to the CREATE_LIST request.
- nbTasks : the number of tasks to print
- tasks : an array of struct task that contains all the info about the tasks
*/
void print_reply_l(uint32_t nbTasks, task **tasks) {
    for (uint32_t i = 0; i < nbTasks; i++) {
        // get the timing as a string
        char buf_timing[TIMING_TEXT_MIN_BUFFERSIZE];
        timing_string_from_timing(buf_timing, tasks[i]->t);

        // print taskid and timing
        fprintf(stdout, "%lu: %s ", tasks[i]->taskid, buf_timing);

        // print command line args
        for (uint32_t j = 0 ; j < tasks[i]->command->argc; j++){
            fprintf(stdout, "%s ", tasks[i]->command->argv[j]->s);
            free(tasks[i]->command->argv[j]->s);
            free(tasks[i]->command->argv[j]);
        }
        fprintf(stdout, "\n");
        free(tasks[i]->command->argv);
        free(tasks[i]->command);
        free(tasks[i]->t);
        free(tasks[i]);
    }
    free(tasks);
}

/* Prints the reply to the request CREATE_TASK.
- taskid : the id of the task that was created */
void print_reply_c(uint64_t taskid) {
    fprintf(stdout, "%lu\n", taskid);
}

/* Prints the error message bases on the error code.
- errcode : the error code sent by the daemon */
void print_error(uint16_t errcode) {
    if (errcode == SERVER_REPLY_ERROR_NOT_FOUND) {
        fprintf(stderr, "No task with this ID.\n");
    } else if (errcode == SERVER_REPLY_ERROR_NEVER_RUN) {
        fprintf(stderr, "This task hasn't been run yet\n");
    }
    exit(1);
}

/* Prints the answer to the GET_TIME_AND_EXIT_CODES request.
- nbRuns : the number of times the task was run
- runs : an array of struct run that contains the time and exit codes
         of each run
*/
void print_times_and_exit_codes(uint32_t nbRuns, run** runs) {
    for (uint32_t i = 0; i < nbRuns; i++) {
        struct tm *t = localtime(&runs[i]->time);
        int year = t->tm_year + 1900;
        int month = t->tm_mon + 1; // 0 = january
        int day = t->tm_mday;
        int h = t->tm_hour;
        int m = t->tm_min;
        int s = t->tm_sec;
        fprintf(stdout, "%02d-%02d-%02d %02d:%02d:%02d %u\n", year, month, day, h, m, s, runs[i]->exitcode);
        free(runs[i]);
    }
    free(runs);
}

/* Prints the answer to the GET_STDERR or GET_STDOUT requests :
- output : the string that contains everything that was writtent to
           stdout or stderr on the last run of the task
*/
void print_output(string *output) {
    fprintf(stdout, "%s\n", output->s);
}