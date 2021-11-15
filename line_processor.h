#ifndef LINE_PROCESSOR
#define LINE_PROCESSOR

#define MAX_INPUT_LINE 1000
#define MAX_LINES 49
#define OUTPUT_LINE_LEN 80

void *get_input(void *);
void *separate_line(void *);
void *replace_double_plus(void *);
void *write_output(void *);
void put_buf_1(char);
char read_buf_1(void);
void put_buf_2(char);
char read_buf_2(void);
void put_buf_3(char);
char read_buf_3(void);
bool stop_token_found(char []);
bool next_char_is_plus(void);

#endif