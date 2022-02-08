#include "saturnd.h"

/* Returns an array of all the arguments for the task.
The arguments are read from the file opened with the fd fd_a.
There are argc arguments to read. */
string **get_argv(uint32_t argc, int fd_a){
    int nb = 0;
    string **argv = malloc(nb * sizeof(string));
    for (int i = 0; i < argc;i++) {
        argv = realloc(argv,(nb+1)*sizeof(string));
        argv[nb] = malloc(sizeof(string));
        is_malloc_error(argv[nb]);

        uint32_t t;
        int res = read(fd_a, &t, sizeof(uint32_t));
        if (res <= 0){
            perror("Read error");
            exit(EXIT_FAILURE);
        }

        argv[nb]->length = t;
        argv[nb]->s = malloc(sizeof(BYTE) * (t+1));
        is_malloc_error(argv[nb]->s);
        res = read(fd_a, argv[nb]->s, (sizeof(BYTE) * t));
        if (res <= 0){
            perror("Read error");
            exit(EXIT_FAILURE);
        }
        argv[nb]->s[t] = '\0';
        nb = nb+1;
    }
    return argv;
}

/* Looks into the directory dir_path to see if the "removed"
 file is present. If so, returns true, otherwise, false. */
bool find_if_removed(char* dir_path) {
    char *removed_path = get_file_path(dir_path, "/removed");

    int ret = open(removed_path, O_RDONLY);
    free(removed_path);
    if (ret == -1) {
        return false;
    } else {
        return true;
    }
    close(ret);
}

/* Reads all the arguments of a task whose folder is dir_path.
Returns a s_task with all fields completed. */
s_task *read_all_arguments(s_task *task, char* dir_path) {
    char *path = get_file_path(dir_path, "/argv");
    int fd = open(path, O_RDONLY);

    uint32_t argc;
    int res = read(fd, &argc, sizeof(uint32_t));
    if (res < 0) {
        perror("Read error"); exit(EXIT_FAILURE);
    }

    task->command = malloc(sizeof(commandline));
    is_malloc_error(task->command);

    task->command->argc = argc;
    task->command->argv = get_argv(argc, fd);
    free(path);
    close(fd);
    return task;
}

/* Reads the timing for the task in the folder dir_path and returns
the s_task with the timing field completed. */
s_task *read_task_timing(s_task *task, char* dir_path) {
    char *path = get_file_path(dir_path, "/timing");

    int fd = open(path, O_RDONLY);
    task->t = read_timing(fd);

    free(path);
    close(fd);
    return task;
}

/* Reads the max_id that was already attributed in the saturnd/tasks/last_taskid
file and returns it. */
uint64_t read_max_id(char *dir_path) {
    char *filename = "/last_taskid";
    char *tmp = get_directory_path();
    char *path = malloc((strlen(filename) + strlen(tmp) + 2) * sizeof(char));
    strcpy(path, tmp);
    strcat(path, filename);
    free(tmp);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening the last_taskid file");
        exit(EXIT_FAILURE);
    }
    uint64_t max_id;
    read(fd, &max_id, sizeof(uint64_t));
    close(fd);
    free(path);
    return max_id;
}

/* Reads all the tasks from the disk and returns them in an array. */
s_task **read_all_tasks(uint64_t max_id) {
    s_task **all_tasks = malloc((max_id + 1) * sizeof(s_task));
    for (uint64_t i = 0; i < max_id + 1; i++) {
        all_tasks[i] = malloc(sizeof(s_task));
        is_malloc_error(all_tasks[i]);

        // get the path of the directory (with the id of the task)
        char* dir_path = get_directory_id_path(i);

        // parse the infos
        all_tasks[i]->id = i;
        all_tasks[i]->is_removed = find_if_removed(dir_path);
        all_tasks[i] = read_task_timing(all_tasks[i], dir_path);
        all_tasks[i] = read_all_arguments(all_tasks[i], dir_path);

        free(dir_path);
    }
    return all_tasks;
}

/* The main function of saturnd : it waits for requests from
cassini, writes the replies, and is also in charge of launching
the tasks every second. */
int main(int argc, char * argv[]) {
     create_files(); // create the folders if needed
     create_pipes(); // create the pipes if needed

     char *path = get_directory_tasks_path();
     uint64_t max_id = read_max_id(path);
     s_task** tasks = read_all_tasks(max_id);
     uint64_t nb_tasks = max_id + 1;

     char *pipes_directory = write_default_pipes_directory();
     char *request_pipe_name = get_pipe_name(pipes_directory, "saturnd-request-pipe");
     free(path);
     free(pipes_directory);

    while (1) {
        int fd_req = open_pipe(request_pipe_name, O_RDWR);
        // fd_req needs to be in RDWR so that saturnd doesn't hang waiting
        // for cassini to open its end

        struct pollfd poll_fd;
        poll_fd.fd = fd_req;
        poll_fd.events = POLLIN;

        poll(&poll_fd, 1, 1000);

        if (poll_fd.revents & POLLIN) { // there is a request in the tube
            uint16_t op;
            read(fd_req,&op,sizeof(uint16_t));
            op = be16toh(op);

            switch (op){
                case CLIENT_REQUEST_CREATE_TASK :
                read_request_c(fd_req);
                    for (uint32_t i = 0; i < nb_tasks; i++) {
                        for (uint32_t j = 0; j < tasks[i]->command->argc; j++){
                            free(tasks[i]->command->argv[j]->s);
                            free(tasks[i]->command->argv[j]);
                        }
                        free(tasks[i]->command->argv);
                        free(tasks[i]->command);
                        free(tasks[i]->t);
                        free(tasks[i]);
                    }
                    free(tasks);
                    tasks = read_all_tasks(nb_tasks);
                    nb_tasks = nb_tasks + 1;
                    break;
                case CLIENT_REQUEST_REMOVE_TASK :
                    {uint64_t task_ID = read_taskID(fd_req);
                    read_request_rm(fd_req, task_ID);
                    tasks[task_ID]->is_removed = true;
                    break;} // needs {} because we are initializing a value
                case CLIENT_REQUEST_GET_STDERR :
                    read_request_std(fd_req, false);
                    break;
                case CLIENT_REQUEST_GET_STDOUT :
                    read_request_std(fd_req, true);
                    break;
                case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
                    read_request_t_ec(fd_req);
                    break;
                case CLIENT_REQUEST_LIST_TASKS :
                    close_pipe(fd_req);
                    write_reply_l(tasks, nb_tasks);
                    break;
                case CLIENT_REQUEST_TERMINATE :
                    write_reply_terminate();
                    exit(0); // kill the deamon
                    break;
                default: break;
            }
        } else {
            // it's been 1 sec, go see if some tasks need to
            // be executed
            run_tasks(tasks, nb_tasks);
        }
    }
}