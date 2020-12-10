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


#include <stdarg.h>
#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

void *thread(void *vargp);
char* itoa(int value, char* result, int base);
int build_server_address(struct sockaddr_in* serv_addr, char * ip_add, int port_num);
int connect_to_server(struct sockaddr_in server_address);
void config_file_to_strings(FILE * file_pointer, char * string, char * strings[]);
void server_to_ip(char * string, char * ip_port_array[]);
char* get_user(char * conf_string);
char* get_password(char * conf_string);
char* concat(int count, ...);

int main(int argc, char **argv) 
{
    int port,server1,server2,server3,server4,  check_addr, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in server1_address, server2_address, server3_address,server4_address;
    char conf_string[MAXBUF];
    char * config_array[6];
    char * ip_port_array[2];
    if (argc != 2) {
	fprintf(stderr, "usage: %s <dfc.conf>\n", argv[0]);
	exit(0);
    }

    FILE* file_pointer;
    config_file_to_strings(file_pointer,conf_string, config_array);

    //can build all different server addresses here
      
    //build server 1
    server_to_ip(config_array[0], ip_port_array);
    port = atoi(ip_port_array[1]);
    check_addr = build_server_address(&server1_address, ip_port_array[0], port);
    if(check_addr < 0){
        printf("Error building address 1\n");
        return -1;
    }

    //build server 2
    server_to_ip(config_array[1], ip_port_array);
    port = atoi(ip_port_array[1]);
    check_addr = build_server_address(&server2_address, ip_port_array[0], port);
    if(check_addr < 0){
        printf("Error building address 2\n");
        return -1;
    }

    //build server 3
    server_to_ip(config_array[2], ip_port_array);
    port = atoi(ip_port_array[1]);
    check_addr = build_server_address(&server3_address, ip_port_array[0], port);
    if(check_addr < 0){
        printf("Error building address 3\n");
        return -1;
    }

    //build server 4
    server_to_ip(config_array[3], ip_port_array);
    port = atoi(ip_port_array[1]);
    check_addr = build_server_address(&server4_address, ip_port_array[0], port);
    if(check_addr < 0){
        printf("Error building address 4\n");
        return -1;
    }    

    //get username and password
    char* username;
    char* password;
    /*username = strtok(config_array[4], " ");
    username = strtok(NULL, " ");
    //username = get_user(config_array[4]);
    //password = get_user(config_array[5]);
    password = strtok(config_array[5], " ");
    password = strtok(NULL, " ");
    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';*/
    username = config_array[4];
    printf("username is:\n%s\n",username );
    //connect to all servers
    server1 = connect_to_server(server1_address);
    server2 = connect_to_server(server2_address);
    server3 = connect_to_server(server3_address);
    server4 = connect_to_server(server4_address);

    //connected from this point
    
    //list commands
    while(1){

        printf("Select a command to send to server\n");
        printf("1:list\n");
        printf("2:get\n");
        printf("3:put\n");
        printf("99: exit\n");
        int user_selection;
        char a;
        scanf("%d", &user_selection);
        //a = getchar();
        //exit conditional
        if(user_selection == 99){
            printf("Goodbye!\n");
            return 1;
        }//exit

        //handle list
        else if(user_selection == 1){

            printf("Server's response:\n\n");

        }

        //handle get
        else if(user_selection == 2){

            char filename[MAXBUF];
            char * initial_request = malloc(100);
            char buf[MAXBUF];
            printf("Enter filename\n");
            scanf("%s",filename);
            printf("file name is:\n%s\n",filename );
            printf("%s\n",initial_request );
            //format of "<username> <password> get <filename>""
            initial_request = concat(4, "alice", " password", " get ",filename );
            //build request to server
            //strcat(initial_request, " ");
            //strcat(initial_request, "get");
            //strcat(initial_request, " ");
            //strcat(initial_request, file_name);
            printf("Request is:\n%s\n",initial_request );
            write(server1, initial_request, strlen(initial_request));
            
            //write(server1, password, strlen(password));
            //write(server1, "get", strlen("get"));
            //write(server1, file_name, strlen(file_name));

            printf("Requesting from server\n");

            read(server1, buf, MAXBUF);

            printf("Server response %s\n", buf);

        }

        //handle put
        else if(user_selection == 3){

            char  filename[MAXBUF];
            printf("Enter filename\n");
            scanf("%s",filename);
            printf("Requesting from server\n");

        }
        else{
            printf("Please enter a valid command\n\n");
        }

        /*
        char * message = "server 1 hello";
        char * message2 = "server 2 hello";
        printf("you selected %d\n",user_selection );

        printf("sending to server\n");

        write(server1, message,strlen(message));
        write(server2, message2,strlen(message));
        */

    }//while(1)
    
}//main 

char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

char* get_user(char * conf_string){

    strtok(conf_string, " ");
    return strtok(NULL, " ");

}

char* get_password(char * conf_string){

    strtok(conf_string, " ");

    return strtok(NULL, " ");
}

int build_server_address(struct sockaddr_in* serv_addr, char * ip_add, int port_num){

    serv_addr->sin_family = AF_INET; //ipV4
    serv_addr->sin_port = htons(port_num);

    //https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    //converts IP to correct format
    if(inet_aton(ip_add, (struct in_addr* )&serv_addr->sin_addr.s_addr) ==0){
        return -1;
    }
    return 1;
}

int connect_to_server(struct sockaddr_in server_address)
{
    int server_num;

    struct timeval sock_timeout;
    sock_timeout.tv_sec = 1;
    sock_timeout.tv_usec = 0;

    if((server_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    setsockopt (server_num, IPPROTO_TCP, SO_RCVTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));
    setsockopt (server_num, IPPROTO_TCP, SO_SNDTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));

    if(connect(server_num, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) 
    { 
        printf("Connection Failed\n"); 
        return -1; 
    }

    setsockopt (server_num, IPPROTO_TCP, SO_RCVTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));
    setsockopt (server_num, IPPROTO_TCP, SO_SNDTIMEO, (struct timeval*)&sock_timeout, sizeof (sock_timeout));

    return server_num;

}

void config_file_to_strings(FILE * file_pointer, char * string, char * strings[])
{
    file_pointer = fopen("dfc.conf", "r");
    if(file_pointer == NULL){
        printf("File not found!\n");
        return -1;
    }
    char c;
    int i;
    i =0;
    c = fgetc(file_pointer);
    while(c != EOF){
        string[i]=c;
        c = fgetc(file_pointer);
        i++;
    }
    i=1;
    char * token;
    token = strtok(string, "\n");
    strings[0] = token;
    while((token = strtok(NULL, "\n")) != NULL){
        strings[i] = token;
        i++;
    }

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

void server_to_ip(char * string, char * ip_port_array[])
{
    char * token;
    token= strtok(string, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");

    ip_port_array[0] = strtok(token, ":");
    ip_port_array[1] = strtok(NULL, ":");

}