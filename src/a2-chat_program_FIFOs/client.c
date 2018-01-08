/*
    client part
    Author: Xinlei Chen
    Last Modified: Oct. 21, 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

int is_session_going = 0; // 0 -> no, 1 -> yes
int goto_exit = 0; // 0 -> no, 1 -> yes

int find_unlocked_pipe(char* baseName, int* fd, char* input){
    char fifo_name_in[50], fifo_name_out[50];
    int connect_success = 0;
    int name_flag = 0; // 1 -> fail
    char buf[80];
    char message[80];
    char lock_message[80];

    // fd[0] -> write  fd[1] -> read
    // find the unlocked pipe and connect to it
    for(int i=0; i<5; i++){
        snprintf(fifo_name_in, sizeof(fifo_name_in), "fifo/%s-%d.in", baseName, i+1);
        fd[0] = open(fifo_name_in, O_RDWR); 
        if(fd[0] == -1) perror("Failed reading fifo in client side\n");

        // lock the file from other process
        lockf(fd[0], F_TLOCK, 0);
        if(lockf(fd[0], F_TEST, 0) == 0){
            int pid = getpid();
            snprintf(lock_message, sizeof(lock_message), "FIFO [%s-%d.in] has been successfully locked by PID [%d]", baseName, i+1, pid);
            printf("%s\n", lock_message);

            memset(message, 0, sizeof(message));
            strcat(message, input);
            // printf("input = %s", message);
            write(fd[0], message, sizeof(message));

            // get the server response back
            snprintf(fifo_name_out, sizeof(fifo_name_out), "fifo/%s-%d.out", baseName, i+1);
            fd[1] = open(fifo_name_out, O_RDONLY);
            memset(buf, 0, sizeof(buf));
            read(fd[1], buf, sizeof(buf));
            if(strcmp(buf, "SUCCESS") == 0) {
                connect_success = 1;
                break; 
            }
            if(strcmp(buf, "FAIL") == 0) { break; }
            if(strcmp(buf, "NAME_FAIL") == 0) { name_flag = 1; break;}
        }
    }

    if(connect_success == 0) {
        
        if(name_flag == 1){
            printf("Sorry, your username is be used by other client, please use another username\n");
            lockf(fd[0], F_ULOCK, 0);
            if(lockf(fd[0], F_TEST, 0) == 0) {printf("Success unlock it\n");}
            return 0; // connection failed by same username
        }

        printf("Sorry we have reached our client limit, please try latter\n");
        // printf("If the file be locked, we will unlock it\n");
        lockf(fd[0], F_ULOCK, 0);
        if(lockf(fd[0], F_TEST, 0) == 0) {printf("Success unlock it\n");}
        return 0; // connection failed
    }
    return 1; // connection succeed
}

void c_sent_command(char* input, int fd_write, int fd_read){
    char cmd[16];
    char buf[20];
    char tem[120];

    //cmd is the main command that we need
    for(int i=0; i<sizeof(input); i++){
        cmd[i] = input[i];

        if(input[i] == ' ' || input[i] == '\n'){
            cmd[i] = '\0';
            break;
        }
    }

    if(strcmp(cmd, "exit") == 0) {
        // process of close
        write(fd_write, "close",  sizeof("close"));
        
        // unlock the file
        lockf(fd_write, F_ULOCK, 0);
        if(lockf(fd_write, F_TEST, 0) == -1) { perror("Unlock Failed"); }
        is_session_going = 0;
        
        // process of exit
        goto_exit = 1;

    }
    else if(strcmp(cmd, "who") == 0 || strcmp(cmd, "to") == 0 || strcmp(cmd, "<") == 0 || strcmp(cmd, "close") == 0){
        memset(tem, 0, sizeof(tem));
        strcat(tem, input);
        write(fd_write, tem, sizeof(tem));

        if (strcmp(cmd, "close") == 0){
            // unlock the file
            lockf(fd_write, F_ULOCK, 0);
            if(lockf(fd_write, F_TEST, 0) == -1) { perror("Unlock Failed");  }
            is_session_going = 0;
        }
    }
    else if(strcmp(cmd, "open") == 0){
        printf("The chat session is already going on\n");
        printf("a2chat_client: ");
        fflush(stdout);
    }
    else { 
        printf("Unidentifiable Command\n"); 
        printf("a2chat_client: ");
        fflush(stdout);
    }
}

// return 1 -> chat session close
// return 0 -> chat session still running 
int open_chat_session(char *baseName, char *input){
    int fd[2]; //fd[0] -> write, fd[1] -> read
    int fd_poll[2];
    struct pollfd pollfd[2];
    int is_succeed;
    char get_in[256];
    
    is_succeed = find_unlocked_pipe(baseName, fd, input);
    if(is_succeed == 1){

        is_session_going = 1;

        fd_poll[0] = fd[1];  // fd[1] -> read
        pollfd[0].fd = fd_poll[0]; 
        pollfd[0].events = POLLIN;
        pollfd[0].revents = 0;

        fd_poll[1] = STDIN_FILENO;
        pollfd[1].fd = fd_poll[1];
        pollfd[1].events = POLLIN;
        pollfd[1].revents = 0;
        
        while(1){
            int rval = poll(pollfd, 2, 0); // poll(pollfd, num, timeout)

            for(int j=0; j<2; j++){
                if(pollfd[j].revents & POLLIN){
                    memset(get_in, 0, sizeof(get_in));
                    read(fd_poll[j], get_in, sizeof(get_in));
                    
                    if(j == 1) { c_sent_command(get_in, fd[0], fd[1]);}
                    else if(j == 0 && strcmp(get_in, "") != 0) {
                        printf("%s\n", get_in);
                        if(strcmp(get_in, "[server] done") == 0){ is_session_going = 0; return 1;}
                        printf("a2chat_client: "); 
                        fflush(stdout);
                    }
                }
            }
        }
    }
    return 0;
}

void main_client(char *baseName){
    char input[120];
    int timeout = 0;
    char cmd[10]; // store the command

    printf("Chat client begins\n");

    // poll the stdin
    while(1){
        if(goto_exit == 1) { exit(0); }

        printf("a2chat_client: ");
        fgets(input, sizeof(input), stdin);
        for(int i=0; i<sizeof(input); i++){
            cmd[i] = input[i];
            if(input[i] == ' ' || input[i] == '\n'){
                cmd[i] = '\0';
                break;      
            }   
        }

        if(strcmp(cmd, "exit") == 0){ exit(0); }
        else if(strcmp(cmd, "open") == 0){
            if(is_session_going == 0) {open_chat_session(baseName, input);}
            else {printf("this is a chat session going, please try later \n");}
        }
        else {printf("please use open or exit command \n");}
    }

}

