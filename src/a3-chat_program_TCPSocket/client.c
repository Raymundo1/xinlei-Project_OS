#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define KAL_char 0x6
#define KAL_length 5
#define KAL_interval 1.5

int global_sock;

int make_connection(int port_number, char* server_address, char* input){
	int sock = 0;
	struct sockaddr_in serv_addr;
	struct hostent *hp;
	char message[80];
	char buf[80];


	hp = gethostbyname(server_address);
	if(hp == (struct hostent*) NULL){
		perror("Your hostname is invalid, please re-start your client");
		exit(0);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	memcpy((char *) &serv_addr.sin_addr, hp->h_addr, hp->h_length);
	serv_addr.sin_family = hp->h_addrtype;
	serv_addr.sin_port = htons(port_number);


	if((sock = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0){
		perror("Socket creation error");
		printf("Socket creation failed\n");
		printf("a3chat_client: ");
		fflush(stdout);	
		return 0; //connection failed
	}

	if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
		perror("Connection failed");
		printf("Server is not runnning\n");
		close(sock);
		printf("a3chat_client: ");
		fflush(stdout);	
		return 0; //connection failed
	}

	// send "open" message
	memset(message, 0, sizeof(message));
	strcat(message, input);
	send(sock, message, sizeof(message), 0);

	// read the result
	memset(buf, 0, sizeof(0));
	recv(sock, buf, sizeof(buf), 0);

	//check result
	if(strcmp(buf, "SUCCESS") == 0){
		return sock; //connection succeed
	}
	else if(strcmp(buf, "FAIL") == 0){
		printf("Sorry, we have reached out client limit, please try later\n");
		close(sock);
		printf("a3chat_client: ");
		fflush(stdout);	
		return 0; //connection failed
	}
	else if(strcmp(buf, "NAME_FAIL") == 0){
		printf("Sorry, your username is be used by other client, please try another username\n");
		printf("a3chat_client: ");
		fflush(stdout);	
		close(sock);
		return 0; //connection failed
	}

}

void c_sent_command(char* input, int sock){
	char cmd[16];
	char buf[20];
	char tem[120];

	// cmd is the main command that we need
	for(int i=0; i<sizeof(input); i++){
		cmd[i] = input[i];

		if(input[i] == ' ' || input[i] == '\n'){
			cmd[i] = '\0';
			break;
		}
	}

	if(strcmp(cmd, "exit") == 0){
		//process of close
		memset(tem, 0, sizeof(tem));
		strcat(tem, input);
		send(sock, "close", sizeof("close"), 0);
	}
	else if(strcmp(cmd, "who") == 0 || strcmp(cmd, "to") == 0 || strcmp(cmd, "<") == 0){
		memset(tem, 0, sizeof(tem));
		strcat(tem, input);
		send(sock, tem, sizeof(tem), 0);
	}
	else if(strcmp(cmd, "close") == 0){
		memset(tem, 0, sizeof(tem));
		strcat(tem, input);
		send(sock, tem, sizeof(tem), 0);
	}
	else if(strcmp(cmd, "open") == 0){
		printf("The chat session is already going on\n");
		printf("a3chat_client: ");
		fflush(stdout);
	}
	else{
		printf("Unidentifiable Command\n");
		printf("a3chat_client: ");
		fflush(stdout);
	}
}

void* sendALM(void* arg){
	char message[KAL_length+1] = {'\x6', '\x6', '\x6', '\x6', '\x6', '\0'};

	while(1){
		sleep(1.5);
		send(global_sock, message, sizeof(message), 0);
	}
}

void main_client(int port_number, char* server_address){
	char input[256];
	char get_in[256];
	char cmd[10];
	struct pollfd pollfd_c[2];
	int timeout = 0;
	int sock;
	int chat_open = 0; // 1 -> chat has opened
	int end_client = 0; // 0 -> False, 1 -> True
	int exit_flag = 0;
	pthread_t tid_c;
	struct hostent *hp;
	char serverIP[20];

	// get the host
	hp = gethostbyname(server_address);
	if(hp == (struct hostent*) NULL){
		perror("Your hostname is invalid, please re-start your client");
		exit(0);
	}

	// get the number notation ip address
	if(inet_ntop(AF_INET, hp->h_addr, serverIP, sizeof(serverIP)) == NULL){
		perror("inet_ntop failed()");
		exit(0);
	}

	printf("Chat client begins (server '%s' [%s], port %d)\n", server_address, serverIP, port_number);


	// initialize pollfd_c
	memset(pollfd_c, 0, sizeof(pollfd_c));
	for(int n=0; n<2; n++){
		pollfd_c[n].fd = -1;
	}

	pollfd_c[0].fd = STDIN_FILENO;
	pollfd_c[0].events = POLLIN;
	pollfd_c[0].revents = 0;


	printf("a3chat_client: ");
	fflush(stdout);	
	while(1){
		if(poll(pollfd_c, 2, timeout) < 0){
			perror("poll() failed");
			break;
		}

		for(int i=0; i<2; i++){
			if(pollfd_c[i].revents & POLLIN){

				if(pollfd_c[i].fd == STDIN_FILENO){
					// printf("Terminal is readable = %d\n", STDIN_FILENO);

					memset(input, 0, sizeof(input));
					read(STDIN_FILENO, input, sizeof(input));

					for(int i=0; i<sizeof(input); i++){
						cmd[i] = input[i];
						if(input[i] == ' ' || input[i] == '\n'){
							cmd[i] = '\0';
							break;
						}
					}

					if(chat_open == 0){ // chat hasn't opened
						if(strcmp(cmd, "open") == 0){
							sock = make_connection(port_number, server_address, input);
							if(sock > 0){
								// set the pollfd_c
								pollfd_c[1].fd = sock;
								pollfd_c[1].events = POLLIN;
								pollfd_c[1].revents = 0;


								global_sock = sock;
								// create thread to send keepalive message
								if(pthread_create(&tid_c, NULL, &sendALM, NULL) < 0){
									perror("pthread_create() failed");
								}

								chat_open = 1;
							}
						}
						else if(strcmp(cmd, "exit") == 0){
							end_client = 1;
							break;
						}
						else{
							printf("please use open or exit command\n");
							printf("a3chat_client: ");
							fflush(stdout);	
						}
					}else{ // chat_open == 1
						if(strcmp(cmd, "exit") == 0){
							exit_flag = 1;
							c_sent_command(input, sock);
						} else{
							// printf("input = %s\n", input);
							c_sent_command(input, sock);
						}	
					}

					
				}
				else{ // fd for socket
					// printf("socket %d is readable\n", pollfd_c[i].fd);
					memset(get_in, 0, sizeof(get_in));
					recv(pollfd_c[i].fd, get_in, sizeof(get_in), 0);

					printf("%s\n", get_in);

					if(strcmp(get_in, "[server] done") == 0) {
						pthread_cancel(tid_c); 
						close(sock);
						chat_open = 0;

						if(exit_flag == 1){
							end_client = 1;
						}
					}

					printf("a3chat_client: ");
					fflush(stdout);					
				}
			}

		}
		if(end_client == 1){ exit(0); }
	}


}