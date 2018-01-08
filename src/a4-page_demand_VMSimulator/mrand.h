#ifndef _mrand_h_
#define _mrand_h_


int mrand_accu_reg;
int mrand_write_count;
void mrand_operation_in_refer(char* binary, int* modification_flag);
int mrand_page_num(char* binary, int page_num_bit);
int mrand_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag);
int check_visit_table(int rand_position_page_num, int* last_visit_table);
void main_mrand(int page_len, int memsize);

#endif