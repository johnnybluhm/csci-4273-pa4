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
#include <sys/mman.h>
#include <fcntl.h>
#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

void *thread(void *vargp);
char* itoa(int value, char* result, int base);
int build_server_address(struct sockaddr_in* serv_addr, char * ip_add, int port_num);
int connect_to_server(struct sockaddr_in server_address);
void config_file_to_strings(FILE * file_pointer, char * string, char * strings[]);
void server_to_ip(char * string, char * ip_port_array[]);
char* get_user(char * conf_string);
char* get_password(char * conf_string);
char* concat(int count, ...);
char * file_to_buf(char * filename);
char * build_combo_chunk(char * file_chunk1, char * file_chunk1_num, char * file_chunk2, char * file_chunk2_num);
void parse_combo_chunk(char * combo_chunk, char * strings[]);

int main(int argc, char **argv) 
{
    int port,server1,server2,server3,server4,  check_addr, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in server1_address, server2_address, server3_address,server4_address;
    char * conf_string[MAXBUF];
    char * config_array[6];
    char * ip_port_array[2];
    if (argc != 2) {
	fprintf(stderr, "usage: %s <dfc.conf>\n", argv[0]);
	exit(0);
    }
    char * strings[6];
    char * conf_file_in_buf;
    conf_file_in_buf = file_to_buf("dfc.conf");
    int i;
    i=1;
    char * token;
    token = strtok(conf_file_in_buf, "\n");
    strings[0] = token;
    while((token = strtok(NULL, "\n")) != NULL){
        strings[i] = token;
        i++;
    }
    int conf_file_size = strlen(conf_file_in_buf);

    FILE* file_pointer;
    config_file_to_strings(file_pointer,conf_string, config_array);
    //can build all different server addresses here
    
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
    char* username = malloc(MAXBUF);
    char* password = malloc(MAXBUF);

    printf("Enter username\n");
    scanf("%[^\n]%*c",username);
    printf("Enter password\n");
    scanf("%[^\n]%*c",password);

    //list commands
    while(1){

        char * user_input = malloc(MAXBUF);
        printf("Enter get <filename> OR put <filename> OR list OR exit\n");
        scanf("%[^\n]%*c",user_input);
        char * command = malloc(MAXBUF);
        char * filename = malloc(MAXBUF);        
        command = strtok(user_input, " ");
        command[strlen(command)] = 0;
        filename = strtok(NULL, " ");

        //exit conditional
        if(strcmp(command, "exit") == 0){
            printf("Goodbye!\n");
            return 1;
        }//exit

        //handle list
        else if(strcmp(command,"list") ==0){

            //connect to all servers
            server1 = connect_to_server(server1_address);
            server2 = connect_to_server(server2_address);
            server3 = connect_to_server(server3_address);
            server4 = connect_to_server(server4_address);

            char * initial_request = malloc(100);
            char * initial_request_copy1 = malloc(100);
            char * initial_request_copy2 = malloc(100);
            char * initial_request_copy3 = malloc(100);
            char * initial_request_copy4 = malloc(100);
            char ls1[MAXBUF];
            char ls2[MAXBUF];
            char ls3[MAXBUF];
            char ls4[MAXBUF];

            //will need a copy to send to each server. string seems to be destroyed upon sending
            initial_request = concat(5, username," ", password , " list ", "list_cmd" );
            printf("%s\n",initial_request );

            strcpy(initial_request_copy1, initial_request);
            strcpy(initial_request_copy2, initial_request);
            strcpy(initial_request_copy3, initial_request);
            strcpy(initial_request_copy4, initial_request);


            write(server1, initial_request_copy1, strlen(initial_request_copy1));
            write(server2, initial_request_copy2, strlen(initial_request_copy2));
            write(server3, initial_request_copy3, strlen(initial_request_copy3));
            write(server4, initial_request_copy4, strlen(initial_request_copy4));

            read(server1, ls1, MAXBUF);
            read(server2, ls2, MAXBUF);
            read(server3, ls3, MAXBUF);
            read(server4, ls4, MAXBUF);

            if(strcmp(ls1, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server1);
                return -1;
            }
            if(strcmp(ls2, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server2);
                return -1;
            }
            if(strcmp(ls3, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server3);
                return -1;
            }
            if(strcmp(ls4, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server4);
                return -1;
            }

            printf("Server 1 directory contents:\n%s\n\n", ls1);
            printf("Server 2 directory contents:\n%s\n\n", ls2);
            printf("Server 3 directory contents:\n%s\n\n", ls3);
            printf("Server 4 directory contents:\n%s\n\n", ls4);

            close(server1);
            close(server2);
            close(server3);
            close(server4);

            printf("Listed successfully\n\n");


        }//list

        //handle get
        else if(strcmp(command,"get")== 0){

            //connect to all servers
            server1 = connect_to_server(server1_address);
            server2 = connect_to_server(server2_address);
            server3 = connect_to_server(server3_address);
            server4 = connect_to_server(server4_address);
            
            char * initial_request = malloc(100);
            char * initial_request_copy1 = malloc(100);
            char * initial_request_copy2 = malloc(100);
            char * initial_request_copy3 = malloc(100);
            char * initial_request_copy4 = malloc(100);
            char * file_chunk1 = malloc(MAXBUF);
            char * file_chunk2 = malloc(MAXBUF);
            char * file_chunk3 = malloc(MAXBUF);
            char * file_chunk4 = malloc(MAXBUF);
            
            //build request to server
            //format of "<username> <password> get <filename>""
            initial_request = concat(5, username," ", password , " get ", filename);
            strcpy(initial_request_copy1, initial_request);
            strcpy(initial_request_copy2, initial_request);
            strcpy(initial_request_copy3, initial_request);
            strcpy(initial_request_copy4, initial_request);

            //write request to all servers
            write(server1, initial_request_copy1, strlen(initial_request_copy1));
            write(server2, initial_request_copy2, strlen(initial_request_copy2));
            write(server3, initial_request_copy3, strlen(initial_request_copy3));
            write(server4, initial_request_copy4, strlen(initial_request_copy4));


            printf("Waiting for server reply...\n");
            //read response from all servers
            read(server1, file_chunk1, MAXBUF);
            read(server2, file_chunk2, MAXBUF);
            read(server3, file_chunk3, MAXBUF);
            read(server4, file_chunk4, MAXBUF);

            if(strcmp(file_chunk1, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server1);
                return -1;
            }
            if(strcmp(file_chunk2, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server2);
                return -1;
            }
            if(strcmp(file_chunk3, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server3);
                return -1;
            }
            if(strcmp(file_chunk4, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server4);
                return -1;
            }

            close(server1);
            close(server2);
            close(server3);
            close(server4);

            char * parse_chunk1[4];
            char * parse_chunk2[4];
            char * parse_chunk3[4];
            char * parse_chunk4[4];

            parse_combo_chunk(file_chunk1, parse_chunk1);
            parse_combo_chunk(file_chunk2, parse_chunk2);
            parse_combo_chunk(file_chunk3, parse_chunk3);
            parse_combo_chunk(file_chunk4, parse_chunk4);

            char * client_file_hash = malloc(MAXBUF);
            int hashed_file;
            strcpy(client_file_hash, filename);
            hashed_file = hash(client_file_hash);
            hashed_file = hashed_file % 4;
            if(hashed_file<0){
                hashed_file = hashed_file*-1;
            }

            char * assembled_file = malloc(MAXBUF);
            //concat file chunks, then put to a file
            switch(hashed_file){

                case 0:
                    strcpy(assembled_file, parse_chunk1[0]);
                    strcat(assembled_file, parse_chunk1[2]);
                    strcat(assembled_file, parse_chunk2[2]);
                    strcat(assembled_file, parse_chunk3[2]);
                    break;               

                case 1: 
                    strcpy(assembled_file, parse_chunk1[2]);
                    strcat(assembled_file, parse_chunk2[2]);
                    strcat(assembled_file, parse_chunk3[2]);
                    strcat(assembled_file, parse_chunk4[2]);
                    break;                

                case 2:
                    strcpy(assembled_file, parse_chunk2[2]);
                    strcat(assembled_file, parse_chunk3[2]);
                    strcat(assembled_file, parse_chunk4[2]);
                    strcat(assembled_file, parse_chunk1[2]);
                    break;                

                case 3:
                    strcpy(assembled_file, parse_chunk4[0]);
                    strcat(assembled_file, parse_chunk1[0]);
                    strcat(assembled_file, parse_chunk1[2]);
                    strcat(assembled_file, parse_chunk2[2]);
                    break;

                default: 
                    printf("Bad hash\n");
                    return -1;
            }
            
            //put file strings into a file
            //need to store as indivbudal chunks

            printf("Assembled file:\n%s\n",assembled_file);
            FILE * file_in_client;
            file_in_client = fopen(filename, "w");
            fputs(assembled_file, file_in_client);

            printf("Client got successfully\n\n");           

        }//get

        //handle put
        else if(strcmp(command,"put") ==0){

            //connect to all servers
            server1 = connect_to_server(server1_address);
            server2 = connect_to_server(server2_address);
            server3 = connect_to_server(server3_address);
            server4 = connect_to_server(server4_address);
            
            //char filename[MAXBUF];
            char * initial_request = malloc(100);
            char * initial_request_copy1 = malloc(100);
            char * initial_request_copy2 = malloc(100);
            char * initial_request_copy3 = malloc(100);
            char * initial_request_copy4 = malloc(100);
            char * server_res1 = malloc(MAXBUF);
            char * server_res2 = malloc(MAXBUF);
            char * server_res3 = malloc(MAXBUF);
            char * server_res4 = malloc(MAXBUF);

            //build request to server
            //format of "<username> <password> put <filename>"
            initial_request = concat(5, username," ", password , " put ", filename);
            strcpy(initial_request_copy1, initial_request);
            strcpy(initial_request_copy2, initial_request);
            strcpy(initial_request_copy3, initial_request);
            strcpy(initial_request_copy4, initial_request);

            //need to then open file, hash contents and split up, send each chunk to a different server

            //write request to all servers
            write(server1, initial_request_copy1, strlen(initial_request_copy1));
            write(server2, initial_request_copy2, strlen(initial_request_copy2));
            write(server3, initial_request_copy3, strlen(initial_request_copy3));
            write(server4, initial_request_copy4, strlen(initial_request_copy4));        

            printf("Waiting for server reply...\n");
            //read response from all servers
            read(server1, server_res1, MAXBUF);
            read(server2, server_res2, MAXBUF);
            read(server3, server_res3, MAXBUF);
            read(server4, server_res4, MAXBUF);

            if(strcmp(server_res1, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server1);
                return -1;
            }
            if(strcmp(server_res2, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server2);
                return -1;
            }
            if(strcmp(server_res3, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server3);
                return -1;
            }
            if(strcmp(server_res4, "bad user") ==0){
                printf("User could not be authenticated\n");
                close(server4);
                return -1;
            }                
            

            //convert file to string
            //hash file
            char * client_file_string = malloc(MAXBUF);
            char * client_file_hash = malloc(MAXBUF);
            client_file_string = file_to_buf(filename);
            strcpy(client_file_hash, filename);

            int hashed_file;
            hashed_file = hash(client_file_hash);
            hashed_file = hashed_file % 4;
            if(hashed_file<0){
                hashed_file = hashed_file*-1;
            }

            int filesize = strlen(client_file_string);
            int chunk1 = filesize/4;
            int chunk2 = chunk1*2;
            int chunk3 = chunk1*3;
            char file_chunk1[MAXBUF];
            char file_chunk2[MAXBUF];
            char file_chunk3[MAXBUF];
            char file_chunk4[MAXBUF];

            //fill file chunks
            for(int i =0; i < chunk1; i++){
                file_chunk1[i]=client_file_string[i];
            }
            int j =0;
            for(int i =chunk1; i < chunk2; i++){
                file_chunk2[j]=client_file_string[i];
                j++;
            }
            j = 0;
            for(int i =chunk2; i < chunk3; i++){
                file_chunk3[j]=client_file_string[i];
                j++;
            }
            j=0;
            for(int i =chunk3; i < filesize; i++){
                file_chunk4[j]=client_file_string[i];
                j++;
            }

            char * combo_chunk1 = malloc(MAXBUF);
            char * combo_chunk2 = malloc(MAXBUF);
            char * combo_chunk3 = malloc(MAXBUF);
            char * combo_chunk4 = malloc(MAXBUF);

            switch(hashed_file){

                case 0: 
                combo_chunk1 = build_combo_chunk(file_chunk1,"1", file_chunk2, "2");
                combo_chunk2 = build_combo_chunk(file_chunk2,"2", file_chunk3, "3");
                combo_chunk3 = build_combo_chunk(file_chunk3,"3", file_chunk4, "4");
                combo_chunk4 = build_combo_chunk(file_chunk4,"4", file_chunk1, "1");
                break;

                case 1: 
                combo_chunk1 = build_combo_chunk(file_chunk4,"4", file_chunk1, "1");
                combo_chunk2 = build_combo_chunk(file_chunk1,"1", file_chunk2, "2");
                combo_chunk3 = build_combo_chunk(file_chunk2,"2", file_chunk3, "3");
                combo_chunk4 = build_combo_chunk(file_chunk3,"3", file_chunk4, "4");
                break;

                case 2:
                combo_chunk1 = build_combo_chunk(file_chunk3,"3", file_chunk4, "1");
                combo_chunk2 = build_combo_chunk(file_chunk4,"4", file_chunk1, "1");
                combo_chunk3 = build_combo_chunk(file_chunk1,"1", file_chunk2, "2");
                combo_chunk4 = build_combo_chunk(file_chunk2,"2", file_chunk3, "3");
                break;

                case 3:
                combo_chunk1 = build_combo_chunk(file_chunk2,"2", file_chunk3, "3");
                combo_chunk2 = build_combo_chunk(file_chunk3,"3", file_chunk4, "4");
                combo_chunk3 = build_combo_chunk(file_chunk4,"4", file_chunk1, "1");
                combo_chunk4 = build_combo_chunk(file_chunk1,"1", file_chunk2, "2");

                default: printf("Bad hash\n");
                    return -1;
            }
            
            //write back to servers with file chunks
            write(server1, combo_chunk1, strlen(combo_chunk1));
            write(server2, combo_chunk2, strlen(combo_chunk2));
            write(server3, combo_chunk3, strlen(combo_chunk3));
            write(server4, combo_chunk4, strlen(combo_chunk4));

            read(server1,server_res1, MAXBUF);
            read(server2,server_res2, MAXBUF);
            read(server3,server_res3, MAXBUF);
            read(server4,server_res4, MAXBUF);

            close(server1);
            close(server2);
            close(server3);
            close(server4);

            printf("Client put successfully\n\n");        

        }//put
        else{
            printf("Please enter a valid command\n\n");
        }//bad command

    }//while(1)
    
}//main 

