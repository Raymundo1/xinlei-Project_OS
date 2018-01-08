#ifndef _errors_h_
#define _errors_h_

void err_chdir();
void err_get_current_dir();
void err_get_env(char* env);
void err_exit(char* msg);
void err_file_open(char* filename);

#endif