#ifndef _sec_h_
#define _sec_h_

int sec_accu_reg;
int sec_write_count;
void sec_operation_in_refer(char* binary, int* modification_flag);
int sec_page_num(char* binary, int page_num_bit);
int sec_IntergerToBinary(uint32_t address, int page_num_bit, int* modification_flag);
void main_sec(int page_len, int memsize);

#endif