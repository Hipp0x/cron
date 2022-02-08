#include "pipes.h"

/** Terminates the program if the pointer is NULL. */
void is_malloc_error2(void *p) {
    if (p == NULL) {
        perror("Malloc failure");
        exit(EXIT_FAILURE);
    }
}

/* Opens the pipe and returns the file descriptor.
- name = the path to the pipe
- flag = the permissions
Function fails if the pipe can't be opened. */
int open_pipe(char *name, int flags) {
    int fd = open(name, flags);
    if (fd == -1) {
        fprintf(stderr, "Can't open pipe : %s with flag %d => %s\n", name, flags, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}


/* Closes the pipe. Function fails if it can't be closed. */
void close_pipe(int fd) {
    int ret = close(fd);
    if (ret == -1){
        perror("Can't close the pipe ");
        exit(EXIT_FAILURE);
    }
}

/* Returns the default path for the pipes directory : "/tmp/<USERNAME>/saturnd/pipes" */
char* write_default_pipes_directory() {
	// get the username (smaller than 200 chars)
	char *username = malloc(200 * sizeof(char));
	is_malloc_error2(username);
	getlogin_r(username, 200);

	char buf1[] = "/tmp/";
    char buf2[] = "/saturnd/pipes/";

	char *pipes_directory = malloc((strlen(username) + strlen(buf1) + strlen(buf2) +1) * sizeof(char));
    is_malloc_error2(pipes_directory);

	strcpy(pipes_directory, buf1);
	strcat(pipes_directory, username);
    strcat(pipes_directory, buf2);

    free(username);
    return pipes_directory;
}

/* Returns the name of the pipe : pipe_directory/basename
Adds a '/' between the pipes_directory and the basename if necessary.
- basename should be either "saturnd-request-pipe" or "saturnd-reply-pipe"
Assumes pipes_directory is NOT null.
*/
char *get_pipe_name(char *pipes_directory, char *basename) {
    char slash[] = "/";
    char *name;

    if (pipes_directory[strlen(pipes_directory)-1] == '/') {
       name = malloc ((strlen(pipes_directory) + strlen(basename) +1) * sizeof(char));
       is_malloc_error2(name);
       strcpy(name, pipes_directory);
    } else { // need to add a "/" between dir name and basename
       name = malloc ((strlen(pipes_directory) + strlen(basename) + 2) * sizeof(char));
       is_malloc_error2(name);
       strcpy(name, pipes_directory);
       strcat(name, slash);
    }
    strcat(name, basename);
    return name;
}


/* Returns 1 if the pipe exists, 0 otherwise
* - name : the path to the pipe
*/
int test_pipe_exists(char *name) {
    int fd = open(name, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        return 0;
    } else {
        close(fd);
        return 1;
    }
}

/* Creates the pipes at the default /tmp/<USERNAME>/saturnd/pipes location
 * if they don't already exist
 */
void create_pipes() {
    // find the names
    char *pipes_directory = write_default_pipes_directory();
    char *pipe_req = get_pipe_name(pipes_directory, "saturnd-request-pipe");
    char *pipe_rep = get_pipe_name(pipes_directory, "saturnd-reply-pipe");

    // test request pipe
    if (! test_pipe_exists(pipe_req)) {
        mkfifo(pipe_req, S_IRWXU);
        
    }
    // test the reply pipe
    if (! test_pipe_exists(pipe_rep)) {
        mkfifo(pipe_rep, S_IRWXU);
    }

    free(pipe_rep);
    free(pipe_req);
    free(pipes_directory);
}

/* Opens the reply pipe for saturnd with O_WRONLY permissions */
int open_reply_pipe_saturnd() {
     char *pipes_directory = write_default_pipes_directory();
     char *reply_pipe_name = get_pipe_name(pipes_directory, "saturnd-reply-pipe");
     int fd_rep = open_pipe(reply_pipe_name, O_WRONLY);

     free(pipes_directory);
     free(reply_pipe_name);
     return fd_rep;
}