#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

void *thread(void *vargp);
char* itoa(int value, char* result, int base);
int build_server_addr(struct sockaddr_in* serv_addr, char * ip_add);



int main(int argc, char **argv) 
{
    int listenfd, timeout, port, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid; 
    pthread_mutex_t* file_lock;
    file_lock = malloc(sizeof(pthread_mutex_t));
    //initialize file_lock
    if (pthread_mutex_init(file_lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <dfc.conf>\n", argv[0]);
	exit(0);
    }
    time_t start_time;
    start_time = time(NULL);
    port = atoi(argv[1]);
    timeout = atoi(argv[2]);

    listenfd = open_listenfd(port);

    //initialize thread_object
    struct Thread_object thread_object;
    thread_object.file_lock = file_lock;
    thread_object.timeout = &timeout;
    thread_object.start_time = start_time;

    printf("Listening on port %d\n", port);
    while (1) {    
        thread_object.connfdp = (intptr_t*)accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, (void *)&thread_object);
    }//while(1)
}//main

/* thread subroutine */
void * thread(void * vargp) 
{  

  }//thread  
