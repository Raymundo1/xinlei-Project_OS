#ifndef _server_h_
#define _server_h_

// void s_handle_connection_request(char *baseName, int pipe_index);
int current_client_num;
int MAX_CLINET_NUM;
char username_list[5][20];
int user_send_list[5][5];
// int user_running[5];
int check_user_list(char* username);
void server_open(char* baseName, char* chat_session, int pipe_index);
void server_who(char* baseName, int pipe_index);
void server_to(char* baseName, int pipe_index);
void server_chat(char* baseName, char *input, int pipe_index);
void server_close(char* baseName, int pipe_index);
void s_handle_command(char *baseName, char *input, int pipe_index);
void initialize();
void main_server(char *baseName, int nclient);

#endif