#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>

#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <memory.h>

//Sends error with the message msg
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int lineNumber=0;
//appends to the log file a message


void appendToFile(char s[],FILE *f)
{
    char *ss=s;
   // fwrite(s,sizeof(s),256,f);
    while(ss) {
        lineNumber++;
        char *nextline=strchr(ss,'\n');
        if(nextline)
            *nextline='\0';
        fprintf(f, "%d %s\n", lineNumber, ss);
        fflush(f);

        if (nextline)
            *nextline='\n';
        ss=nextline?(nextline+1):NULL;

        // printf(s);
    }
}

//Creates the log file
FILE  *makeLog()
{
    FILE *f;
    f=fopen("messages.log","a");

    if(f==NULL)
    {
        printf("File cannot be created\n");
    }




    //appendToFile(f,"File created succesfully\n");

    printf("File created\n");
    return f;
}


char *printableMessage(char * string)
{
    int i=0;//number of printable characters
    char *newstring;
    int j;
    for(j=0;j<strlen(string);j++)
    {
          if(string[j]>31 || string[j]=='\n') {
            i++;

           newstring=realloc(newstring,sizeof(char)*i);

            newstring[i-1]=string[j];
        }

    }
    newstring[i]='\0';
    //printf("THIS IS THE PRINTABLE MESSAGE:%s\n",newstring);
    return newstring;
}


/*
int howManyDigits(int a)
{   int i=0;
    while(a)
    {
        a/=10;
        i++;
    }
    return i;
}
/*
char *prepend(int a,char * s)
{
    char number[howManyDigits(a)];
    itoa(a,number,howManyDigits(a));
    return strcat(number,s);
}
*/



pthread_mutex_t mut;
void *processRequest(void *args)
{

    FILE *f=makeLog();
    int *newsockfd=(int *) args;
    char buffer[256];
    int n;

    bzero(buffer,256);
    n=read(*newsockfd,buffer,256);
    if(n<0) {

        error("ERROR reading from socket");
    }
   // printf("Here is the message:%s\n",buffer);

    pthread_mutex_lock(&mut);

    n=write(*newsockfd,printableMessage(buffer),256);


    appendToFile(printableMessage(buffer),f);
       // lineNumber++;
        pthread_mutex_unlock(&mut);


    if(n<0)
    {
        error("Error writing to socket");
    }
    close (*newsockfd);
    free (newsockfd);
    pthread_exit(NULL);

    }


void reallocateThread(pthread_t threads[],int numberOfThreads){
    realloc(threads,numberOfThreads*sizeof(pthread_t));
}


void exitAllThreads(pthread_t threads[],int numberOfThreads)
{
    int i;
    for(i=0;i<numberOfThreads;i++)
    {

        pthread_cancel(threads[i]);
        free(threads[i]);
    }
}



void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("received SIGINT\n");
        _exit(0);
    }

    else if (signo == SIGTERM)
    {
        printf("received SIGTERM\n");
        _exit(0);

    }
}
void signals()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGINT\n");
        _exit(0);
    }

    if (signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGTERM\n");
        _exit(0);
    }
}

void closeAndFreeSocket(int *socket)
{
    close (*socket);
    free (socket);
}
int main(int argc,char *argv[]) {

    socklen_t clilen;
    int sockfd,portno;
    char buffer[256];
    struct sockaddr_in serv_addr,cli_addr;
    int n;


    //remove previous log file in order to reset the log to a new one
    int previousFileDeleted;
    previousFileDeleted=remove("messages.log");
    if(previousFileDeleted==0)
    {
        printf("File deleted successfully\n");
    } else
    {
        printf("Error:unable to delete previous log");
    }

    //making log file

    //checking port number
    if (argc<2)
    {
        fprintf(stderr,"ERROR,no port provided\n");
        exit(1);
    }
    portno=atoi(argv[1]);
    if(portno<1000 || portno >65535)
    {
        error("ERROR invalid portnumber");
    }
    //creating socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        closeAndFreeSocket(sockfd);
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(portno);


    //binding the socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
       // closeAndFreeSocket(sockfd);
        error("ERROR on binding");
    }
    //ready to accept connections
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    int numberOfThreads=0;
    pthread_t threads[1];


    signals();

    //endless loop in order to wait for connections and process them
    while(1) {

        pthread_t server_thread;

        int *newsockfd; //allocate memory for each instance to avoid race condition


        pthread_attr_t  pthread_attr;
        newsockfd=malloc(sizeof(int));
        if(!newsockfd)
        {
            closeAndFreeSocket(newsockfd);
            fprintf(stderr,"Memory allocation failed");
            exit(1);
        }

        *newsockfd=accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

        if (*newsockfd < 0) {
            closeAndFreeSocket(sockfd);
            closeAndFreeSocket(newsockfd);

            error("ERROR on accept");

        }

        bzero(buffer, 256);
        if (pthread_attr_init (&pthread_attr)) {
            fprintf (stderr, "Creating initial thread attributes failed!\n");
            exit (1);
        }
            n=pthread_create(&server_thread,&pthread_attr,processRequest,(void *) newsockfd);
            //  reallocateThread(threads,++numberOfThreads);
            //threads[numberOfThreads-1]=&server_thread;

            if(n!=0)
            {
                fprintf(stderr,"Thread creation failed!\n");
                exit(1);
            }



    }

    return 0;
}