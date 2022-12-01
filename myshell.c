#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdint.h>


char** appendStr(char** arr, char* str, size_t* nArr, size_t* arrSize){
    if(*nArr == *arrSize){
        arr = (char**) realloc(arr,sizeof(char*)*(*arrSize+1));
        *arrSize++;
    }
    arr[*nArr] = str;
    *nArr++;
    return arr;
}

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

void commandRunner(char* fullCommand){
    char* command;
    command = strtok_r(fullCommand," ",&fullCommand);
    //printf("running: %s\n",fullCommand);
    if (strcmp("DIR",command) == 0){
        printf("%s",dir_command());
    }
    else if(strcmp("COPY",command) == 0){
        char* f1;
        char* f2;
        if((f1 = strtok_r(fullCommand," ",&fullCommand))==NULL){
            perror("Usage: COPY <src> <dest>");
            return;
        }
        if((f2 = strtok_r(fullCommand," ",&fullCommand))==NULL){
            perror("Usage: COPY <src> <dest>");
            return;
        }
        copy(f1,f2);
        exit(1);
    }
    else{
        char** newArgv;
        size_t argc = 1;
        
        char* arg;
        
        newArgv = (char**) malloc(sizeof(char*));
        char* temp = strdup("/usr/bin/");
        temp = (char*) realloc(temp,sizeof(char)*(strlen(temp)+strlen(command)+1));
        command = strcat(temp,command);
        //printf("command now is: %s\n",command);
        newArgv[0] = command;
        //printf("arg: %s, argc: %ld\n",newArgv[argc-1],argc);

        while((arg = strtok_r(fullCommand," ",&fullCommand))){
            if((newArgv = realloc(newArgv, sizeof(char*)*(argc+1)))==NULL){
                perror("cannot realloc newArg");
                exit(-1);
            }
            newArgv[argc] = arg; 
            argc++;
            //printf("arg: %s, argc: %ld\n",newArgv[argc-1],argc);
        }

        if((newArgv = realloc(newArgv, sizeof(char*)*(argc+1)))==NULL){
            perror("cannot realloc newArg");
            exit(-1);
        }
        newArgv[argc] = NULL;
        char* newEnv[] = {NULL};
        execve(command, newArgv, newEnv);
        
    }
}

char* singleCommandHandler(char* fullCommand,char* extraArg){
    
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
        dup2(outputPipe[0],STDIN_FILENO);
        close(outputPipe[0]);
        dup2(outputPipe[1],STDOUT_FILENO);
        close(outputPipe[1]);
        commandRunner(fullCommand);
        exit(0);
    }
    else{
        char buf;
        
        write(outputPipe[1],extraArg,strlen(extraArg));
    
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
        //free(fullCommand);
        if(outLen == 0){
            //free(out);
            return NULL;
        }
        return out;
        
    }
    return NULL;
}


char* parseLine(char* line, char* extraArg){
    char* command;
    size_t nCommands = 0;
    size_t commandsSize = 1;
    //printf("line: [%s], char: %d\n",line, line[0]);
    if(strlen(line) > 0 && (command = strtok_r(line,"|",&line))!=NULL){
        //printf("command: [%s], rest: [%s],\n", command, line);
        //return parseLine(line,command,outputFD);
        //printf("2 command: [%s], rest: [%s],\n", command, line);
        return parseLine(line,singleCommandHandler(command,extraArg));
    }
    return extraArg;

}

int main(int argc, char* argv[]){
    char *line = NULL;
    size_t len = 0;
    ssize_t readSize;
    printf("shell: ");
    
    
    while ((readSize = getline(&line, &len, stdin)) != -1) {
        char* output = "";
        char* extraInput = "";
        int outFD = -1;
        line = strtok_r(line,"\n",&line);
        char* tempLine = strdup(line);
        char* commands;
        char* inputFile;
        commands = strtok_r(tempLine,"<",&tempLine);
        if((inputFile = strtok_r(tempLine," \n",&tempLine))!=NULL){
            int inFD = -1;
            struct stat fStat;
            if(stat(inputFile,&fStat)<0){
                free(tempLine);
                return -1;
            }
            if((inFD = open(inputFile,O_RDONLY))<0){
                free(tempLine);
                return -1;
            }    
            //st_size returns size of string without terminating char
            if((extraInput = (char*) malloc(fStat.st_size+1)) == NULL){
                close(inFD);
                free(tempLine);
                return -1;
            }
            read(inFD,extraInput,fStat.st_size);
            
            //add terminating char
            extraInput[fStat.st_size] = '\0';
            close(inFD); 
            
            if((commands = (char*) realloc(commands,sizeof(char)*(strlen(commands)+strlen(tempLine)+1)))==NULL){
                free(tempLine);
                return -1;
            }
            
            commands = strcat(commands,tempLine);
        }
        //free(tempLine);
        
        tempLine = strdup(commands);
        commands = strtok_r(tempLine,">",&tempLine);
        char* outputFile;
        if((outputFile = strtok_r(tempLine,">",&tempLine))!=NULL){
            if((outFD = open(outputFile,O_WRONLY | O_TRUNC | O_CREAT,S_IRUSR | S_IWUSR))<0){
                free(tempLine);
                return -1;
            }
        }

        output = parseLine(commands,extraInput);
        //free(commands);
        if(output != NULL){
            if(outFD != -1){
                write(outFD,output,strlen(output));
                close(outFD);
            }
            else{
                printf("%s",output);
            }
            free(output);
        }
        else{
            if(outFD != -1){
                close(outFD);
            }
        }
        printf("shell: ");
        
    }

    free(line);
    return 1;
}

