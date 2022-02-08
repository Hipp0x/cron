#include "create-task.h"

/*
 * Updates the last_taskid file with id.
 */
void update_last_id (char *file, uint64_t id){
    // delete the old file
    int r = remove(file);
    if (r == -1) {
        printf("echec de la suppression du fichier : %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    // recreate the file
    int fd = open(file, O_CREAT | O_RDWR, S_IRWXU);
    // write the new value
    write(fd,&id,sizeof(uint64_t));
    close(fd);
}


/*
 * Returns the next available id
 */
uint64_t get_avalaible_id (char *file){
    // read the last used id
    uint64_t id;
    int fd = open(file, O_RDONLY);
    read(fd,&id,sizeof(uint64_t));
    close(fd);
    // the next available id is +1
    id = id + 1;
    return id;
}

/* Creates a task (and its folder and files) and returns its id */
uint64_t create_new_task(struct timing *t, uint32_t length, string **s) {
    char *dir = get_directory_path(); //general directory
    char *file_last_id = get_file_path(dir, "/last_taskid"); //for last_taskid
    // get the next available id
    uint64_t id = get_avalaible_id(file_last_id);

    // create the folder for the task
    char *directory_name = get_directory_id_path(id); //for id directory
    is_mkdir_error(mkdir(directory_name, 0700));

    // timing" file contains the 3 fields of the struct timing
    char *file_timing = get_file_path(directory_name, "/timing");
    int fd = open(file_timing,O_CREAT,S_IRWXU);
    close(fd);
    fd = open(file_timing, O_WRONLY);
    size_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t);
    BYTE buff[len];

    size_t sze = 0;
    uint64_t minutes = htobe64(t->minutes);
    memcpy(buff, &minutes, sizeof(uint64_t));
    uint32_t hours = htobe32(t->hours);
    sze += sizeof(uint64_t);
    memcpy(buff+sze, &hours, sizeof(uint32_t));
    uint8_t days = t->daysofweek;
    sze += sizeof(uint32_t);
    memcpy(buff+sze, &t->daysofweek, sizeof(uint8_t)); // no need to convert endian for days : there is only 1 byte
    write(fd, buff, sizeof(buff));
    close(fd);

    // "argv" containts the commandline struct
    char *file_argv = get_file_path(directory_name, "/argv");   
    fd = open(file_argv,O_CREAT,S_IRWXU);
    fd = open(file_argv, O_WRONLY);
    size_t size = sizeof(uint32_t);
    for (int i = 0; i < length; i++) {
        size += sizeof(uint32_t);
        size += (s[i]->length) * sizeof(char);
    }
    char *buf = malloc(size);
    sze = 0;
    uint32_t le = htobe32(length);
    memcpy(buf + sze,&length,sizeof(uint32_t));
    sze += sizeof(uint32_t);
    for (int i = 0; i < length; i++) {
        uint32_t ta = (s[i]->length);
        memcpy(buf + sze,&ta,sizeof(uint32_t));
        sze += sizeof(uint32_t);
        memcpy(buf + sze,s[i]->s,sizeof(char)*(s[i]->length));
        sze += sizeof(char)*(s[i]->length);
    }
    write(fd,buf,size);
    close(fd);
    free(buf);

    // "runs" file contains the timestamps and return codes of all the executions
    char *file_runs = get_file_path(directory_name, "/runs");
    fd = open(file_runs,O_CREAT,S_IRWXU);
    close(fd); 

    // update the last_taskid file
    update_last_id(file_last_id, id);

    free(dir);
    free(file_last_id);
    free(directory_name);
    free(file_timing);
    free(file_argv);

    free(file_runs);
    free(t);
    for (int i = 0; i < length; i++){
        free(s[i]->s);
        free(s[i]);
    }
    free(s);

    return id;
}