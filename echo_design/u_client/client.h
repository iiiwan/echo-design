#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <string.h>  
#include <unistd.h>


#define MAXLINE 80  
 
char *client_path = "/home/johnson/echo_design/domain_socket/client.socket";  
char *server_path = "/home/johnson/echo_design/domain_socket/server.socket";  



void client_run_socket(void){  
    struct  sockaddr_un cliun, serun;  
    int len;  
    char buf[100];  
    int sockfd, n;  
 
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){  
        perror("client socket error");  
        exit(1);  
    }  
      
    //bind
    memset(&cliun, 0, sizeof(cliun));  
    cliun.sun_family = AF_UNIX;  
    strcpy(cliun.sun_path, client_path);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cliun.sun_path);  
    unlink(cliun.sun_path);  
    if (bind(sockfd, (struct sockaddr *)&cliun, len) < 0) {  
        perror("bind error");  
        exit(1);  
    }  
    
 	//connect
    memset(&serun, 0, sizeof(serun));  
    serun.sun_family = AF_UNIX;  
    strcpy(serun.sun_path, server_path);  
    len = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);  
    if (connect(sockfd, (struct sockaddr *)&serun, len) < 0){  
        perror("connect error");  
        exit(1);  
    }

    read(sockfd,buf,sizeof(buf));
    printf("%s \n",buf);
    if(strncmp(buf,"The",3) == 0){
         close(sockfd);
         return ;  
    }
 	//communication
    while(1) {
        memset(buf,0,sizeof(buf));

        //Send the message
        printf("Client > ");
        fgets(buf,sizeof(buf)-1,stdin);    
        n = write(sockfd, buf, strlen(buf));    
        if ( n < 0 ) {    
            perror("write error!");
            close(sockfd);
            exit(1);    
        }

        //Read the message
        if(strncmp(buf,"quit",4) == 0){
            printf("Disconnected from the server!\n");
            break;
        }
        read(sockfd,buf,sizeof(buf));
        printf("Echo: %s",buf);
    }   
    close(sockfd);  
}


#endif /* client.h */
