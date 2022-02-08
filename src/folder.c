#include "folder.h"


/* Creates the following folders hierarchies if they don't exist :
"/tmp/<USERNAME>/saturnd/tasks" and "/tmp/<USERNAME>/saturnd/tasks" */
void create_intermediaire_folders(){
    char *username = malloc(200 * sizeof(char));
    is_malloc_error(username);
    getlogin_r(username, 200);
    
    char *tmp1 = "/tmp";
    char *tmp2 = "/saturnd";
    char *tmp3 = "/";
    char *tmp4 = "/tasks";
    char *tmp5 = "/pipes";
    char *name = malloc( (strlen(tmp1) + strlen(tmp2) + strlen(tmp3) + strlen(username) + strlen(tmp4) + 1 )* sizeof(char));
    char *name2 = malloc( (strlen(tmp1) + strlen(tmp2) + strlen(tmp3) + strlen(username) + strlen(tmp5) + 1 )* sizeof(char));
    strcpy(name,tmp1);
    strcpy(name2,tmp1);
    struct stat st = {0};
    if (stat(name, &st) == -1) {
         is_mkdir_error(mkdir(name, 0700));
    }
    strcat(name,tmp3);
    strcat(name2,tmp3);
    strcat(name,username);
    strcat(name2,username);
    if (stat(name, &st) == -1) {
         is_mkdir_error(mkdir(name, 0700));
    }
    strcat(name, tmp2);
    strcat(name2, tmp2);
    if (stat(name, &st) == -1) {
         is_mkdir_error(mkdir(name, 0700));
    }
    strcat(name, tmp4);
    strcat(name2, tmp5);
    if (stat(name, &st) == -1) {
         is_mkdir_error(mkdir(name, 0700));
    }
    if (stat(name2, &st) == -1) {
         is_mkdir_error(mkdir(name2, 0700));
    }

    free(username);
    free(name);
    free(name2);
}

/* Creates the folder hierarchy at startup if necessary. */
void create_files(){

    create_intermediaire_folders();

    // last_taskid file contains -1 (if it didn't exist, then no tasks existed either)
    char *dir = get_directory_path();
    char *last_task = get_file_path(dir,"/last_taskid");
    int fp = open(last_task, O_RDWR);
    if (fp == -1){
        int fd = open(last_task, O_CREAT | O_RDWR, S_IRWXU);
        uint64_t id = -1;
        write(fd, &id ,sizeof(uint64_t));
        close(fd);
    }
    close(fp);
    free(last_task);
    free(dir);
}
