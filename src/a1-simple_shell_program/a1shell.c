/*
	a1shell program
	Author: Xinlei Chen
	Last modified time: October 2, 2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include "errors.h"

/* ----------------- child monitor process --------------------- */
void print_local_time(){
	time_t rawtime;
	struct tm *timeinfo;
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("%s", asctime(timeinfo));
}

void print_avg_numpro(){
	char ch;
 	FILE *fp;
 	int space_num = 0;
 		
 	printf("Load average: ");

 	fp = fopen("/proc/loadavg", "r"); // read mode
	
	// read error
 	if (fp == NULL){
 		err_file_open("/proc/loadavg");
 	}

 	while( (ch = fgetc(fp) ) != EOF){
 		
 		if(ch == ' '){
			space_num += 1;
			
			if(space_num < 3){
				printf(", ");			
			}else if(space_num == 3) {
				printf("\n");
				printf("Processes: ");
			}else { printf("\n\n"); break;} 		
 		
 		}else {
 			printf("%c", ch); 
 		}
 	}
 	fclose(fp);
}

void quit(){
	_exit(0);
}

void set_signal_handler(){
	signal(SIGHUP, quit);
}

/* a1monitor Process */
void child_process(char *interval){
	int time_interval;
	
	time_interval = atoi(interval);
	
	// get error in changing interval
	if (time_interval == 0){
		perror("what you type is not integer/n");
	}
	
	// loop until parent process terminate	
	while(1){
		set_signal_handler();
		printf("a1monitor: ");
		print_local_time();
		print_avg_numpro();
		sleep(time_interval);
	}
}
/* ------------------------------------------------------ */

/* ----------------- parent process --------------------- */
// tool function
void upper_string(char* string){
	int c = 0;
 
    while (string[c] != '\0') {
      if (string[c] >= 'a' && string[c] <= 'z') {
         string[c] = string[c] - 32;
      }
      c++;
   }
		
}

void cmd_cd(char *pathname){
	char cwd[2048];
	char *env;
	char tem[1024];	
	char tem_rest[1024] = "";

	if(getcwd(cwd, sizeof(cwd)) == NULL)
		err_get_current_dir();
	
	if(pathname[0] == '\0'){
		env = getenv("HOME");
 		if (env == NULL)
 			err_get_env("HOME");
 		strcpy(cwd, env);
	}else if(pathname[0] == '$'){

		for(int j=1; j<sizeof(pathname); j++){
			tem[j-1] = pathname[j];
			
			if(pathname[j] == '\n'){
				tem[j-1] = '\0';				
				break;
			}

			if(pathname[j] == '/'){
				tem[j-1] = '\0';
				
				int k = j;
				do{
					tem_rest[k-j] = pathname[k];
					k += 1;

				}while(pathname[k] != '\n');
				tem_rest[k-j] = '\0'; 		
			}			
		}
		
		upper_string(tem);

		// situation for root
		if(strcmp(tem, "ROOT") == 0){
			strcpy(tem, "HOME");
		}

		env = getenv(tem);
		if (env == NULL)
			err_get_env(tem);
		
		strcpy(cwd, env);
		if(tem_rest[0] != '\0')
			strcat(cwd, tem_rest);
		
	}else if(pathname[0] != '/'){
		strcat(cwd, "/");
		strcat(cwd, pathname);
	}else {
		strcpy(cwd, pathname);
	}	
	
	for(int i=0; i<sizeof(cwd); i++){
		if(cwd[i] == '\n'){
			cwd[i] = '\0';	
			break;	
		}	
	}
	
	if(chdir(cwd) < 0){
		err_chdir();
	}
	
}

void cmd_pwd(){	
	char cwd[1024];
	
	if(getcwd(cwd, sizeof(cwd)) == NULL)
		err_get_current_dir();
	
	printf("%s\n", cwd);
	printf("\n");	
}

