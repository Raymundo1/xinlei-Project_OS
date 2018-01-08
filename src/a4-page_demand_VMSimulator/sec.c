#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

int sec_accu_reg = 0;
int sec_write_count = 0;


void sec_operation_in_refer(char* binary, int* modification_flag){
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

		sec_accu_reg += result;
	}
	else if(strcmp(operation_code, "01") == 0){
		for(int j=5; j>=0; j--){
			if(value[j] == '1'){
				result += (1 << (5-j));
			}	
		}

		sec_accu_reg -= result;
	}
	else if(strcmp(operation_code, "10") == 0){
		sec_write_count += 1;

		*modification_flag = 1;
	}
}

int sec_page_num(char* binary, int page_num_bit){
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

int sec_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag){
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

    sec_operation_in_refer(binary, modification_flag);
    page_num = sec_page_num(binary, page_num_bit);

    return page_num;
}

void main_sec(int page_len, int memsize){
	uint32_t address;
	clock_t begin;
	clock_t end;
	int nref = 0;
	int phy_mem_page_num;
	int page_num_bit;
	int page_num;
	int flag;
	int page_fault = 0;
	int pointer;
	int start_pointer;
	int modification_flag = 0;
	int sec_flush_count = 0;

	// the maximum number of pages in physical memory
	phy_mem_page_num = memsize / page_len;

	// get the number of bits in page number
	page_num_bit = 32 - (log(page_len) / log(2));

	// create the page_table
	int sec_page_table[phy_mem_page_num];
	// initialize the page table
	for(int i=0; i<phy_mem_page_num; i++){
		sec_page_table[i] = -1;
	}

	// create the used_bit_table
	int used_bit_table[phy_mem_page_num];
	// initialize the bit_table
	for(int i=0; i<phy_mem_page_num; i++){
		used_bit_table[i] = 0;
	}

	// create the flush table
	int sec_flush_table[phy_mem_page_num];
	// initilizae the page table
	for(int i=0; i<phy_mem_page_num; i++){
		// sec_flush_table[i] = -1;
		sec_flush_table[i] = 0;
	}

	// initilize the pointer
	pointer = -1;

	begin = clock();
	while(read(STDIN_FILENO, &address, sizeof(address)) > 0){
		page_num = sec_IntergerToBinary(address, page_num_bit, &modification_flag);

		flag = 0;
		// check if it exist in page table
		for(int i=0; i<phy_mem_page_num; i++){
			if(page_num == sec_page_table[i]){
				used_bit_table[i] = 1;
				flag = 1;

				if(modification_flag == 1){
					sec_flush_table[i] = 1;
				}

				break;
			}
		}

		// if the page number is not in the sec_page_table
		if(flag == 0){
			page_fault += 1;

			start_pointer = pointer;

			pointer += 1;
			if(pointer == phy_mem_page_num) pointer = 0;

			while(used_bit_table[pointer] == 1){
				if(pointer == start_pointer) break;

				used_bit_table[pointer] = 0;

				pointer += 1;
				if(pointer == phy_mem_page_num) pointer = 0;
			}

			if(sec_flush_table[pointer] == 1){
				sec_flush_count += 1;
			}

			if(modification_flag == 1){
					sec_flush_table[pointer] = 1;
				}else{
					sec_flush_table[pointer] = 0;
				}

			used_bit_table[pointer] = 1;
			sec_page_table[pointer] = page_num;
		}

		modification_flag = 0;
		nref += 1;
	}
	end = clock();

	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("[a4vmsim] %d references processed using 'sec' in %.2f sec\n", nref, time_spent);
	printf("[a4vmsim] page faults = %d, write count= %d, flushes= %d\n", page_fault, sec_write_count, sec_flush_count);
	printf("[a4vmsim] Accumulator = %d\n", sec_accu_reg);
}