#include "write-reply.h"

/* Writes a reply in the fd.
If ok, then only writes SERVER_REPLY_OK.
If not ok, then write both SERVER_REPLY_ERROR and the errcode */
void write_reply_code(int fd, bool ok, uint16_t errcode){
    if(ok) {
        uint16_t rep = be16toh(SERVER_REPLY_OK);
        write(fd, &rep, sizeof(uint16_t));
    } else {
        size_t length = sizeof(uint16_t) * 2;
        BYTE buff[length];
        uint16_t rep = SERVER_REPLY_ERROR;
        rep = be16toh(rep);
        uint16_t error = be16toh(errcode);
        memcpy(buff, &rep, sizeof(uint16_t));
        memcpy(buff + sizeof(uint16_t), &error, sizeof(uint16_t));
        write(fd, buff, length);
        close_pipe(fd);
    }
}

/* First creates a new task on the disk.
Then writes a reply for the CREATE request in the reply pipe.
*/
void write_reply_c (struct timing *t, uint32_t argc, string **argv){
    // create the task
    uint64_t taskid = create_new_task(t, argc, argv);

    // open the fd
    int fd = open_reply_pipe_saturnd();

    // prepare the reply
    int length = sizeof(uint16_t) + sizeof(uint64_t);
    BYTE buff[length];
    uint16_t op = htobe16(SERVER_REPLY_OK);
    memcpy(buff, &op, sizeof(op));
    taskid = be64toh(taskid);
    memcpy(buff+2, &taskid, sizeof(uint64_t));

    // write the reply to the pipe and close it
    write(fd, buff, length);
    close_pipe(fd);
}

/* Returns the size of the file of descriptor fd. */
uint32_t get_size_file(int fd) {
    struct stat st;
    fstat(fd, &st);
    uint32_t size = st.st_size;
    return htobe32(size - 1); // need to remove the last \0
}

/* Writes a reply in the pipe fd for the STDOUT or STDERR request when
 there is the error errcode. */
void write_reply_std_error(uint16_t errcode, int fd) {
    BYTE buf[sizeof(uint16_t) * 2];
    uint16_t code = htobe16(SERVER_REPLY_ERROR);
    memcpy(buf, &code, sizeof(uint16_t));
    errcode = htobe16(errcode);
    memcpy(buf+sizeof(uint16_t), &errcode, sizeof(uint16_t));

    write(fd, buf, sizeof(uint16_t) *2);
}

/* Writes the reply on the reply pipe.
 If is_stdout : for STDOUT, otherwise, for STDERR
 First the function will try to open the "/stdout" (or "/stderr")
 file in the folder for taskid.
 If it can't open (the file doesn't exist) then the task was never
 run and the reply is an error.
 Otherwise, the content of the file is written (+ the OK code) */
void write_reply_std(uint64_t taskid, bool is_stdout) {
    int fd = open_reply_pipe_saturnd();

    char *folder_path = get_directory_id_path(taskid);
    DIR *dirp = opendir(folder_path);

    if (dirp == NULL) { // no task with this id
        write_reply_std_error(SERVER_REPLY_ERROR_NOT_FOUND, fd);
    } else {
        // get the name of the file
        char *filepath;
        if (is_stdout) {
            filepath = get_file_path(folder_path, "/stdout");
        } else {
            filepath = get_file_path(folder_path, "/stderr");
        }

        int filefd = open(filepath, O_RDONLY);
        if (filefd == -1) { // file doesn't exist == task was never run
            write_reply_std_error(SERVER_REPLY_ERROR_NEVER_RUN, fd);
        } else { // task was run, reply with the content of the file (+ the OK code)
            // make the first buffer with the repcode and the size of the string
            uint16_t code = htobe16(SERVER_REPLY_OK);
            uint32_t length = get_size_file(filefd);

            BYTE buf_numbers [sizeof(uint16_t) + sizeof(uint32_t)];
            memcpy(buf_numbers, &code, sizeof(uint16_t));
            memcpy(&buf_numbers[sizeof(uint16_t)], &length, sizeof(uint32_t));
            write(fd, &buf_numbers, sizeof(uint16_t) + sizeof(uint32_t));

            // write the contents to the pipe in chunks of 1024 bytes util the end of the file
            BYTE *buf = malloc(1024 * sizeof(BYTE));
            int nb_read = 1024;
            while (nb_read == 1024) {
                nb_read = read(filefd, buf, 1024);
                write(fd, buf, nb_read);
            }
            free(buf);
        }
        free(filepath);
        free(dirp);
    }
    free(folder_path);
    close_pipe(fd);
}