void cmd_umask(){
	mode_t current_mask;	

	// Get current mask
	current_mask = umask((mode_t) 0);
	
	// Print current mask. Octal values are used by mask
	printf("Current file mode creation mask is %o\n", (int) current_mask);
	
	printf("S_IRWXU = %o\n", S_IRWXU);
	printf("S_IRWXG = %o\n", S_IRWXG);
	printf("S_IRWXO = %o\n", S_IRWXO);
	printf("\n");
}

void cmd_done(int pid){	
	// sent the signal to child process
	kill(pid, SIGHUP);
	exit(0);
}


static void pr_times(clock_t real, struct tms *tmsstart_total, struct tms *tmsend_total){
   static long clktck = 0;

   if (clktck == 0)    /* fetch clock ticks per second first time */
   	if ((clktck = sysconf(_SC_CLK_TCK)) < 0)
      	perror("sysconf error");
   
   printf("  real:  %7.2f\n", real / (double) clktck);
   printf("  user_ii:  %7.2f\n", (tmsend_total->tms_utime - tmsstart_total->tms_utime) / (double) clktck);
   printf("  sys_ii:   %7.2f\n", (tmsend_total->tms_stime - tmsstart_total->tms_stime) / (double) clktck);
   printf("  user_iii:  %7.2f\n", (tmsend_total->tms_cutime - tmsstart_total->tms_cutime) / (double) clktck);
   printf("  sys_iii:   %7.2f\n", (tmsend_total->tms_cstime - tmsstart_total->tms_cstime) / (double) clktck);
   printf("\n");
}

void cmd_bash(char *input){
	pid_t pid;	
	int status;
	struct tms tmsstart_total, tmsend_total;
	clock_t start_total, end_total;
	
	// start time
	if((start_total = times(&tmsstart_total)) == -1)    
		perror("times error");	
		
	//fork a new child
	pid = fork();
	if(pid == 0){    //the child bash process
		execl("/bin/bash", "bash", "-c", input, (char *) 0);
		_exit(0);
	}else {          // the parent process
		waitpid(pid, &status, 0);
		
		// end time
		if((end_total = times(&tmsend_total)) == -1)
			perror("times error");
	
		pr_times(end_total-start_total, &tmsstart_total, &tmsend_total);
	}
}

void wait_user_stdin(int pid){
	char input[85];
	char cmd[85];
	char cmd_content[85];
	int break_point;
	
	printf("a1shell%% ");
	fgets(input, sizeof(input), stdin);
	
	// cmd is the main command that we need
	for(int i=0; i<sizeof(input); i++){
		cmd[i] = input[i];
		
		if(input[i] == ' ' || input[i] == '\n'){
			cmd[i] = '\0';
			break_point = i;
			break;		
		}	
	}
	
	if(strcmp(cmd, "cd") == 0){
		
		for(int j=0; j<sizeof(input); j++){
			cmd_content[j] = input[j+break_point+1];
			if(input[j+break_point+1] == '\0'){break;}	
		}
		cmd_cd(cmd_content); 
		
	}else if(strcmp(cmd, "pwd") == 0) { cmd_pwd(); }
	else if(strcmp(cmd, "umask") == 0) { cmd_umask(); }
	else if(strcmp(cmd, "done") == 0) { cmd_done(pid); }
	else { cmd_bash( input ); }
}

void parent_process(int pid){
	wait_user_stdin(pid);
}
/* ------------------------------------------------------ */

/* --------------------- main function ------------------ */
void rlimit_set(){
	// Define and object of structure rlimit
	struct rlimit rl;
	
	// First get the time limit on CPU
	getrlimit(RLIMIT_CPU, &rl);
	
	// Change the time limit (10 mins)
	rl.rlim_cur = 600;
	
	// Now call setrlimit() to set the changed value
	setrlimit(RLIMIT_CPU, &rl);
	
}

int main(int argc, char **argv){
	//vars
	pid_t pid;	
	
	// setrlimit
	rlimit_set();
	
	// forks child process
	pid = fork();
	
	if (pid == 0){     /* the child process */
		child_process(argv[1]);
	}else {            /* the parent process */
		while(1){
			parent_process(pid);
		}
	}
	
	return 0;
}
/* ------------------------------------------------------ */



