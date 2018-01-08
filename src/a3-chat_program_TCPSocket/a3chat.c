#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "client.h"
#include "server.h"


int main(int argc, char **argv){
	int port_number;


	// for server
	if(argc == 4 && (strcmp(argv[1], "-s") == 0)){
		int nclient = atoi(argv[3]);
		if(nclient == 0){
			perror("Please type integer for last part as max number of clients");
			exit(0);
		}

		if(nclient > 5 || nclient < 0){
			// nclient <= NMAX=5
			printf("Failed, please try again\n");
			printf("Only can serve 5 or less clients\n");
			exit(0);
		}

		port_number = atoi(argv[2]);
		if(port_number == 0){
			perror("Your port number shoud be a 4-digit integer");
			exit(0);
		}

		main_server(port_number, nclient);
	}

	// for client
	else if(argc == 4 && (strcmp(argv[1], "-c") == 0)){
		port_number = atoi(argv[2]);
		if(port_number == 0){
			perror("Your port number shoud be a 4-digit integer");
			exit(0);
		}

		main_client(port_number, argv[3]);
	}

	else{
		printf("Failed, please try again\n");
		exit(0);
	}

	return 0;
}