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
int build_server_address(struct sockaddr_in* serv_addr, char * ip_add, int port_num);
int connect_to_server(struct sockaddr_in server_address);
void config_file_to_strings(FILE * file_pointer, char * string, char * strings[]);
void server_to_ip(char * string, char * ip_port_array[]);

int main(int argc, char **argv) 
{
    int port,server1,server2,server3,server4,  check_addr, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in server1_address, server2_address, server3_address,server4_address;
    char conf_string[MAXBUF];
    char * config_array[5];
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
    strtok(config_array[4], " ");
    username = strtok(NULL, " ");
    strtok(config_array[5], " ");
    password = strtok(NULL, " ");

    //connect to all serves
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
        scanf("%d", &user_selection);
        if(user_selection == 99){
            printf("Goodbye!\n");
            return 1;
        }//exit
        //handle list
        else if(user_selection == 1){

        }
        //handle get
        else if(user_selection == 2){

        }
        //handle put
        else if(user_selection == 3){

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

int build_server_address(struct sockaddr_in* serv_addr, char * ip_add, int port_num){
    printf("Building address\n");
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
    if((server_num = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    if(connect(server_num, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0) 
    { 
        printf("Connection Failed\n"); 
        return -1; 
    }

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