void parse_combo_chunk(char * combo_chunk, char * strings[]){

    if(strcmp("no_file", combo_chunk)==0){
    strings[0] = NULL;
    strings[1] = NULL;
    strings[2] = NULL;
    strings[3] = NULL;

    }
    else if(combo_chunk == NULL){
    strings[0] = NULL;
    strings[1] = NULL;
    strings[2] = NULL;
    strings[3] = NULL;
    }
    else{

        char *chunk1 = malloc(MAXBUF);
        char *chunk2 = malloc(MAXBUF);
        char *chunk1_num = malloc(MAXBUF);
        char *chunk2_num = malloc(MAXBUF);
        chunk1 = strtok(combo_chunk, "\n\n\n\n");
        chunk1_num = strtok(NULL, "\n\n\n\n");
        chunk2 = strtok(NULL, "\n\n\n\n");
        chunk2_num = strtok(NULL, "\n\n\n\n");

        strings[0] = chunk1;
        strings[1] = chunk1_num;
        strings[2] = chunk2;
        strings[3] = chunk2_num;
    }//else
}

char * build_combo_chunk(char * file_chunk1, char * file_chunk1_num, char * file_chunk2, char * file_chunk2_num){

    char * combo_chunk = malloc(MAXBUF);
    strcpy(combo_chunk, file_chunk1);
    strcat(combo_chunk, "\n\n\n\n");
    strcat(combo_chunk, file_chunk1_num);
    strcat(combo_chunk, "\n\n\n\n");
    strcat(combo_chunk, file_chunk2);
    strcat(combo_chunk, "\n\n\n\n");
    strcat(combo_chunk, file_chunk2_num);

    return combo_chunk;
}
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

