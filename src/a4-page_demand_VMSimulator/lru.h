#ifndef _lru_h_
#define _lru_h_

struct Page{
	int page_num;
	int modification;
};

int lru_accu_reg;
int lru_write_count;
void lru_operation_in_refer(char* binary, int* modification_flag);
int lru_page_num(char* binary, int page_num_bit);
int lru_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag);
void set_most_recent(int index, int* lru_page_table, int page_num, int modification);
void main_lru(int page_len, int memsize);

#endif