/* Writes the reply "OK" to the reply pipe
for the request TERMINATE */
void write_reply_terminate() {
    uint16_t code = htobe16(SERVER_REPLY_OK);
    int fd = open_reply_pipe_saturnd();
    write(fd, &code, sizeof(uint16_t));
}


/* Writes the reply for request LIST to the reply pipe.
- tasks : the array of all the tasks
- nb_tasks : the length of the array
All the tasks are returned, even the ones who were removed. */
void write_reply_l(s_task **tasks, uint32_t nb_tasks){
    uint16_t code = htobe16(SERVER_REPLY_OK);
    int fd = open_reply_pipe_saturnd();

    write(fd,&code,sizeof(uint16_t));

    uint32_t nb = htobe32(nb_tasks);
    write(fd,&nb,sizeof(uint32_t));


    for (uint32_t i = 0; i < nb_tasks; i++) {

        int length = sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t)  // timing fields
                + sizeof(uint32_t) // argc
                + sizeof(uint64_t); //id
        for (uint32_t j = 0; j < tasks[i]->command->argc; ++j) {
            // add the length of the string + 32 (storing the length)
            length += sizeof(uint32_t) + tasks[i]->command->argv[j]->length*sizeof(BYTE);
        }

        int current = 0;
        BYTE buff[length];

        //id
        uint64_t id = htobe64(tasks[i]->id);
        memcpy(buff+current, &id, sizeof(uint64_t));
        current += sizeof(uint64_t);

        // copy the timing
        uint64_t m = htobe64(tasks[i]->t->minutes);
        memcpy(buff+current, &m, sizeof(uint64_t));
        current += sizeof(uint64_t);
        
        uint32_t h = htobe32(tasks[i]->t->hours);
        memcpy(buff+current, &h, sizeof(uint32_t));
        current += sizeof(uint32_t);

        memcpy(buff+current, &tasks[i]->t->daysofweek, sizeof(uint8_t)); // no need to convert endian for days : there is only 1 byte
        current += sizeof(uint8_t);
       
        //argc
        uint32_t argc_tmp = htobe32(tasks[i]->command->argc);
        memcpy(buff+current, &argc_tmp, sizeof(uint32_t));
        current += sizeof(uint32_t);

        // copy command->argv
        for (int j = 0; j < tasks[i]->command->argc; j++) {
            string *str = tasks[i]->command->argv[j];

            uint32_t length_tmp = htobe32(str->length);
            memcpy(buff+current, &length_tmp, sizeof(uint32_t));
            current += sizeof(uint32_t);
            memcpy(buff+current, str->s, str->length*sizeof(BYTE));
            current += str->length*sizeof(BYTE);
        }
        // write the request
        write(fd, buff, length);
    }

    close_pipe(fd);  
    
}

