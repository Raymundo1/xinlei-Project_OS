/*
    Main part
    Author: Ray Chen
    Last Modified: Oct. 15, 2017
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "client.h"
#include "server.h"

void open_fifos(char* baseName){
    int fd;
    char fifo_name[50];

    for(int i=0; i<5; i++){
        snprintf(fifo_name, sizeof(fifo_name), "fifo/%s-%d.in", baseName, i+1);

        fd = open(fifo_name, O_RDWR);
        if(fd == -1) perror("Failed reading fifo in client side");
    }
}

int main(int argc, char **argv){
    int  nclient;

    // for server
    if(argc == 4 && (strcmp(argv[1], "-s") == 0)){
        nclient = atoi(argv[3]);
        if(nclient == 0){
            perror("Please type interger for last part as max number of clients");
            exit(0);
        }

        if(nclient > 5 || nclient < 0){
            // nclient <= NAMX=5
            printf("Failed, please try again\n");
            printf("Only can server 5 or less client");
            exit(0);
        }

        open_fifos(argv[2]);
        main_server(argv[2], nclient);
    }
    // for client 
    else if(argc == 3 && (strcmp(argv[1], "-c") == 0)){
        main_client(argv[2]);
    }

    else{
        printf("Failed, please try again\n");
        exit(0);
    }
    return 0;
}