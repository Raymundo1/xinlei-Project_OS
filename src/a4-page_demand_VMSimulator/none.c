#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

int none_accu_reg = 0;
int none_write_count = 0;
int none_page_table[100000000];


void none_operation_in_refer(char* binary){
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

		none_accu_reg += result;
	}
	else if(strcmp(operation_code, "01") == 0){
		for(int j=5; j>=0; j--){
			if(value[j] == '1'){
				result += (1 << (5-j));
			}	
		}

		none_accu_reg -= result;
	}
	else if(strcmp(operation_code, "10") == 0){
		none_write_count += 1;
	}
}

int none_page_num(char* binary, int page_num_bit){
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

int none_IntergerToBinary(uint32_t address, int page_num_bit){
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

    none_operation_in_refer(binary);
    page_num = none_page_num(binary, page_num_bit);

    return page_num;
}

int binary_search(int* none_page_table, int l, int r, int x){
	if (r >= l){
        int mid = l + (r - l)/2;
        if (none_page_table[mid] == x)  return mid;
 
        if (none_page_table[mid] > x) return binary_search(none_page_table, l, mid-1, x);
 
        return binary_search(none_page_table, mid+1, r, x);
   }
 
   return -1;
}

void table_insert(int* none_page_table, int page_count, int page_num){
	int i;

	if(page_count == 0){
		none_page_table[0] = page_num;
	}
	else{
		
		for(i=page_count-1; (i>=0 && none_page_table[i] > page_num); i--){
			none_page_table[i+1] = none_page_table[i];
		}

		none_page_table[i+1] = page_num;
	}

}

void main_none(int page_len, int memsize){
	uint32_t address;
	clock_t begin;
	clock_t end;
	int nref = 0;
	int page_num_bit;
	int page_num;
	int flag;
	int page_fault = 0;

	// get the number of bits in page number
	page_num_bit = 32 - (log(page_len) / log(2));

	begin = clock();
	while(read(STDIN_FILENO, &address, sizeof(address)) > 0){
		page_num = none_IntergerToBinary(address, page_num_bit);
		
		flag = 0;
		// check if it exist in page table
		if(binary_search(none_page_table, 0, page_fault-1, page_num) >= 0){
			flag = 1;
		}

		// if not exist, and table is not full
		if(flag == 0){
			table_insert(none_page_table, page_fault, page_num);
			page_fault += 1;
		}

		nref += 1;
	}
	end = clock();

	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("[a4vmsim] %d references processed using 'none' in %.2f sec\n", nref, time_spent);
	printf("[a4vmsim] page faults = %d, write count= %d, flushes= 0\n", page_fault, none_write_count);
	printf("[a4vmsim] Accumulator = %d\n", none_accu_reg);

}