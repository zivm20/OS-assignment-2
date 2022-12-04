#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netinet/tcp.h> 



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

void dir_command(){
    DIR* dir;
    struct dirent* dirElement;
    
    if((dir = opendir("./")) == NULL){
        exit(EXIT_FAILURE);
    }
    while((dirElement = readdir(dir)) != NULL){
       
        printf("%s ",dirElement->d_name);
    }
    printf("\n");
    closedir(dir);
    
}

void commandRunner(char* fullCommand){
    char* command;
    
    command = strtok_r(fullCommand," ",&fullCommand);
    

    if (strcmp("DIR",command) == 0){
        dir_command();
        exit(1);
    }
    else if(strcmp("COPY",command) == 0){
        char* f1;
        char* f2;
        if((f1 = strtok_r(fullCommand," ",&fullCommand))==NULL){
            perror("Usage: COPY <src> <dest>\n");
            return;
        }
        if((f2 = strtok_r(fullCommand," ",&fullCommand))==NULL){
            perror("Usage: COPY <src> <dest>\n");
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
       
        newArgv[0] = command;
        

        while((arg = strtok_r(fullCommand," ",&fullCommand))){
            if((newArgv = realloc(newArgv, sizeof(char*)*(argc+1)))==NULL){
                perror("cannot realloc newArg\n");
                exit(EXIT_FAILURE);
            }
            newArgv[argc] = arg; 
            argc++;
            
        }

        if((newArgv = realloc(newArgv, sizeof(char*)*(argc+1)))==NULL){
            perror("cannot realloc newArg\n");
            exit(EXIT_FAILURE);
        }
        newArgv[argc] = NULL;
        char* newEnv[] = {NULL};
        execve(command, newArgv, newEnv);
        perror("command not found!\n");
        exit(EXIT_FAILURE);
        
    }
}

char* singleCommandHandler(char* fullCommand,char* extraInput){
    
    int outputPipe[2];
    
    if (pipe(outputPipe) == -1) {
        return NULL;
    }
    //create child process
    pid_t childId;
    if((childId = fork())==-1){
        close(outputPipe[0]);
        close(outputPipe[1]);
        return NULL;
    }
    if(childId == 0){
        char* out;
        char buf;
        //redirect stdin as pipe read side
        dup2(outputPipe[0],STDIN_FILENO);
        close(outputPipe[0]);
        //redirect stdout as pipe write side
        dup2(outputPipe[1],STDOUT_FILENO);
        close(outputPipe[1]);
        commandRunner(fullCommand);
        //should never reach here
        exit(EXIT_FAILURE);
    }
    else{
        char buf;
        //write our desired stdin data
        if(extraInput != NULL)
            write(outputPipe[1],extraInput,strlen(extraInput));
        close(outputPipe[1]);
        //wait for child process to terminate
        wait(NULL);
        
        char* out = (char*) malloc(sizeof(char));
        size_t outLen = 0;
        //read the output from the child process
        while (read(outputPipe[0], &buf, 1) > 0){
            if((out = (char*)realloc(out, outLen+1)) == NULL){
                close(outputPipe[0]);
                perror("read pipe realloc\n");
                exit(EXIT_FAILURE);
            }
            out[outLen] = buf;
            outLen++;
        }
        //add terminating char
        if((out = (char*)realloc(out, outLen+1)) == NULL){
            close(outputPipe[0]);
            perror("read pipe realloc\n");
            exit(EXIT_FAILURE);
        }
        out[outLen] = '\0';

        close(outputPipe[0]);
        //return child process output
        if(outLen == 0){
            return NULL;
        }
        return out;
        
    }
    return NULL;
}

char* pipeHandler(char* line, char* extraInput){
    char* command;
    size_t nCommands = 0;
    size_t commandsSize = 1;
    //recursive calls for piping
    if((command = strtok_r(line,"|",&line))!=NULL){
        
        return pipeHandler(line,singleCommandHandler(command,extraInput));
    }
    //command is null so we have ran a command, or gotten an empty command so we return
    //our extra input
    return extraInput;

}



int main(int argc, char* argv[]){
    char *line = NULL;
    char* tempLine;
    size_t len = 0;
    ssize_t readSize;
    printf(">");
    
    
    while ((readSize = getline(&line, &len, stdin)) != -1) {
        char* output = NULL;
        char* extraInput = NULL;
        int outFD = -1;
        line = strtok_r(line,"\n",&line);
        tempLine = strdup(line);
        char* commands = NULL;
        
        //file input
        if(strchr(tempLine,'<') != NULL){
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
                //cleaning commands from input char
                if((commands = (char*) realloc(commands,sizeof(char)*(strlen(commands)+strlen(tempLine)+1)))==NULL){
                    free(tempLine);
                    return -1;
                }
                commands = strcat(commands,tempLine);
                tempLine = strdup(commands);
            }
        }
        //open server and get client output as input
        else if(strchr(tempLine,'{') != NULL){
            char* strPort;
            commands = strtok_r(tempLine,"{",&tempLine);
            if((strPort = strtok_r(tempLine," \n",&tempLine))!=NULL){
                int port = atoi(strPort);
                char* ip = "127.0.0.1";
            
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == -1) {
                    perror("socket\n");
                    free(tempLine);
                    return -1;
                } 
                int t = 1;
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &t, sizeof(t));
                
                //bind socket
                struct sockaddr_in server;
                memset(&server,0,sizeof(server));
                server.sin_port = htons(port);
                if(inet_pton(AF_INET,ip,&server.sin_addr)<=0){
                    perror("inet_pton\n");
                    free(tempLine);
                    return -1;
                }
                server.sin_family = AF_INET;
                int connection = bind(sock,(struct sockaddr*)&server,sizeof(server));
                if(connection == -1){
                    perror("Binding failed!\n");
                    free(tempLine);
                    return -1;
                }
                
                //start listening
                if (listen(sock, 500) == -1){
                    perror("Listen failed!\n");
                    free(tempLine);
                    return -1;
                }

                //accept first client
                struct sockaddr_in clientAddress;
                socklen_t clientAddressLen = sizeof(clientAddress);
                memset(&clientAddress, 0, sizeof(clientAddress));
                clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLen);
                
                //reading input from socket
                extraInput = (char*) malloc(sizeof(char));
                size_t eiLen = 0;
                char buf;
                while (recv(clientSocket,&buf,sizeof(char),0) > 0){
                    if((extraInput = (char*)realloc(extraInput, eiLen+1)) == NULL){
                        close(sock);
                        free(tempLine);
                        perror("extraInput realloc\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("%c(%d)",buf,buf);
                    extraInput[eiLen] = buf;
                    eiLen++;
                }
                close(sock);
                //cleaning commands from input char
                if((commands = (char*) realloc(commands,sizeof(char)*(strlen(commands)+strlen(tempLine)+1)))==NULL){
                    free(tempLine);
                    return -1;
                }
                commands = strcat(commands,tempLine);
                tempLine = strdup(commands);

            }
        }
        
        //file output
        if(strchr(tempLine,'>') != NULL){
            
            commands = strtok_r(tempLine,">",&tempLine);
            
            char* outputFile;
            if((outputFile = strtok_r(tempLine,">",&tempLine))!=NULL){
                //set outFD to our wanted output file name
                if((outFD = open(outputFile,O_WRONLY | O_TRUNC | O_CREAT,S_IRUSR | S_IWUSR))<0){
                    printf("failed to open file");
                    free(tempLine);
                    return -1;
                }
            }
        }
        //output to server
        else if(strchr(tempLine,'}') != NULL){
            char* ipAndPort;
            commands = strtok_r(tempLine,"}",&tempLine);
            if((ipAndPort = strtok_r(tempLine," \n",&tempLine))!=NULL){
            
                char* ip = strtok_r(ipAndPort,":",&ipAndPort);;
                int port = atoi(ipAndPort);

                //server socket
                struct sockaddr_in server;
                memset(&server,0,sizeof(server));
                server.sin_port = htons(port);
                if(inet_pton(AF_INET,ip,&server.sin_addr)<=0){
                    perror("inet_pton\n");
                    return -1;
                }
                server.sin_family = AF_INET;
                //outFD will be the socket we wish to write stdout to
                outFD = socket(AF_INET, SOCK_STREAM, 0);
                if (outFD == -1) {
                    perror("socket\n");
                    return -1;
                } 
                int connection = connect(outFD,(struct sockaddr*)&server,sizeof(server));
                if(connection == -1){
                    perror("connection failed!\n");
                }
            }
        }
        
        //if commands hasn't been initialized, init commands
        if(commands == NULL){
            commands = tempLine;
        }
        //get the output of the program
        output = pipeHandler(commands,extraInput);
        
        if(output != NULL){
            //write the output to the correct file descriptor
            if(outFD != -1){
                
                write(outFD,output,strlen(output));
                close(outFD);
            }
            else{
                printf("%s",output);
            }
           
            free(output);
            free(extraInput);
        }
        else{
            if(outFD != -1){
                close(outFD);
            }
        }
        
        printf(">");
        
    }
    
    free(line);
    return 1;
}

