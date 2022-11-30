#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdint.h>

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

void commandRunner(char* in){
    
    char command[256];
    sscanf(in,"%s",command);
    /*if (sscanf(in,"%s",command) == NULL){
        command = in;
    }
    */
    
    if (strcmp("DIR",command) == 0){
        printf("%s",dir_command());
    }
    else if(strcmp("COPY",command) == 0){
        char f1[256];
        char f2[256];
        sscanf(in,"%s %s %s",command, f1, f2);
        
        copy(f1,f2);
        exit(1);
    }
    else{
        char** newArgv;
        int argc = 0;
        char* arg = strtok(in," ");
        char newCommand[266];
        newArgv = (char**) malloc(sizeof(char*)*2);
        sprintf(newCommand, "/usr/bin/%s",command);
        newArgv[argc] = newCommand;
        argc++;
        while((arg = strtok(NULL, " "))){
            
            if((newArgv = realloc(newArgv, sizeof(char*)*(argc+2)))==NULL){
                exit(-1);
            }
            /*
            if((newArgv[argc] = realloc(newArgv, sizeof(char)*(strlen(arg))))==NULL){
                exit(-1);
            }*/
            newArgv[argc] = arg;
            //strcpy(newArgv[argc],arg);
            argc++;
        }
        newArgv[argc] = NULL;
        char* newEnv[] = {NULL};
        execve(newCommand, newArgv, newEnv);
        
    }
}

char* singleCommandHandler(char* fullCommand){
   
    int outputPipe[2];
    
    if (pipe(outputPipe) == -1) {
        return NULL;
    }
    pid_t childId;
    if((childId = fork())==-1){
        return NULL;
    }
    if(childId == 0){
        
        char* out;
        char buf;
        
        close(outputPipe[0]);

       
        dup2(outputPipe[1],STDOUT_FILENO);
        close(outputPipe[1]);
        commandRunner(fullCommand);
        exit(0);
    }
    else{
        char buf;
        close(outputPipe[1]);
        wait(NULL);
        
        char* out = (char*) malloc(sizeof(char));
        size_t outLen = 0;
        while (read(outputPipe[0], &buf, 1) > 0){
            if((out = (char*)realloc(out, outLen+1)) == NULL){
                close(outputPipe[0]);
                perror("read pipe realloc");
                exit(EXIT_FAILURE);
            }
            out[outLen] = buf;
            outLen++;
        }
        close(outputPipe[0]);
        if(outLen == 0){
            free(out);
            return NULL;
        }
        return out;
        
    }
    return NULL;
}

int main(int argc, char* argv[]){
    char *fullCommand = NULL;
    size_t len = 0;
    ssize_t readSize;
    printf("shell: ");
    
    while ((readSize = getline(&fullCommand, &len, stdin)) != -1) {
        char* output = "";
        fullCommand = strtok(fullCommand,"\n");
        
        //output = singleCommandHandler(strcat(strcat(command," "),output));
        output = singleCommandHandler(fullCommand);
        if(output != NULL){
            printf("> %s\n",output);
            free(output);
        }
        printf("shell: ");
        
    }

    free(fullCommand);
    return 1;
}

