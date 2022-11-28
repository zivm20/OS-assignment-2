#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


//copy from string to file
int copy(const char* src, const char* file, const size_t len, const int append){
    
    int fd;
    
    //write to new/existing file, or append
    if(!append){
        if((fd = open(file, O_WRONLY | O_TRUNC | O_CREAT,S_IRUSR | S_IWUSR))<0){
            return -1;
        }
    }
    else if((fd = open(file, O_WRONLY | O_APPEND))<0){
        return -1;
    }
    write(fd, src, len);
    close(fd);

    return 1;
}


int main(int argc, char* argv[]){
    char *line = NULL;
    size_t len = 0;
    int lineNum = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("%s", line);
        for(int i=1; i<argc; i++){
            if(copy(line,argv[i],read,lineNum)<0){
                free(line);
                printf("unexpected error when copying");
                return -1;
            }
        }
        lineNum++;
    }

    free(line);
    return 1;
}



