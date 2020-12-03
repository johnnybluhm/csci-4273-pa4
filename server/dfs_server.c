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

int open_listenfd(int port);
void *thread(void *vargp);
char* itoa(int value, char* result, int base);
int dnslookup(const char* hostname, char* firstIPstr, int maxSize);
void * test(void * vargp);
int hostname_to_ip(char *hostname , char *ip);
int build_server_addr(struct sockaddr_in* serv_addr, char * ip_add);
void send_error(int connfd, char * error_msg);
char* itoa(int value, char* result, int base);
int hash(char *str);
void send_black(int connfd, char * error_msg);
char* ultostr(unsigned long value, char *ptr, int base);

struct Thread_object{
    pthread_mutex_t* file_lock;
    int* connfdp;
};

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

    port = atoi(argv[1]);

    //creates and binds socket
    listenfd = open_listenfd(port);

    //initialize thread_object
    struct Thread_object thread_object;
    thread_object.file_lock = file_lock;
    printf("Listening on port %d\n", port);
    while (1) {    
        thread_object.connfdp = (intptr_t*)accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, (void *)&thread_object);
    }//while(1)
}//main

/* thread subroutine */
void * thread(void * vargp) 
{  
    //pointer to struct
    struct Thread_object *thread_object;
    thread_object = (struct Thread_object*)vargp;
    int connfd = (int)thread_object->connfdp;

    char * message = malloc(MAXBUF);
    
    pthread_detach(pthread_self());     
    printf("Connection received\n");
    read(connfd, message, MAXBUF);

    printf("read %s from client \n",message);

  }//thread  

/*


 

HELPER FUNCTIONS BELOW  





*/
int build_server_addr(struct sockaddr_in* serv_addr, char * ip_add){
    printf("Building address\n");
    serv_addr->sin_family = AF_INET; //ipV4
    serv_addr->sin_port = htons(80);

    //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    //converts IP to correct format
    if(inet_aton(ip_add, (struct in_addr* )&serv_addr->sin_addr.s_addr) ==0){
        return -1;
    }
    printf("Address built successfully\n");
    return 1;
}

int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */

void send_error(int connfd, char * error_msg)
{

    char httpmsg[]="HTTP/1.1 400 Bad Request\r\nContent-Type:text/plain\r\nContent-Length:";
    char after_content_length[]="\r\n\r\n";
    char content_length[MAXLINE];
    
    //convert file size int to string
    itoa(strlen(error_msg), content_length, 10);
    
    strcat(httpmsg, content_length);
    strcat(httpmsg, after_content_length);    

    //add file contents to http header
    strcat(httpmsg, error_msg);
    //printf("server returning a http message with the following content.\n%s\n",httpmsg);
    write(connfd, httpmsg, strlen(httpmsg));
    
}

char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

//https://stackoverflow.com/questions/7666509/hash-function-for-string
int hash(char *str)
{
    int hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

char* ultostr(unsigned long value, char *ptr, int base)
{
  unsigned long t = 0, res = 0;
  unsigned long tmp = value;
  int count = 0;

  if (NULL == ptr)
  {
    return NULL;
  }

  if (tmp == 0)
  {
    count++;
  }

  while(tmp > 0)
  {
    tmp = tmp/base;
    count++;
  }

  ptr += count;

  *ptr = '\0';

  do
  {
    res = value - base * (t = value / base);
    if (res < 10)
    {
      * -- ptr = '0' + res;
    }
    else if ((res >= 10) && (res < 16))
    {
        * --ptr = 'A' - 10 + res;
    }
  } while ((value = t) != 0);

  return(ptr);
}

void send_black(int connfd, char * error_msg)
{


    char httpmsg[]="HTTP/1.1 403 Forbidden\r\nContent-Type:text/plain\r\nContent-Length:";
    char after_content_length[]="\r\n\r\n";
    char content_length[MAXLINE];
    
    //convert file size int to string
    itoa(strlen(error_msg), content_length, 10);
    
    strcat(httpmsg, content_length);
    strcat(httpmsg, after_content_length);    

    //add file contents to http header
    strcat(httpmsg, error_msg);
    //printf("server returning a http message with the following content.\n%s\n",httpmsg);
    write(connfd, httpmsg, strlen(httpmsg));
    
}