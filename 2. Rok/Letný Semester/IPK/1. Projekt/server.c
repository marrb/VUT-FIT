#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>

//Structure for makeshift string
typedef struct{
    char *str;
    int string_lenght;
    int string_alloc_lenght;
}string;

#define DEFAULT_STRING_LEN 20
#define ERROR (-1)

void init_string(string *str){
    str->str = (char *)malloc(DEFAULT_STRING_LEN);
    if(!str->str) exit(-1);
    str->str[0] = '\0';
    str->string_lenght = 0;
    str->string_alloc_lenght = DEFAULT_STRING_LEN;
}

void free_string(string *str){
    free(str->str);
}

void string_clear(string *str){
    str->string_lenght = 0;
    str->str[0] = '\0';
}

//Gets the next number from string of numbers--------
int get_next_number(char *str, int *index){
    char num_buffer[50] = {0};
    int counter = 0;
    int i = *index;

    for(; (str[i] != ' ' && str[i] != '\0'); i++){
        num_buffer[counter] = str[i];
        counter++;
    }

    *index = ++i;
    return atoi(num_buffer);
}
//--------------------------------------------------   

//Gets the % load of CPU-------------------------------------------------------------
int get_load(){
    char prev_stats[50] = {0};
    char current_stats[50] = {0};

    FILE *cpu_info = popen("cat /proc/stat | grep -w 'cpu' | cut -c 6- | tr -d '\n'", "r"); //Gets the required values from /proc/stat
    fgets(prev_stats, 50, cpu_info);
    pclose(cpu_info);

    sleep(1); //Wait for 1 second

    cpu_info = popen("cat /proc/stat | grep -w 'cpu' | cut -c 6- | tr -d '\n'", "r"); //Gets the required values from /proc/stat
    fgets(current_stats, 50, cpu_info);
    pclose(cpu_info);

    int index = 0;
    int user = get_next_number(current_stats, &index);
    int nice = get_next_number(current_stats, &index);
    int system = get_next_number(current_stats, &index);
    int idle = get_next_number(current_stats, &index);
    int iowait = get_next_number(current_stats, &index);
    int irq = get_next_number(current_stats, &index);
    int softirq = get_next_number(current_stats, &index);
    int steal = get_next_number(current_stats, &index);


    index = 0;
    int prev_user = get_next_number(prev_stats, &index);
    int prev_nice = get_next_number(prev_stats, &index);
    int prev_system = get_next_number(prev_stats, &index);
    int prevv_idle = get_next_number(prev_stats, &index);
    int prev_iowait = get_next_number(prev_stats, &index);
    int prev_irq = get_next_number(prev_stats, &index);
    int prev_softirq = get_next_number(prev_stats, &index);
    int prev_steal = get_next_number(prev_stats, &index);


    //Calculate load
    int Prev_Idle = prevv_idle + prev_iowait;
    int Idle = idle + iowait;
    
    int Prev_non_Idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    int non_Idle = user + nice + system + irq + softirq + steal;

    int Prev_Total = Prev_Idle + Prev_non_Idle;
    int Total = Idle + non_Idle;

    int total_work = Total - Prev_Total;
    int total_idle = Idle - Prev_Idle;

    return (((total_work - total_idle)/total_work) * 100);
}
//----------------------------------------------------------------------------------   


//Get instruction from html header-------------------------
void get_instr(string *dest, char *source){
    int new_len = 4;
    while(source[new_len] != ' '){
        new_len++;
    }
    new_len -= 3;

    if(new_len > dest->string_alloc_lenght){
        dest->str = (char *)realloc(dest->str, new_len);
        if(!dest->str) exit(-1);
        dest->string_alloc_lenght = new_len;
    }

    strncpy(dest->str, source+4, new_len-1);
    dest->str[new_len-1] = '\0';
    dest->string_lenght = new_len-1;
}
//---------------------------------------------------------


char *web = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: "; //HTML header for sending data to client

