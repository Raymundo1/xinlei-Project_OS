#ifndef _none_h_
#define _none_h_

int none_accu_reg;
int none_write_count;
// int none_flush_count;
int none_page_table[100000000];
void none_operation_in_refer(char* binary);
int none_page_num(char* binary, int page_num_bit);
int binary_search(int* none_page_table, int l, int r, int x);
void table_insert(int* none_page_table, int page_count, int page_num);
void main_none(int page_len, int memsize);

#endif