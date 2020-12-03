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
int build_server_address(struct sockaddr_in* serv_addr, char * ip_add);



int main(int argc, char **argv) 
{
    int port,sock,check_addr, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in server_address;
    pthread_t tid; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <dfc.conf>\n", argv[0]);
	exit(0);
    }

    check_addr = build_server_address(&server_address, "127.0.0.1");

    if(check_addr < 0){
        printf("Error building address\n");
        printf("Exiting\n");
        return -1;
    }
    else{

            if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
            { 
                printf("\n Socket creation error \n"); 
                return -1; 
            } 

            if(connect(sock, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) 
            { 
                printf("\nConnection Failed\n"); 
                printf("Exiting\n");
                return -1; 
            }

    }//else
}//main

/* thread subroutine */
void * thread(void * vargp) 
{  

  }//thread  

int build_server_address(struct sockaddr_in* serv_addr, char * ip_add){
    printf("Building address\n");
    serv_addr->sin_family = AF_INET; //ipV4
    serv_addr->sin_port = htons(8000);

    //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    //converts IP to correct format
    if(inet_aton(ip_add, (struct in_addr* )&serv_addr->sin_addr.s_addr) ==0){
        return -1;
    }
    printf("Address built successfully\n");
    return 1;
}