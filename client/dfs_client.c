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


int main(int argc, char **argv) 
{
    int port,server1,server2,server3,server4,  check_addr, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in server1_address, server2_address, server3_address,server4_address;
    char server1s[MAXBUF];
    char server2s[MAXBUF];
    char server3s[MAXBUF];
    char server4s[MAXBUF];

    if (argc != 2) {
	fprintf(stderr, "usage: %s <dfc.conf>\n", argv[0]);
	exit(0);
    }

    FILE* file_pointer;

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
        server1s[i]=c;
        c = fgetc(file_pointer);
        i++;
    }

    printf("server 1 is %s\n",server1s );
    //can build all different server addresses here
    if(check_addr < 0){
        printf("Error building address 1\n");
        return -1;
    }

    check_addr = build_server_address(&server2_address, "127.0.0.1", 8001);

    if(check_addr < 0){
        printf("Error building address 1\n");
        return -1;
    }
    else{

        server1 = connect_to_server(server1_address);
        server2 = connect_to_server(server2_address);
        //connected from this point
        
        //list commands
        printf("Select a command to send to server\n");
        printf("1:list\n");
        printf("2:get\n");
        printf("3:put\n");
        int user_selection;
        scanf("%d", &user_selection);
        char * message = "server 1 hello";
        char * message2 = "server 2 hello";
        printf("you selected %d\n",user_selection );

        printf("sending to server\n");

        write(server1, message,strlen(message));
        write(server2, message2,strlen(message));

    }//else
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
    printf("Address built successfully\n");
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
        printf("\nConnection Failed\n"); 
        return -1; 
    }

    return server_num;

}