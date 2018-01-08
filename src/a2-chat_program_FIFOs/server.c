/*
    server part
    Author: Ray Chen
    Last Modified: Oct. 21, 2017
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>

#define N 5

int current_client_num = 0;
int MAX_CLINET_NUM;
char username_list[5][512];
int user_send_list[5][5]; // user's send list

// check if there is another client use specified username
// return 0 -> it has same username
// return 1 -> it doesn't has same username
int check_user_list(char* username){
    for(int i=0; i<5; i++){
        if(strcmp(username_list[i], username) == 0) {
            printf("same username in username list");   
            return 0;
        }
    }
    return 1;
}

void server_open(char *baseName, char* username, int pipe_index){
    char fifo_name[50];
    int fd;
    char message[100];
    
    printf("server_open\n");
    printf("chatting session = %s\n", username);

    snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.out", baseName, pipe_index);
    fd = open(fifo_name, O_WRONLY);
    if(fd == -1) perror("Failed reading fifo in server side handle_connection_request()\n");

    // if there is a same username in username_list
    if(check_user_list(username) == 0){
        printf("current client number = %d\n", current_client_num);
        printf("NAME_FAIL\n");
        write(fd, "NAME_FAIL", sizeof("NAME_FAIL"));
        close(fd);
        return;
    }

    if(current_client_num < MAX_CLINET_NUM){
        write(fd, "SUCCESS", sizeof("SUCCESS"));
        current_client_num++;

        // fill username_list
        memset(username_list[pipe_index-1], 0, sizeof(username_list[pipe_index-1]));
        strcat(username_list[pipe_index-1], username);
        printf("username = %s\n", username_list[pipe_index-1]);

        memset(message, 0, sizeof(message));
        snprintf(message, sizeof(message), "[server] User ’%s’ connected on FIFO %d", username, pipe_index);
        user_send_list[pipe_index-1][0] = pipe_index-1;
        sleep(1);
        write(fd, message, sizeof(message));
        close(fd);
    } else{
        write(fd, "FAIL", sizeof("FAIL"));
        close(fd);
    }

    printf("current client number = %d\n", current_client_num); 
    printf("\n");

}

void server_who(char *baseName, int pipe_index){
    char fifo_name[50];
    int fd;
    char message[100];
    int current_user[5] = {6, 6, 6, 6, 6};
    char num_str[4];
    int n = 0;
    char tem[100];

    snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.out", baseName, pipe_index);
    fd = open(fifo_name, O_WRONLY);
    if(fd == -1) perror("Failed reading fifo in server side server_who()\n");

    memset(message, 0, sizeof(message));
    strcat(message, "[server] Current users: ");
    for(int i=0; i<5; i++){
        if(strcmp(username_list[i], "") != 0){
            current_user[i] = i;
        }
    }

    for(int j=0; j<5; j++){
        if(current_user[j] != 6){
            memset(num_str, 0, sizeof(num_str));
            n += 1;
            snprintf(num_str, sizeof(num_str), "[%d] ", n);
            strcat(message, num_str);
            strcat(message, username_list[j]);
            strcat(message, " ");
        }
    }


    memset(tem, 0, sizeof(tem));
    strcat(tem, message);
    printf("tem = %s\n", tem);
    write(fd, tem, sizeof(tem));
    close(fd);
    
}

void server_to(char *baseName, char *input, int pipe_index){
    char fifo_name[50];
    int fd;
    char message[100];
    char recipients[20];

    printf("server_to\n");

    // clean recipients
    memset(recipients, 0, sizeof(recipients));

    snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.out", baseName, pipe_index);
    fd = open(fifo_name, O_WRONLY);
    if(fd == -1) perror("Failed reading fifo in server side server_who()\n");

    memset(message, 0, sizeof(message));
    strcat(message, "[server] recipients added: ");

    for(int k=0; k<5; k++){
        printf("send_list: %d\n", user_send_list[pipe_index-1][k]);
    }

    int i = 3;
    int j = 0;
    do{
        if(input[i] != ' ' && input[i] != '\n'){
            recipients[j] = input[i];
            j++;
        }else{  // finish one user_name
            recipients[j] = '\0';
            printf("recipients = %s\n", recipients);
            
            // judge if it is in the username_list
            for(int k=0; k<5; k++){
                if(strcmp(username_list[k], recipients) == 0){
                    strcat(message, recipients);
                    strcat(message, " ");
                    
                    int check_flag = 0; // 1 -> it has existed
                    // check if it is in user_send_list
                    for(int q=0; q<5; q++){
                        if(user_send_list[pipe_index-1][q] == k){check_flag = 1;}
                    }

                    if(check_flag == 0) {
                        for(int p=0; p<5; p++){
                            if(user_send_list[pipe_index-1][p] == 6) {
                                user_send_list[pipe_index-1][p] = k;
                                break;
                            }
                        }
                    }
                }
            }
            j = 0;
        }
        i++;
    }while(input[i] != '\0');
    
    write(fd, message, sizeof(message));
    close(fd);

    for(int k=0; k<5; k++){
        printf("send_list: %d\n", user_send_list[pipe_index-1][k]);
    }
}

void server_chat(char *baseName, char *input, int pipe_index){
    char fifo_name[50];
    int fd;
    // if size message is too large, the client will get over one times message stream
    char message[256];

    printf("server_chat\n");

    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "[%s] %s", username_list[pipe_index-1], input+2);

    for(int i=0; i<5; i++){
        if(user_send_list[pipe_index-1][i] != 6){
            snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.out", baseName, user_send_list[pipe_index-1][i] + 1);
            fd = open(fifo_name, O_WRONLY);
            if(fd == -1) perror("Failed reading fifo in server side server_chat()\n");
            write(fd, message, sizeof(message));
            close(fd);
        }
    }

    for(int k=0; k<5; k++){
        printf("send_list: %d\n", user_send_list[pipe_index-1][k]);
    }
}

void server_close(char *baseName, int pipe_index){
    int fd;
    char fifo_name[50];

    printf("server_close\n");
    current_client_num--;

    // clear the username in username_list
    memset(username_list[pipe_index-1], 0, sizeof(username_list[pipe_index-1]));
    strcat(username_list[pipe_index-1], "");
    // clear the username_send_list
    for(int i=0; i<5; i++){
        user_send_list[pipe_index-1][i] = 6;
    }
    // clear someone's send_list which has closed client
    for(int m=0; m<5; m++){
        for(int n=0; n<5; n++){
            if(user_send_list[m][n] == pipe_index-1){ user_send_list[m][n] = 6;}
        }
    }

    snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.out", baseName, pipe_index);
    fd = open(fifo_name, O_WRONLY);
    if(fd == -1) perror("Failed reading fifo in server side server_chat()\n");

    write(fd, "[server] done", sizeof("[server] done"));   
    close(fd);
}


void s_handle_command(char *baseName, char *input, int pipe_index){
    int break_point;
    char cmd_content[256];
    char cmd[16];

    //cmd is the main command that we need
    for(int i=0; i<sizeof(input); i++){
        cmd[i] = input[i];
        if(input[i] == ' ' || input[i] == '\n'){
            cmd[i] = '\0';
            break_point = i;
            break;
        }
    }
    memset(cmd_content, 0, sizeof(cmd_content));
    if(strcmp(cmd, "open") == 0){
        for(int j=0; j<sizeof(input); j++){
            cmd_content[j] = input[j+break_point+1];
            if(input[j+break_point+1] == '\n') {cmd_content[j] = '\0'; break;}
            if(input[j+break_point+1] == '\0') { break; }
        }
        server_open(baseName, cmd_content, pipe_index);
    }
    else if(strcmp(cmd, "who") == 0){ server_who(baseName, pipe_index); }
    else if(strcmp(cmd, "to") == 0){ server_to(baseName, input, pipe_index); }
    else if(strcmp(cmd, "<") == 0) { server_chat(baseName, input, pipe_index); }
    else if(strcmp(cmd, "close") == 0) { server_close(baseName, pipe_index); }
}

void initialize(){
    // initialize username_list
    for(int i=0; i<5; i++){
        memset(username_list[i], 0, sizeof(username_list[i]));
        strcat(username_list[i], "");
    }

    for(int m=0; m<5; m++){
        for(int n=0; n<5; n++){
            user_send_list[m][n] = 6;
        }
    }
}

void main_server(char* baseName, int nclient){
	int is_locked;
	char buf[256]; // maybe too small
	char fifo_name[50];
	int timeout;
	int fd[6]; // five fd fo FIFOS, one fd for Stdin
	struct pollfd pollfd[6];

    printf("Chat server begins [nclient = %d]\n", nclient);
    MAX_CLINET_NUM = nclient;

    // initialize
    initialize();

    for(int i=0; i<5; i++){
    	snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.in", baseName, i+1);
    	fd[i] = open(fifo_name, O_RDONLY);
    	if(fd[i] == -1) perror("Failed reading fifo in server side");

    	pollfd[i].fd = fd[i];
    	pollfd[i].events = POLLIN;
    	pollfd[i].revents = 0;
    }

    // for the standard input
    fd[5] = STDIN_FILENO;
    pollfd[5].fd = fd[5];
    pollfd[5].events = POLLIN;
    pollfd[5].revents = 0;

    timeout = 0;
    // loop all the time until terminate
    while(1){
    	int rval = poll(pollfd, N+1, timeout);
    	for(int j=0; j<6; j++){
    		if(pollfd[j].revents & POLLIN){
    			printf("detect the string steam from fd[%d]\n", j);

    			memset(buf, 0, sizeof(buf));
    			read(fd[j], buf, sizeof(buf));
    			printf("%s\n", buf);

                if(j != 5){ s_handle_command(baseName, buf, j+1); } 
                
                // if (strstr(buf, "exit") != NULL) exit(0);
                if(strcmp(buf, "exit") == 0) exit(0);
    		}
    	}
    }
    
}

            







