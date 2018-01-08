#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "none.h"
#include "mrand.h"
#include "lru.h"
#include "sec.h"

int main(int argc, char **argv){
	char policy[10];
	int page_len;
	int memsize;

	if(argc != 4){
		printf("Please put three parameters on this program\n");
		exit(0);
	}

	page_len = atoi(argv[1]);
	if(page_len == 0){
		perror("please use valid $page");
		exit(0);
	}

	memsize = atoi(argv[2]);
	if(memsize == 0){
		perror("please use valid $memsize");
		exit(0);
	}
	
	memset(policy, 0, sizeof(policy));
	strcat(policy, argv[3]);

	printf("a4vmsim [page=%d, mem=%d, %s]\n", page_len, memsize, policy);

	if(strcmp(policy, "none") == 0){
		main_none(page_len, memsize);
	}
	else if(strcmp(policy, "mrand") == 0){
		main_mrand(page_len, memsize);
	}
	else if(strcmp(policy, "lru") == 0){
		main_lru(page_len, memsize);
	}
	else if(strcmp(policy, "sec") == 0){
		main_sec(page_len, memsize);
	}
	else{
		printf("Please use valid $policy\n");
		exit(0);
	}

	return 0;
}