/* Writes the reply for the TIME_AND_EXITCODES request to the reply pipe.
- fd : the fd of the reply pipe
- errcode : if this is an error code, then writes an error response with that code.
Otherwise, writes the normal response with :
- runs : the array of runs (contains a timestamp and an error code for each run)
- nb_runs : the length of the array */
void write_times_exitcodes(int fd, run **runs, uint32_t nb_runs, uint16_t errcode){
    if(errcode == SERVER_REPLY_OK){
        // create the buffer for the reply
        size_t length = sizeof(uint16_t) + sizeof(uint32_t)
                        + (nb_runs * (sizeof(int64_t) + sizeof(uint16_t)));
        BYTE buff[length];
        int current_size = 0;

        uint16_t rep = SERVER_REPLY_OK;
        memcpy(buff, &rep, sizeof(uint16_t));
        current_size += sizeof(uint16_t);

        uint32_t tmp = htobe32(nb_runs);
        memcpy(buff + current_size, &tmp, sizeof(uint32_t));
        current_size += sizeof(uint32_t);

        for (uint32_t i = 0; i < nb_runs; i++){
            int64_t time = be64toh(runs[i]->time);
            memcpy(buff+current_size, &time, sizeof(int64_t));
            current_size += sizeof(int64_t);

            memcpy(buff+current_size, &(runs[i]->exitcode), sizeof(uint16_t));
            current_size += sizeof(uint16_t);
        }

        // write to the pipe
        write(fd, buff, length);
        close_pipe(fd);

        // clean up
        for(int i = 0; i < nb_runs; i++) {
            free(runs[i]);
        }
        free(runs);
    } else {
        write_reply_code(fd, false, errcode);
    }
}

/* First gets all the infos about runs for the task number taskid.
Then writes the reply for the TIME_EXIT_CODES request to the reply
pipe. */
void write_reply_t_ec(uint64_t taskid){
    int fd = open_reply_pipe_saturnd();
    char* folder_path = get_directory_id_path(taskid);

    char* runs_path = get_file_path(folder_path, "/runs");
    char* nb_runs_path = get_file_path(folder_path, "/nb_runs");

    free(folder_path);

    int fd_runs = open(runs_path, O_RDONLY);
    if (fd_runs == -1) {
        write_times_exitcodes(fd, NULL, 0, SERVER_REPLY_ERROR_NOT_FOUND);
    } else {
        // read the number of runs
        int nb_runs_fd = open(nb_runs_path, O_RDONLY);
        uint32_t nb_runs;
        read(nb_runs_fd, &nb_runs, sizeof(uint32_t));
        close(nb_runs_fd);

        if (nb_runs_fd < 1) { // never run
            write_times_exitcodes(fd, NULL, 0, SERVER_REPLY_ERROR_NEVER_RUN);
        } else { // read the runs
            run **runs = malloc(sizeof(run*) * nb_runs);
            is_malloc_error(runs);

            // read timecode and exit code for each run
            for(uint32_t i = 0; i < nb_runs; i++){
                runs[i] = malloc(sizeof(int64_t) + sizeof(uint16_t));
                is_malloc_error(runs[i]);

                int64_t date;
                uint16_t exitcode;

                // read timecode
                int res = read(fd_runs, &date, sizeof(int64_t));
                if (res > 0) {
                    runs[i]->time = be64toh(date);
                    res = read(fd_runs, &exitcode, sizeof(uint16_t));
                    if (res <= 0){
                        perror("Reading error : can't read the return code from the runs file");
                        exit(EXIT_FAILURE);
                    } else {
                        runs[i]->exitcode = exitcode;
                    }
                } else {
                    perror("Reading error : can't read the timecode from the runs file");
                    exit(EXIT_FAILURE);
                }
            }
            // write the reply
            write_times_exitcodes(fd, runs, nb_runs, SERVER_REPLY_OK);
            // runs was already freeed in write_times_exitcodes
        }
    }
    free(runs_path);
    // no need to close the pipe "fd" : it was closed in the
    // other helper methods when they finished writing
}

/* First creates a "removed" file in the folder for taskid
(to show that the task is removed).
Then writes the reply to the reply pipe. */
void write_reply_rm(uint64_t taskid){
    int fd = open_reply_pipe_saturnd();
    char *folder_path = get_directory_id_path(taskid);

    DIR *d = opendir(folder_path);
    if(d) {
        char filename[] = "/removed";
        char* removed_path = get_file_path(folder_path, filename);

        int removed_fd = open(removed_path, O_CREAT);
        close(removed_fd);
        free(removed_path);
    } else {
        write_reply_code(fd, false, SERVER_REPLY_ERROR_NOT_FOUND);
    }

    free(folder_path);
    close_pipe(fd);
}