//https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
char * file_to_buf(char * filename)
{
    printf("%s\n",filename );
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        printf("File not found\n");
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
    char * token = malloc(100);
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
//https://stackoverflow.com/questions/4023895/how-do-i-read-a-string-entered-by-the-user-in-c
int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
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

/*void put_func(char * initial_request)
{
    char * initial_request_copy1 = malloc(100);
    char * initial_request_copy2 = malloc(100);
    char * initial_request_copy3 = malloc(100);
    char * initial_request_copy4 = malloc(100);
    char * server_res1 = malloc(MAXBUF);
    char * server_res2 = malloc(MAXBUF);
    char * server_res3 = malloc(MAXBUF);
    char * server_res4 = malloc(MAXBUF);


    strcpy(initial_request_copy1, initial_request);
    strcpy(initial_request_copy2, initial_request);
    strcpy(initial_request_copy3, initial_request);
    strcpy(initial_request_copy4, initial_request);

    //need to then open file, hash contents and split up, send each chunk to a different server

    //write request to all servers
    write(server1, initial_request_copy1, strlen(initial_request_copy1));
    write(server2, initial_request_copy2, strlen(initial_request_copy2));
    write(server3, initial_request_copy3, strlen(initial_request_copy3));
    write(server4, initial_request_copy4, strlen(initial_request_copy4));        

    printf("Waiting for server reply...\n");
    //read response from all servers
    read(server1, server_res1, MAXBUF);
    read(server2, server_res2, MAXBUF);
    read(server3, server_res3, MAXBUF);
    read(server4, server_res4, MAXBUF);

    if(strcmp(server_res1, "bad user") ==0){
        printf("User could not be authenticated\n");
        close(server1);
        return -1;
    }
    if(strcmp(server_res2, "bad user") ==0){
        printf("User could not be authenticated\n");
        close(server2);
        return -1;
    }
    if(strcmp(server_res3, "bad user") ==0){
        printf("User could not be authenticated\n");
        close(server3);
        return -1;
    }
    if(strcmp(server_res4, "bad user") ==0){
        printf("User could not be authenticated\n");
        close(server4);
        return -1;
    }                
    

    //convert file to string
    //hash file
    char * client_file_string = malloc(MAXBUF);
    char * client_file_hash = malloc(MAXBUF);
    client_file_string = file_to_buf(filename);
    strcpy(client_file_hash, client_file_string);
    int hashed_file;
    hashed_file = hash(client_file_hash);
    hashed_file = hashed_file % 4;
    if(hashed_file<0){
        hashed_file = hashed_file*-1;
    }

    int filesize = strlen(client_file_string);
    int chunk1 = filesize/4;
    int chunk2 = chunk1*2;
    int chunk3 = chunk1*3;
    char file_chunk1[MAXBUF];
    char file_chunk2[MAXBUF];
    char file_chunk3[MAXBUF];
    char file_chunk4[MAXBUF];

    //fill file chunks
    for(int i =0; i < chunk1; i++){
        file_chunk1[i]=client_file_string[i];
    }
    int j =0;
    for(int i =chunk1; i < chunk2; i++){
        file_chunk2[j]=client_file_string[i];
        j++;
    }
    j = 0;
    for(int i =chunk2; i < chunk3; i++){
        file_chunk3[j]=client_file_string[i];
        j++;
    }
    j=0;
    for(int i =chunk3; i < filesize; i++){
        file_chunk4[j]=client_file_string[i];
        j++;
    }

    //need to switch hash value
    
    
    //write back to servers with file chunks
    write(server1, file_chunk1, strlen(file_chunk1));
    write(server2, file_chunk2, strlen(file_chunk2));
    write(server3, file_chunk3, strlen(file_chunk3));
    write(server4, file_chunk4, strlen(file_chunk4));

    read(server1,server_res1, MAXBUF);
    read(server2,server_res2, MAXBUF);
    read(server3,server_res3, MAXBUF);
    read(server4,server_res4, MAXBUF);

    close(server1);
    close(server2);
    close(server3);
    close(server4);
}*/