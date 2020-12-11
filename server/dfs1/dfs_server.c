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
#include <dirent.h> //ls
#include <unistd.h>
#include <sys/stat.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

int open_listenfd(int port);
void * handle_connection(void * vargp);
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
char * file_to_buf(char * filename);
void config_string_to_strings(char * string, char * strings[]);
void check_dir(char * user);
int ls(char *ls_contents, char * user);

struct Thread_object{
    pthread_mutex_t* file_lock;
    int* client_socket;
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
        thread_object.client_socket = malloc(sizeof(int));
        thread_object.client_socket = (intptr_t*)accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, handle_connection, (void *)&thread_object);
        printf("Connection received\n");
    }//while(1)
}//main

/* thread subroutine */
void * handle_connection(void * vargp) 
{  
    //pointer to struct
    struct Thread_object *thread_object;
    thread_object = (struct Thread_object*)vargp;
    int client_socket = (int)thread_object->client_socket;

    //adding 1 sec timeout
    struct timeval sock_timeout;
    sock_timeout.tv_sec = 5;
    sock_timeout.tv_usec = 0;
   /* setsockopt (client_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));
    setsockopt (client_socket, SOL_SOCKET, SO_SNDTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));*/

    char * entire_request = malloc(MAXBUF);
    char * username = malloc(MAXBUF);
    char * password = malloc(MAXBUF);
    char * filename = malloc(MAXBUF);
    char * request_type = malloc(MAXBUF);
    char * ls_string = malloc(MAXBUF);
    char * file_in_buf = malloc(MAXBUF);
    char * conf_file_string = malloc(MAXBUF);
    int msgsize = 0;
    pthread_detach(pthread_self());     
    

    int bytes_read;
    char * response = "server 1 got it";

    
    /*while((bytes_read = read(client_socket, entire_request +msgsize, sizeof(entire_request)-msgsize-1)) > 0 ){
        msgsize += bytes_read;

    }*/
    printf("Waiting for client request...\n");
    bytes_read = read(client_socket, entire_request, MAXBUF);

    //parse client request
    //comes in form <username> <password> <request_type> <filename>
    char * request_array[4];
    int i;
    i=1;
    char * saveptr = malloc(MAXBUF);
    char * token = malloc(50);
    token = strtok_r(entire_request, " ", &saveptr);
    request_array[0] = token;
    while((token = strtok_r(NULL, " ", &saveptr)) != NULL){
        request_array[i] = token;
        i++;
    }

    //authenticate user here
    conf_file_string = file_to_buf("dfs.conf");
    if(strcmp(conf_file_string, "no_file") ==0){
        printf("Error opening dfs.conf\n");
        return NULL;
    }
    username = request_array[0];
    password = request_array[1];
    
    char * user_password_array[100];  

    config_string_to_strings(conf_file_string,user_password_array);

    char * saveptr2 = malloc(MAXBUF);
    char *compare_user = malloc(MAXBUF);
    char * compare_password = malloc(MAXBUF);
    //printf("SEF\n");
    //compare username and password to users and paswords in conf file
    for(int i =0; i< sizeof(user_password_array)/sizeof(user_password_array[0]); i++){
        if(user_password_array[i] == NULL){
            //can exit here
            printf("User not found\n");
            char * response = malloc(MAXBUF);
            response = "bad user";
            write(client_socket, response, strlen(response));
            close(client_socket);
            return NULL;
        }
        //parse <username> <password>
        compare_user = strtok_r(user_password_array[i], " ", &saveptr2);
        compare_password = strtok_r(NULL, " ", &saveptr2);

        if(strcmp(compare_user, username) == 0 && strcmp(compare_password, password) ==0 ){
            printf("User authenticated\n");
            break;
        }        
    }//for

    //make home directory for user
    check_dir(username);
    
    //handle different request type
    request_type = request_array[2];
    filename = request_array[3];

    //get
    if(strcmp("get", request_type)==0){
        printf("In get\n");
        FILE * client_file;
        char * hom_dir_file = malloc(MAXBUF);
        strcpy(hom_dir_file, username);
        strcat(hom_dir_file,"/");
        strcat(hom_dir_file, filename);
        strcat(hom_dir_file, ".1");
        file_in_buf = file_to_buf(hom_dir_file);
        write(client_socket, file_in_buf, strlen(file_in_buf));
        close(client_socket);

    }//get
    else if(strcmp("put", request_type)==0){

        printf("In put\n");
        char * put_response = malloc(MAXBUF);
        char * file_chunk = malloc(MAXBUF);
        put_response = "Ready for files";
        write(client_socket,put_response, strlen(put_response));

        bytes_read = read(client_socket, file_chunk, MAXBUF);
        put_response = "File received successfully";
        
        write(client_socket,put_response,strlen(put_response));
        close(client_socket);

        //save file to home dir
        FILE * file_to_save;
        char * filename_with_num = malloc(MAXBUF);
        //create file string
        strcpy(filename_with_num, username);
        strcat(filename_with_num,"/");
        strcat(filename_with_num, filename);
        strcat(filename_with_num,".1");

        file_to_save = fopen(filename_with_num, "a");
        if(file_to_save == NULL){
            printf("Error writing to file\n");
            return NULL;
        }
        file_chunk[strlen(file_chunk)] = 0;
        fprintf(file_to_save, "%s", file_chunk);
        fclose(file_to_save);

        return NULL;

        

    }//put
    else if(strcmp("list", request_type)==0){

        ls(ls_string, username);
        write(client_socket, ls_string, strlen(ls_string));
        close(client_socket);

    }//list

    printf("Terminating thread\n");
    return NULL;
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

int ls(char *ls_contents, char * user)
{
    DIR *d;
    struct dirent *dir;

    d = opendir(user);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            strcat(ls_contents, dir->d_name);
            strcat(ls_contents, "\n");
        }
        closedir(d);
    }
    return(0);
}
//https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
char * file_to_buf(char * filename)
{
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        printf("File not found\n");
        return "no_file\0";
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

void config_string_to_strings(char * string, char * strings[])
{
    int i;
    i=1;
    char * token = malloc(100);
    token = strtok(string, "\n");
    strings[0] = token;
    while((token = strtok(NULL, "\n")) != NULL){
        strings[i] = token;
        i++;
    }
}

void check_dir(char * user){
    
    struct stat st = {0};
    if (stat(user, &st) == -1) {
        mkdir(user, 0755);
    }
}