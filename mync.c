#include <sys/socket.h> 
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <fcntl.h>



int send_data(const char* content, const int sock, const size_t len){
    
    int char_sent = 0;
    while(char_sent < len){
        char_sent += send(sock, content+char_sent, len-char_sent, 0);
        if(char_sent == -1){
            perror("failed to send conetent!\n");
            return -1;
        }
        
    }
    return 1;
    
}



int main(int argc, char **argv) {
    int sender = 1;
    int opt;
     while((opt=getopt(argc,argv, "l")) != -1){
        switch(opt){
            case 'l':
                sender=0;
                break;
            default:
                printf("Usage: mync <IP or -l><port>\n");
                return -1;
        }
    }
    
    if(sender){
        int port = atoi(argv[2]);
        char* ip = argv[1];
        

        //server socket
        struct sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_port = htons(port);
        if(inet_pton(AF_INET,ip,&server.sin_addr)<=0){
            perror("inet_pton\n");
            return -1;
        }
        server.sin_family = AF_INET;
        
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&line, &len, stdin)) != -1) {
            //open TCP socket
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == -1) {
                perror("socket\n");
                return -1;
            } 
            int connection = connect(sock,(struct sockaddr*)&server,sizeof(server));
            if(connection == -1){
                perror("connection failed!\n");
            }
        
            
            if(send_data(line,sock,read)<0){
                free(line);
                close(sock);
                return -1;
            }
            close(sock);
        }
        free(line);
    }
    else{
        
        int port = atoi(argv[2]);
        char* ip = "127.0.0.1";
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            perror("socket\n");
            return -1;
        } 
        

        //bind socket
        struct sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_port = htons(port);
        if(inet_pton(AF_INET,ip,&server.sin_addr)<=0){
            perror("inet_pton\n");
            return -1;
        }
        server.sin_family = AF_INET;
        int connection = bind(sock,(struct sockaddr*)&server,sizeof(server));
        if(connection == -1){
            perror("Binding failed!\n");
            return -1;
        }

        //start listening
        if (listen(sock, 500) == -1){
            perror("Listen failed!\n");
            return -1;
        }
        
        char c;
        //first packet is a packet containing the size of the data
        while(1){
            //accept client connection
            struct sockaddr_in clientAddress;
            socklen_t clientAddressLen = sizeof(clientAddress);
            memset(&clientAddress, 0, sizeof(clientAddress));
            clientAddressLen = sizeof(clientAddress);
            int clientSocket = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLen);

            if (clientSocket == -1){
                perror("Failed to accept client\n");
                return -1;
            }
            //receive the message from the client
            printf("OUTPUT:");
            while(recv(clientSocket,&c,sizeof(char),0) > 0){
                printf("%c",c);
            }

            
        }
        
        close(sock); 
         
    }

    return 0;
}
    
