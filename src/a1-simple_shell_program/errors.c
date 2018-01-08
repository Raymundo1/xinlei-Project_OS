#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

void err_chdir(){
	printf("YOU MAY USE TWO SPACES BETWEEN CD AND PATHNAME\n");
	perror("chdir failed");	
}

void err_get_current_dir(){
	perror("Get current directory failed");
}

void err_get_env(char* env){
	perror("Get environment variable failed");
	printf("please check if the %s is exist in env variables\n", env);
}

void err_exit(char* msg){
	perror(msg);
	exit(0);
}

void err_file_open(char* filename){
	perror("Error while opening the file");
	printf("please check if the %s is exist \n", filename);
	exit(EXIT_FAILURE);
}