#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define MAX_INPUT_LINE 1000
#define MAX_LINES 49
#define OUTPUT_LINE_LEN 80

char buffer_1[MAX_INPUT_LINE * (MAX_LINES + 1)];
pthread_mutex_t buf_1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_1_full = PTHREAD_COND_INITIALIZER;
bool buf_1_closed = false;
int buf_1_count = 0;
int buf_1_prod_idx = 0;
int buf_1_cons_idx = 0;

char buffer_2[MAX_INPUT_LINE * (MAX_LINES + 1)];
pthread_mutex_t buf_2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_2_full = PTHREAD_COND_INITIALIZER;
bool buf_2_closed = false;
int buf_2_count = 0;
int buf_2_prod_idx = 0;
int buf_2_cons_idx = 0;

char buffer_3[MAX_INPUT_LINE * (MAX_LINES + 1)];
pthread_mutex_t buf_3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_3_full = PTHREAD_COND_INITIALIZER;
int buf_3_count = 0;
int buf_3_prod_idx = 0;
int buf_3_cons_idx = 0;

bool stop_processing = false;

void put_buf_1(char);
bool stop_token_found(char []);
void *get_input(void *);
char read_buf_1(void);
void put_buf_2(char);
void *separate_line(void *);
char read_buf_2(void);
void put_buf_3(char);
bool next_char_is_plus(void);
void *replace_double_plus(void *);
char read_buf_3(void);
void *write_output(void *);

int main(void)
{
    pthread_t input_t, line_separate_t, plusses_t, output_t;
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_separate_t, NULL, separate_line, NULL);
    pthread_create(&plusses_t, NULL, replace_double_plus, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);
    pthread_join(input_t, NULL);
    pthread_join(line_separate_t, NULL);
    pthread_join(plusses_t, NULL);
    pthread_join(output_t, NULL);
    return EXIT_SUCCESS;
}

void put_buf_1(char item)
{
  pthread_mutex_lock(&buf_1_mutex);
  buffer_1[buf_1_prod_idx++] = item;
  buf_1_count++;
  pthread_cond_signal(&buf_1_full);
  pthread_mutex_unlock(&buf_1_mutex);
}

bool stop_token_found(char line[])
{
    if (line[0] == 'S')
        if (line[1] == 'T')
            if (line[2] == 'O')
                if (line[3] == 'P')
                    if (line[4] == '\n')
                        return true;
    return false;
}

void *get_input(void *args)
{
    while(true) 
    {
        char input_buffer[MAX_INPUT_LINE];
        fgets(input_buffer, (MAX_INPUT_LINE * (MAX_LINES + 1)), stdin);
    
        for (int i = 0; i < strlen(input_buffer); i++)
        {            
            if (stop_token_found(input_buffer))
            {
                stop_processing = true;
                pthread_cond_signal(&buf_1_full);
                return NULL;
            }
            put_buf_1(input_buffer[i]);
        }
    }
    return NULL;
}

char read_buf_1(void)
{
    pthread_mutex_lock(&buf_1_mutex);
    while (buf_1_count == 0)
    {
        if (stop_processing)
            return 0;
        pthread_cond_wait(&buf_1_full, &buf_1_mutex);
    }
    char ch = buffer_1[buf_1_cons_idx++];
    buf_1_count--;
    pthread_mutex_unlock(&buf_1_mutex);
    return ch;
}

void put_buf_2(char ch)
{
    pthread_mutex_lock(&buf_2_mutex);
    buffer_2[buf_2_prod_idx++] = ch;
    buf_2_count++;
    pthread_cond_signal(&buf_2_full);
    pthread_mutex_unlock(&buf_2_mutex);
}

void *separate_line(void *args)
{
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * (MAX_LINES + 1); i++)
    {
        ch = read_buf_1();
        if (ch == '\n')
            put_buf_2(' ');
        else
            put_buf_2(ch);
    
        if (buf_1_count == 0 && stop_processing)
        {
            buf_1_closed = true;
            pthread_cond_signal(&buf_2_full);
            break;
        }
    }
    return NULL;
}

char read_buf_2(void)
{
    pthread_mutex_lock(&buf_2_mutex);
    while (buf_2_count == 0)
        pthread_cond_wait(&buf_2_full, &buf_2_mutex);
    char ch = buffer_2[buf_2_cons_idx++];
    buf_2_count--;
    pthread_mutex_unlock(&buf_2_mutex);
    return ch;
}

void put_buf_3(char ch)
{
    pthread_mutex_lock(&buf_3_mutex);
    buffer_3[buf_3_prod_idx++] = ch;
    buf_3_count++;
    pthread_cond_signal(&buf_3_full);
    pthread_mutex_unlock(&buf_3_mutex);
}

bool next_char_is_plus(void)
{
    pthread_mutex_lock(&buf_2_mutex);
    bool next_char_is_plus = buffer_2[buf_2_cons_idx] == '+' ? true : false;
    pthread_mutex_unlock(&buf_2_mutex);
    return next_char_is_plus;
}

void *replace_double_plus(void *args)
{
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * (MAX_LINES + 1); i++)
    {
        ch = read_buf_2();
        if (ch == '+')
        {
            if (next_char_is_plus())
            {
                read_buf_2();
                put_buf_3('^');
            }
            else
                put_buf_3(ch);
        }
        else
            put_buf_3(ch);
    
        if (buf_2_count == 0 && buf_1_closed)
        {
            buf_2_closed = true;
            pthread_cond_signal(&buf_3_full);
            break;
        }
    }
    return NULL;
}

char read_buf_3(void)
{
    pthread_mutex_lock(&buf_3_mutex);
    while (buf_3_count == 0)
        pthread_cond_wait(&buf_3_full, &buf_3_mutex);
    char ch = buffer_3[buf_3_cons_idx++];
    buf_3_count--;
    pthread_mutex_unlock(&buf_3_mutex);
    return ch;
}

void *write_output(void *args)
{
    char output_line[OUTPUT_LINE_LEN + 1];
    int line_count = 0;

    for (int i = 0; i < MAX_INPUT_LINE * (MAX_LINES + 1); i++)
    {
        if (i < OUTPUT_LINE_LEN)
        {
            output_line[i] = read_buf_3();
            if (i + 1 == OUTPUT_LINE_LEN)
            {
                line_count++;
                printf("%s\n", output_line);
            }
            if (buf_3_count == 0 && buf_2_closed)
                break;
        }
        else
        {
            output_line[i % OUTPUT_LINE_LEN] = read_buf_3();
            if ((i + 1) % OUTPUT_LINE_LEN == 0)
            {
                line_count++;
                printf("%s\n", output_line);
            }
            if (buf_3_count == 0 && buf_2_closed)
                break;
        }
    
        if (line_count >= (MAX_LINES + 1))
            break;
    }
    return NULL;
}