int main(int argc, char *argv[]){
    //Parameter processing
    if(argc == 1){
        fprintf(stderr, "Please specify a port!\nUsage: ./hinfosvc [PORT]\n");
        exit(ERROR);
    }

    for(int i = 0; argv[1][i] != '\0'; i++){
        if(!isdigit(argv[1][i])){
            fprintf(stderr, "Port has to be a whole positive number!\n");
            exit(ERROR);
        }
    }
    
    
    int PORT = atoi(argv[1]);

    if(PORT < 0 || PORT > 65535){
        fprintf(stderr, "Port has to be in range of 0 - 65535!\n");
        exit(ERROR);
    }


    //Create server socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == -1){
        fprintf(stderr, "Cannot create socket!\n");
        exit(ERROR);
    }
    int optval = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));


    //Set-up the TCP socket for communicating with client
    struct sockaddr_in address;
    memset((char *)&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(server, (struct sockaddr *)&address, sizeof(address)) < 0){
        fprintf(stderr, "Bind error, port is reserved!\n");
        exit(ERROR);
    }

    //Listen for incomming connections
    if(listen(server, 128) < 0){
        fprintf(stderr, "Creating listen socket failed!\n");
        exit(ERROR);
    }

    int client_msg;
    int address_len = sizeof(address);
    string instruction;
    char msg[256] = {0};

    while(1){
        printf("\n------------------Waiting for connection------------------\n\n");
        memset(msg, 0, 256);
        strcat(msg, web);

        if((client_msg = accept(server, (struct sockaddr *)&address, (socklen_t *)&address_len)) < 0){
            fprintf(stderr, "Creating accept socket failed!\n");
            exit(ERROR);
        }

        char buffer[1024] = {0};
        read(client_msg, buffer, 1024);

        init_string(&instruction);
        get_instr(&instruction, buffer);

        printf("Wanted Data: %s\n", instruction.str);

        if(!strcmp(instruction.str, "/hostname")){
            char hostname[256];

            FILE *hostname_info = popen("hostname | tr -d '\n'", "r"); //Get hostname
            fgets(hostname, 256, hostname_info);
            pclose(hostname_info);

            char result[100] = {0};
            int len = strlen(hostname)+1;
            sprintf(result, "%d", len);

            //Construct message
            strcat(msg, result);
            strcat(msg, "\n\n");
            strcat(msg, hostname);
            strcat(msg, "\n");
            
            write(client_msg, msg , strlen(msg));
        }
        else if(!strcmp(instruction.str, "/cpu-name")){
            char cpu[256];
            char result[100] = {0};

            FILE *cpu_info = popen("cat /proc/cpuinfo | grep -m 1 'model name' | cut -c 14- | tr -d '\n'", "r");
            if(!cpu_info){
                fprintf(stderr, "Cannot execute!");
                break;
            }

            fgets(cpu, 256, cpu_info);
            pclose(cpu_info);

            int len = strlen(cpu)+1;
            sprintf(result, "%d", len);
            //Construct message
            strcat(msg, result);
            strcat(msg, "\n\n");
            strcat(msg, cpu);
            strcat(msg, "\n");

            write(client_msg, msg , strlen(msg));
        }
        else if(!strcmp(instruction.str, "/load")){ 
            char load_char[20] = {0};
            char len_char[20] = {0};
            int load = get_load();

            sprintf(load_char, "%d", load);
            int len = strlen(load_char)+1;
            sprintf(len_char, "%d", len);
            //Construct message
            strcat(msg, len_char+1);
            strcat(msg, "\n\n");
            strcat(msg, load_char);
            strcat(msg, "%");
            strcat(msg, "\n");

            write(client_msg, msg , strlen(msg));
        }   
        else{
            write(client_msg, msg , strlen(msg));
        }

        free_string(&instruction);
        close(client_msg);
        printf("\n------------------Data sent, closing connection------------------\n\n");
    }
    return 0;
}
