#ifndef _client_h_
#define _client_h_

// int is_session_going;
// int goto_exit;
int global_sock;
int open_chat_session(int port_number, char* server_address, char* input);
void c_sent_command(char* input, int sock);
void main_client(int port_number, char* server_address);


#endif