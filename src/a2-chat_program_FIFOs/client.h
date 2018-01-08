#ifndef _client_h_
#define _client_h_

int is_session_going;
int goto_exit;
int find_unlocked_pipe(char *baseName, int* fd, char* input);
void c_sent_command(char* input, int fd_write);
int open_chat_session(char *baseName, char *input);
void main_client(char *baseName);

#endif