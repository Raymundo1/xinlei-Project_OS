#ifndef _server_h_
#define _server_h_


int current_client_num;
int MAX_CLIENT_NUM;
char username_list[5][512];
int user_send_list[5][5];
int number_pollfd_s;
char activity_report[6][512];
// int alarm_flag;
int get_msg_times[5];

void initialize();
int create_listen_socket(int port_number);
int check_user_list(char* username);
void server_open(char* username, int sock);
void server_who(int sock);
void server_to(char* input, int sock);
void server_chat(char* input, int sock);
void server_close(int sock);
void s_handle_command(char* buf, int sock);
void main_server(int port_number, int nclient);

void handle_alarm(int sig);


#endif