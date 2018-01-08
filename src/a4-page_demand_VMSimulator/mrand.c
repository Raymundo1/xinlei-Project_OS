#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

int mrand_accu_reg = 0;
int mrand_write_count = 0;


void mrand_operation_in_refer(char* binary, int* modification_flag){
	char operation_code[3];
	char value[7];
	int result = 0;

	memset(operation_code, 0, sizeof(operation_code));
	memset(value, 0, sizeof(value));

	operation_code[0] = binary[24];
	operation_code[1] = binary[25];
	operation_code[2] = '\0';

	for(int i=0; i<6; i++){
		value[i] = binary[26+i];
	}
	value[6] = '\0';

	if(strcmp(operation_code, "00") == 0){
		for(int j=5; j>=0; j--){
			if(value[j] == '1'){
				result += (1 << (5-j));
			}
		}

		mrand_accu_reg += result;
	}
	else if(strcmp(operation_code, "01") == 0){
		for(int j=5; j>=0; j--){
			if(value[j] == '1'){
				result += (1 << (5-j));
			}	
		}

		mrand_accu_reg -= result;
	}
	else if(strcmp(operation_code, "10") == 0){
		mrand_write_count += 1;

		*modification_flag = 1;
	}
	
}

int mrand_page_num(char* binary, int page_num_bit){
	char value[page_num_bit+1];
	int result = 0;
	
	memset(value, 0, sizeof(value));

	for(int i=0; i<page_num_bit; i++){
		value[i] = binary[i];
	}
	value[page_num_bit] = '\0';

	for(int j=page_num_bit-1; j>=0; j--){
		if(value[j] == '1'){
			result += (1 << (page_num_bit-1-j));
		}
	}

	// printf("binary = %s\n", binary);
	// printf("page_num = %d\n", result);
	return result;
}

int mrand_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag){
	char binary[33];
	int k;
	int page_num;

   	memset(binary, 0, sizeof(binary));
	
	for(int i=31; i>=0; i--){
		k = address >> i;

		if(k&1){
			binary[31-i] = '1';
		}
		else{
			binary[31-i] = '0';
		}
	}
	binary[32] = '\0';

    mrand_operation_in_refer(binary, modification_flag);
    page_num = mrand_page_num(binary, page_num_bit);

    return page_num;
}

int check_visit_table(int rand_position_page_num, int* last_visit_table){
	for(int i=0; i<3; i++){
		
		if(rand_position_page_num == last_visit_table[i]){
			return 1;
		}
	}

	return 0;
}

void main_mrand(int page_len, int memsize){
	uint32_t address;
	clock_t begin;
	clock_t end;
	int nref = 0;
	int phy_mem_page_num;
	int page_num_bit;
	int page_num;
	int flag;
	int rand_position;
	int page_fault = 0;
	int last_visit_position = 0;
	int last_visit_table[3];
	int modification_flag = 0;
	int mrand_flush_count = 0;

	// the maximum number of pages in physical memory
	phy_mem_page_num = memsize / page_len;

	if(phy_mem_page_num < 3){
		printf("In mrand case, the number of pages should be greater than 2\n");
		exit(0);
	}

	// get the number of bits in page number
	page_num_bit = 32 - (log(page_len) / log(2));

	// create the page_table
	int mrand_page_table[phy_mem_page_num];
	// initialize the page table
	for(int i=0; i<phy_mem_page_num; i++){
		mrand_page_table[i] = -1;
	}

	// create the flush table
	int mrand_flush_table[phy_mem_page_num];
	// initilizae the page table
	for(int i=0; i<phy_mem_page_num; i++){
		// mrand_flush_table[i] = -1;
		mrand_flush_table[i] = 0;
	}

	// initilialize the last_visit_table
	for(int j=0; j<3; j++){
		last_visit_table[j] = -1;
	}

	begin = clock();
	while(read(STDIN_FILENO, &address, sizeof(address)) > 0){
		page_num = mrand_IntergerToBinary(address, page_num_bit, &modification_flag);

		// store the last three visited page num
		last_visit_table[last_visit_position] = page_num;
		last_visit_position += 1;

		if(last_visit_position == 3){
			last_visit_position = 0;
		}

		flag = 0;
		// check if it exist in page table
		for(int i=0; i<phy_mem_page_num; i++){
			if(page_num == mrand_page_table[i]){
				flag = 1;
				
				if(modification_flag == 1){
					mrand_flush_table[i] = 1;
				}

				break;
			}

			// if not exist, and table is not full
			if(mrand_page_table[i] == -1){
				page_fault += 1;
				mrand_page_table[i] = page_num;

				if(modification_flag == 1){
					mrand_flush_table[i] = 1;
				}else{
					mrand_flush_table[i] = 0;
				}

				flag = 1;
				break;
			}
		}

		// if page table is full and page number not in page table
		if(flag == 0){
			// the procedure of policy mrand
			page_fault += 1;

			do{
				rand_position = rand() % phy_mem_page_num;
				// test += 1;
			}while(check_visit_table(mrand_page_table[rand_position], last_visit_table));

			if(mrand_flush_table[rand_position] == 1){
				mrand_flush_count += 1;
			}

			if(modification_flag == 1){
				mrand_flush_table[rand_position] = 1;
			}else{
				mrand_flush_table[rand_position] = 0;
			}
			
			mrand_page_table[rand_position] = page_num;
		}

		modification_flag = 0;
		nref += 1;
	}
	end = clock();

	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("[a4vmsim] %d references processed using 'mrand' in %.2f sec\n", nref, time_spent);
	printf("[a4vmsim] page faults = %d, write count= %d, flushes= %d\n", page_fault, mrand_write_count, mrand_flush_count);
	printf("[a4vmsim] Accumulator = %d\n", mrand_accu_reg);

}