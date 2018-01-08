#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

struct Page{
	int page_num;
	int modification;
};

int lru_accu_reg = 0;
int lru_write_count = 0;


void lru_operation_in_refer(char* binary, int* modification_flag){
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

		lru_accu_reg += result;
	}
	else if(strcmp(operation_code, "01") == 0){
		for(int j=5; j>=0; j--){
			if(value[j] == '1'){
				result += (1 << (5-j));
			}	
		}

		lru_accu_reg -= result;
	}
	else if(strcmp(operation_code, "10") == 0){
		lru_write_count += 1;

		*modification_flag = 1;
	}

}

int lru_page_num(char* binary, int page_num_bit){
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

int lru_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag){
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

    lru_operation_in_refer(binary, modification_flag);
    page_num = lru_page_num(binary, page_num_bit);

    return page_num;
}

void set_most_recent(int index, struct Page* lru_page_table, int page_num, int modification){
	for(int i = index-1; i >= 0; i--){
		lru_page_table[i+1] = lru_page_table[i];
	}

	lru_page_table[0].page_num = page_num;
	lru_page_table[0].modification = modification;
}

void main_lru(int page_len, int memsize){
	uint32_t address;
	clock_t begin;
	clock_t end;
	int nref = 0;
	int phy_mem_page_num;
	int page_num_bit;
	int page_num;
	int flag;
	int page_fault = 0;
	int modification_flag = 0;
	int lru_flush_count = 0;

	// the maximum number of pages in physical memory
	phy_mem_page_num = memsize / page_len;

	// get the number of bits in page number
	page_num_bit = 32 - (log(page_len) / log(2));

	// create the page table
	struct Page lru_page_table[phy_mem_page_num];
	// initialize the page table
	for(int i=0; i<phy_mem_page_num; i++){
		lru_page_table[i].page_num = -1;
		lru_page_table[i].modification = 0;
	}

	begin = clock();
	while(read(STDIN_FILENO, &address, sizeof(address)) > 0){
		page_num = lru_IntergerToBinary(address, page_num_bit, &modification_flag);

		flag = 0;
		// check if it exist in page table
		for(int i=0; i<phy_mem_page_num; i++){
			// if exist in lru_page_table
			if(page_num == lru_page_table[i].page_num){
				flag = 1;

				if(modification_flag == 1){
					set_most_recent(i, lru_page_table, page_num, 1);
				}else{
					set_most_recent(i, lru_page_table, page_num, lru_page_table[i].modification);
				}

				break;
			}

			// if not exist, and table is not full
			if(lru_page_table[i].page_num == -1){
				page_fault += 1;

				if(modification_flag == 1){
					set_most_recent(i, lru_page_table, page_num, 1);
				}else{
					set_most_recent(i, lru_page_table, page_num, 0);
				}

				flag = 1;
				break;
			}
		}

		// if page table is full and page number not in page table
		if(flag == 0){
			// the procedure of policy lru
			page_fault += 1;

			if(lru_page_table[phy_mem_page_num-1].modification == 1){
				lru_flush_count += 1;
			}

			if(modification_flag == 1){
				set_most_recent(phy_mem_page_num-1, lru_page_table, page_num, 1);
			}else{
				set_most_recent(phy_mem_page_num-1, lru_page_table, page_num, 0);
			}

		}

		modification_flag = 0;
		nref += 1;
	}
	end = clock();

	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("[a4vmsim] %d references processed using 'lru' in %.2f sec\n", nref, time_spent);
	printf("[a4vmsim] page faults = %d, write count= %d, flushes= %d\n", page_fault, lru_write_count, lru_flush_count);
	printf("[a4vmsim] Accumulator = %d\n", lru_accu_reg);
}