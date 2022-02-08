#include "common-folder.h"

void is_mkdir_error(int res){
    if (res == -1) {
        switch (res) {
            case EACCES :
                printf("the parent directory does not allow write");
                exit(EXIT_FAILURE);
            case EEXIST:
                printf("pathname already exists");
                exit(EXIT_FAILURE);
            case ENAMETOOLONG:
                printf("pathname is too long");
                exit(EXIT_FAILURE);
            default:
                perror("mkdir");
                exit(EXIT_FAILURE);
        }
    }
}

/* Returns the default path for the file : "/tmp/<USERNAME>/tasks/id/..." = directory/file */
char* get_file_path(char *directory, char *file) {

    char *name_file = malloc((strlen(directory) + strlen(file) +1) * sizeof(char));
    is_malloc_error(name_file);

    strcpy(name_file, directory);
    strcat(name_file, file);

    return name_file;
}

/* Returns the default path for the tasks directory : "/tmp/<USERNAME>/saturnd" */
char* get_directory_path() {
    // get the username (smaller than 200 chars)
    char *username = malloc(200 * sizeof(char));
    is_malloc_error(username);
    getlogin_r(username, 200);

    char buf1[] = "/tmp/";
    char buf2[] = "/saturnd";

    char *id_directory = malloc((strlen(username) + strlen(buf1) + strlen(buf2) + 1) * sizeof(char) + sizeof(uint64_t));
    is_malloc_error(id_directory);

    strcpy(id_directory, buf1);
    strcat(id_directory, username);
    strcat(id_directory, buf2);

    free(username);
    return id_directory;
}

/* Returns the default path for the tasks directory : "/tmp/<USERNAME>/saturnd/tasks" */
char* get_directory_tasks_path() {
    // get the username (smaller than 200 chars)
    char *username = malloc(200 * sizeof(char));
    is_malloc_error(username);
    getlogin_r(username, 200);

    char buf1[] = "/tmp/";
    char buf2[] = "/saturnd/tasks";

    char *id_directory = malloc((strlen(username) + strlen(buf1) + strlen(buf2) + 1) * sizeof(char) + sizeof(uint64_t));
    is_malloc_error(id_directory);

    strcpy(id_directory, buf1);
    strcat(id_directory, username);
    strcat(id_directory, buf2);

    free(username);
    return id_directory;
}


/* Returns the default path for the pipes directory : "/tmp/<USERNAME>/saturnd/tasks/id" */
char* get_directory_id_path(uint64_t id) {
    char *d = get_directory_path();
    char *a = "/tasks/";

    char *ids = malloc(sizeof(uint64_t)*sizeof(char));
    is_malloc_error(ids);
    sprintf(ids,"%lu",id);

    char *id_directory = malloc((strlen(d) + strlen(a) + strlen(ids) + 1) * sizeof(char));
    is_malloc_error(id_directory);

    strcpy(id_directory, d);
    strcat(id_directory, a);
    strcat(id_directory, ids);

    free(ids);
    free(d);
    return id_directory;
}