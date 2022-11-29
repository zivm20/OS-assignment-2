#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

int copy(const char* f1, const char* f2){
    int fd1;
    int fd2;
    struct stat fStat;
    char* content;
    if(stat(f1,&fStat)<0){
        return -1;
    }
    if((fd1 = open(f1,O_RDONLY))<0){
        return -1;
    }    
    //st_size returns size of string without terminating char
    if((content = (char*) malloc(fStat.st_size+1)) == NULL){
        close(fd1);
        return -1;
    }
    read(fd1,content,fStat.st_size);
    
    //add terminating char
    content[fStat.st_size] = '\0';
    close(fd1);

    //write to new or existing file
    if((fd2 = open(f2,O_WRONLY | O_TRUNC | O_CREAT,S_IRUSR | S_IWUSR))<0){
        free(content);
        return -1;
    }
    write(fd2,content,fStat.st_size);
    close(fd2);
    free(content);

    return 1;
}

char* dir_command(){
    DIR* dir;
    struct dirent* dirElement;
    char* out = (char*) malloc(sizeof(char));
    size_t outlen = 1; 
    if((dir = opendir("./")) == NULL){
        return NULL;
    }
    
    while((dirElement = readdir(dir)) != NULL){
        size_t newSize = sizeof(char)*(1+outlen+strlen(dirElement->d_name));
        printf("%ld\n",newSize);
        if((out = (char*)realloc(out, newSize)) == NULL){
            closedir(dir);
            return NULL;
        }
        strcat(out, dirElement->d_name);
        strcat(out," ");
        outlen = newSize;
    }
    closedir(dir);
    return out;
}

char* commandHandler(char* in){
    char* command;
    if((command = strtok(in, " ")) == NULL){
        command = in;
    }
    if(strcmp("DIR",command) == 0){
        return dir_command();
    }
    else if(strcmp("COPY",command) == 0){
        char* f1;
        char* f2;
        
        if((f1 = strtok(NULL, " ")) == NULL){
            return NULL;
        }
        if((f2 = strtok(NULL, " ")) == NULL){
            return NULL;
        }
        copy(f1,f2);
        return NULL;
        
    }
}

int main(int argc, char* argv[]){
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    printf("shell: ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        char* output = commandHandler(strtok(line,"\n"));
        if(output != NULL){
            printf("> %s",output);
            free(output);    
        }
        
        printf("\nshell: ");
    }

    free(line);
    return 1;
}

