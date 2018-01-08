#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#define N 5
#define KAL_count 5

int current_client_num = 0;
int MAX_CLIENT_NUM;
char username_list[5][512];
int user_send_list[5][5];
struct pollfd pollfd_s[7];
int number_pollfd_s = 2;
char activity_report[6][512]; // report[5] is used for storing unexpectivedly client
int get_msg_times[5];


/* initialize the list */
void initialize(){
	// initialize username_list
	for(int i=0; i<5; i++){
		memset(username_list[i], 0, sizeof(username_list[i]));
		strcat(username_list[i], "");
	}

	// initialize user_send_list
	for(int m=0; m<5; m++){
		for(int n=0; n<5; n++){
			user_send_list[m][n] = 6;
		}
	}

	// initilize pollfd_s
	memset(pollfd_s, 0, sizeof(pollfd_s));
	for(int n=0; n<7; n++){
		pollfd_s[n].fd = -1;
	}

	// initilize activity_report
	for(int j=0; j<6; j++){
		memset(activity_report[j], 0, sizeof(activity_report[j]));
		strcat(activity_report[j], "");
	}

	// initialize get_msg_times
	for(int k=0; k<5; k++){
		get_msg_times[k] = 0;
	}

}

/* create the listen socket which to listen the connection request */
int create_listen_socket(int port_number){
	int listen_sd = -1;
	int return_value;
	int on = 1;
	struct sockaddr_in server_addr;

	// create listen socket
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sd < 0){
		perror("Listen Socket creation failed");
		exit(-1);
	}


	// set socket to be nonblocking
	return_value = ioctl(listen_sd, FIONBIO, (char*) &on);
	if(return_value < 0){
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	// bind the socket
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port_number);
	return_value = bind(listen_sd, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if(return_value < 0){
		perror("bind() failed, please use valid port number");
		close(listen_sd);
		exit(-1);
	}

	return listen_sd;
}

// check if there is another client use specified username
// return 0 -> it has same username
// return 1 -> it doesn't have same username
int check_user_list(char* username){
	for(int i=0; i<5; i++){
		if(strcmp(username_list[i], username) == 0){
			// printf("same username in username list\n");
			return 0;
		}
	}
	return 1;
}

void server_open(char* username, int sock){
	int index;
	char message[100];

	// printf("server_open\n");
	// printf("chatting session = %s\n", username);

	for(int i=2; i<7; i++){
		if(pollfd_s[i].fd == sock){
			index = i - 2;
			break;
		}
	}


	if(check_user_list(username) == 0){
		send(sock, "NAME_FAIL", sizeof("NAME_FAIL"), 0);
		close(sock);
		pollfd_s[index+2].fd = -1;
		number_pollfd_s -= 1;
		return;
	}

	if(current_client_num < MAX_CLIENT_NUM){
		send(sock, "SUCCESS", sizeof("SUCCESS"), 0);
		current_client_num += 1;

		//fill username_list
		memset(username_list[index], 0, sizeof(username_list[index]));
		strcat(username_list[index], username);
		// printf("username = %s\n", username);

		//fill user_send_list
		memset(message, 0, sizeof(message));
		snprintf(message, sizeof(message), "[server] connected \n[server] User '%s' logged in", username);
		user_send_list[index][0] = index;
		sleep(1);
		send(sock, message, sizeof(message), 0);
	}else{
		send(sock, "FAIL", sizeof("FAIL"), 0);
		close(sock);
		pollfd_s[index+2].fd = -1;
		number_pollfd_s -= 1;
	}

	// printf("current client number = %d\n", current_client_num);
	// printf("\n");
}

void server_who(int sock){
	char message[100];

	// printf("server_who\n");

	memset(message, 0, sizeof(message));
	strcat(message, "[server] Current users: ");
	for(int i=0; i<5; i++){
		if(strcmp(username_list[i], "") != 0){
			strcat(message, username_list[i]);
			strcat(message, ", ");
		}
	}

	send(sock, message, sizeof(message), 0);
}

