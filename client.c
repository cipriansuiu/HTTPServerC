//
// Created by Ciprian on 10/17/17.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <memory.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int secondmain(int argc,char *argv[]) {
    int sockfd, newsockfd, portno, clilen,n;
    char buffer[256];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if(argc<3)
    {
        fprintf(stderr,"usage %s hostname port \n",argv[0]);
    }
    portno=atoi(argv[2]);
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        error("ERROR opening socket");
    }
    server=gethostbyname(argv[1]);
    if(server==NULL)
    {
        fprintf(stderr,"ERROR ,no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    bcopy((char *) server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);

    serv_addr.sin_port=htons(portno);
    if(connect(sockfd,&serv_addr,sizeof(serv_addr))<0)
    {
        error("Error connecting");
    }
    while()

    {

        printf("Please enter the message:");
        bzero(buffer, 256);

        fgets(buffer, 255, stdin);
        n = writev(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
        }
    }
    return 0;

}