void server_to(char* input, int sock){
	char message[100];
	char recipients[20];
	int index;

	// printf("server_to\n");

	// get the index of sock
	for(int i=2; i<7; i++){
		if(pollfd_s[i].fd == sock){
			index = i - 2;
			break;
		}
	}

	// initialize
	memset(recipients, 0, sizeof(recipients));
	memset(message, 0, sizeof(message));

	strcat(message, "[server] recipients added: ");

	int i = 3;
	int j = 0;
	do{
		if(input[i] != ' ' && input[i] != '\n'){
			recipients[j] = input[i];
			j++;
		}else{ // finish one userName
			recipients[j] = '\0';
			// printf("recipients = %s\n", recipients);

			// judge if it is in the username_list
			for(int k=0; k<5; k++){
				if(strcmp(username_list[k], recipients) == 0){
					strcat(message, recipients);
					strcat(message, " ");

					int check_flag = 0; // 1 -> it has existed
					// check if it is in user_send_list
					for(int q=0; q<5; q++){
						if(user_send_list[index][q] == k){check_flag = 1; break;}
					}

					if(check_flag == 0){
						for(int p=0; p<5; p++){
							if(user_send_list[index][p] == 6){
								user_send_list[index][p] = k;
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

	send(sock, message, sizeof(message), 0);

	// for(int m=0; m<5; m++){
	// 	printf("send_list: %d\n", user_send_list[index][m]);
	// }
}

void server_chat(char* input, int sock){
	char message[256];
	int index;

	// printf("server_chat\n");

	// get the index of sock
	for(int i=2; i<7; i++){
		if(pollfd_s[i].fd == sock){
			index = i - 2;
			break;
		}
	}

	memset(message, 0, sizeof(message));
	snprintf(message, sizeof(message), "[%s] %s", username_list[index], input+2);

	// int number;
	for(int i=0; i<5; i++){
		if(user_send_list[index][i] != 6){
			// printf("pollfd_s = %d\n", pollfd_s[i+2].fd);
			// number = send(pollfd_s[user_send_list[index][i]+2].fd, message, sizeof(message), 0);
			send(pollfd_s[user_send_list[index][i]+2].fd, message, sizeof(message), 0);
			// printf("number = %d\n", number);
		}
	}

	// for(int m=0; m<5; m++){
	// 	printf("send_list: %d\n", user_send_list[index][m]);
	// }
}

void server_close(int sock){
	int index;

	// printf("server_close\n");

	current_client_num--;

	for(int i=2; i<7; i++){
		if(pollfd_s[i].fd == sock){
			index = i - 2;
			break;
		}
	}

	//clear the username in username_list
	memset(username_list[index], 0, sizeof(username_list[index]));
	strcat(username_list[index], "");

	//clear the user_send_list
	for(int i=0; i<5; i++){
		user_send_list[index][i] = 6;
	}

	//clear someone's send_list which has closed client
	for(int m=0; m<5; m++){
		for(int n=0; n<5; n++){
			if(user_send_list[m][n] == index){ user_send_list[m][n] = 6;}
		}
	}

	// set -1 into get_msg_times
	get_msg_times[index] = -1;

	send(sock, "[server] done", sizeof("[server] done"), 0);
	close(sock);

	pollfd_s[index+2].fd = -1;
	number_pollfd_s -= 1;

	// printf("number_pollfd_s = %d\n", number_pollfd_s);
}

void s_handle_command(char* input, int sock){
	int break_point;
	char cmd_content[256];
	char cmd[16];
	int size = 250;

	// cmd is the main command what we need
	for(int i=0; i<size; i++){
		cmd[i] = input[i];
		if(input[i] == ' ' || input[i] == '\n'){
			cmd[i] = '\0';
			break_point = i;
			break;
		}
	}
	memset(cmd_content, 0, sizeof(cmd_content));
	if(strcmp(cmd, "open") == 0){
		for(int j=0; j<size; j++){
			cmd_content[j] = input[j+break_point+1];
			if(input[j+break_point+1] == '\n') {cmd_content[j] = '\0';  break;}
			if(input[j+break_point+1] == '\0') {break;}
		}

		// printf("cmd_content = %s\n", cmd_content);
		server_open(cmd_content, sock);
	}
	else if(strcmp(cmd, "who") == 0) { server_who(sock); }
	else if(strcmp(cmd, "to") == 0) { server_to(input, sock); }
	else if(strcmp(cmd, "<") == 0) { server_chat(input, sock); }
	else if(strcmp(cmd, "close") == 0) { server_close(sock); }
}

/*Put the content responding to alive message*/
void handle_alive_msg(int sock){
	int index;
	char tem[512];
	time_t curtime;

	memset(tem, 0, sizeof(tem));

	for(int i=2; i<7; i++){
		if(pollfd_s[i].fd == sock){
			index = i - 2;
			break;
		}
	}

	if(strcmp(username_list[index], "") != 0){
		time(&curtime);
		snprintf(tem, sizeof(tem), "'%s' [sockfd= %d]:%s", username_list[index], pollfd_s[index+2].fd, ctime(&curtime));

		memset(activity_report[index], 0, sizeof(activity_report[index]));
		strcat(activity_report[index], tem);
	}
}

void* reportAct(void* arg){
	while(1){
		sleep(15);

		printf("activity report:\n");
		for(int i=0; i<6; i++){
			if(strcmp(activity_report[i], "") != 0){
				
				printf("%s", activity_report[i]);

				if(i == 5){
					memset(activity_report[5], 0, sizeof(activity_report[5]));
					strcat(activity_report[5], "");
				}

				if(strcmp(username_list[i], "") == 0){
					memset(activity_report[i], 0, sizeof(activity_report[i]));
					strcat(activity_report[i], "");
				}
			}
		}
		printf("\n");
	}
}

/*The thread which to check if it has unexpctively quit client, free the space*/
void* alarmAct(void* arg){
	char tem[512];
	time_t curtime;
	int alive_client[5];

	memset(tem, 0, sizeof(tem));

	while(1){
		// before alarm check the alive client and set msg
		for(int j=0; j<5; j++){
			alive_client[j] = 0;
			if(strcmp(username_list[j], "") != 0){
				alive_client[j] = 1;
			}
		}

		//test print
		// for(int w=0; w<5; w++){
		// 	printf("alive_client[%d] = %d\n", w, alive_client[w]);
		// }

		sleep(7.5);

		for(int i=0; i<5; i++){
			if(alive_client[i] == 1){

				// check the alive message times
				// print to see
				// for(int q=0; q<5; q++){
				// 	printf("get_msg_times[%d] = %d\n", q, get_msg_times[q]);
				// }

				if(get_msg_times[i] < KAL_count){ 
					if(get_msg_times[i] == -1){ // close or exit
						get_msg_times[i] = 0;

					}
					else{ // unexpectedly quit
						// write into the postion 6 which for the crush client message
						memset(tem, 0, sizeof(tem));
						time(&curtime);
						snprintf(tem, sizeof(tem), "'%s' [sockfd= %d]: connection closed, loss of keepalive messages detected at %s", username_list[i], pollfd_s[i+2].fd, ctime(&curtime));
						memset(activity_report[5], 0, sizeof(activity_report[5]));
						strcat(activity_report[5], tem);

						// clean the original position
						memset(activity_report[i], 0, sizeof(activity_report[i]));
						strcat(activity_report[i], "");

						/* free resource */
						current_client_num--;

						//clear the username in username_list
						memset(username_list[i], 0, sizeof(username_list[i]));
						strcat(username_list[i], "");

						//clear the user_send_list
						for(int k=0; k<5; k++){
							user_send_list[i][k] = 6;
						}

						//clear someone's send_list which has closed client
						for(int m=0; m<5; m++){
							for(int n=0; n<5; n++){
								if(user_send_list[m][n] == i){ user_send_list[m][n] = 6;}
							}
						}

						close(pollfd_s[i+2].fd);
						pollfd_s[i+2].fd = -1;
						number_pollfd_s -= 1;
					}
				}
			}

			get_msg_times[i] = 0;
			alive_client[i] = 0;
		}
	}
}

void main_server(int port_number, int nclient){
	int listen_sd;
	int end_server = 0; // 0 -> False, 1 -> True
	int close_connection = 0; // 0 -> False. 1 -> True
	int timeout = 0;
	char buf[256];
	int tem_flag;
	pthread_t tid_s;
	pthread_t tid_alarm;
	
	printf("Chat server begins [port= %d] [nclient= %d]\n", port_number, nclient);

	MAX_CLIENT_NUM = nclient;

	initialize();
	listen_sd = create_listen_socket(port_number);

	// start listen (MAXIMUM LISTEN 5 in waiting queue)
	if(listen(listen_sd, 10) < 0){
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	//create thread to alarm
	if(pthread_create(&tid_alarm, NULL, &alarmAct, NULL) < 0){
		perror("pthread_create() failed in alarm");
	}

	// create thread to repost activity
	if(pthread_create(&tid_s, NULL, &reportAct, NULL) < 0){
		perror("pthread_create() failed in report");
	}

	// set up first two descriptor
	pollfd_s[0].fd = listen_sd;
	pollfd_s[0].events = POLLIN;
	pollfd_s[0].revents = 0;

	pollfd_s[1].fd = STDIN_FILENO;
	pollfd_s[1].events = POLLIN;
	pollfd_s[1].revents = 0;

	while(1){
		if(poll(pollfd_s, N+2, timeout) < 0){
			perror("poll() failed");
			break;
		}

		for (int i=0; i<7; i++){
			if(pollfd_s[i].fd != -1){
				
				if(pollfd_s[i].revents & POLLIN){

					if(pollfd_s[i].fd == STDIN_FILENO){  // ready for reading from terminal
						
						// printf("Terminal is readable = %d\n", STDIN_FILENO);
						memset(buf, 0, sizeof(buf));
						read(STDIN_FILENO, buf, sizeof(buf));

						if(strcmp(buf, "exit\n") == 0) {end_server = 1; break;}

					}
					else if(pollfd_s[i].fd == listen_sd){ // ready for reading from listen socket
						// printf("Listening socket is readable = %d\n", listen_sd);

						int new_sd = -1;

						do{
							new_sd = accept(listen_sd, NULL, NULL);
							if(new_sd < 0){
								if(errno != EWOULDBLOCK){
									perror("accept() failed");
									end_server = 1;
								}
								break;
							}

							// printf("New Connection = %d\n", new_sd);

							tem_flag = 0;
							for(int m=2; m<7; m++){
								if(pollfd_s[m].fd == -1){
									pollfd_s[m].fd = new_sd;
									pollfd_s[m].events = POLLIN;
									pollfd_s[m].revents = 0;

									tem_flag = 1;
									number_pollfd_s += 1;
									break;
								}
							}

							if(tem_flag == 0){
								printf("the number of client exceeds 5\n");
								send(new_sd, "FAIL", sizeof("FAIL"), 0);
								sleep(1);
								close(new_sd);
							}
						
						}while(new_sd != -1);
						
					}
					else{ // ready fro reading from client
						// printf("descriptor %d is readable\n", pollfd_s[i].fd);
						// printf("index in pollfd_s = %d\n", i);

						memset(buf, 0, sizeof(buf));
						recv(pollfd_s[i].fd, buf, sizeof(buf), 0);

						// printf("length2 = %d\n", sizeof(buf));
						if(strcmp(buf, "\x6\x6\x6\x6\x6") == 0){
							handle_alive_msg(pollfd_s[i].fd);
							// add one
							get_msg_times[i-2] += 1;
						}else{
							if(buf[0] != '\0'){
								// printf("%s\n", buf);
								handle_alive_msg(pollfd_s[i].fd);
								s_handle_command(buf, pollfd_s[i].fd);
							}
						}
					}
				}
			}
		}

		if(end_server == 1){ pthread_cancel(tid_s); pthread_cancel(tid_alarm); exit(0); }
	}
}