#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "line_processor.h"

bool stop_processing = false;

char buffer_1[MAX_INPUT_LINE * MAX_LINES];
pthread_mutex_t buf_1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_1_full = PTHREAD_COND_INITIALIZER;
bool buf_1_closed = false;
int buf_1_count = 0;
int buf_1_prod_idx = 0;
int buf_1_cons_idx = 0;

char buffer_2[MAX_INPUT_LINE * MAX_LINES];
pthread_mutex_t buf_2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_2_full = PTHREAD_COND_INITIALIZER;
bool buf_2_closed = false;
int buf_2_count = 0;
int buf_2_prod_idx = 0;
int buf_2_cons_idx = 0;

char buffer_3[MAX_INPUT_LINE * MAX_LINES];
pthread_mutex_t buf_3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_3_full = PTHREAD_COND_INITIALIZER;
int buf_3_count = 0;
int buf_3_prod_idx = 0;
int buf_3_cons_idx = 0;

int main(void)
{
    pthread_t input_t, line_separate_t, plus_replace_t, output_t;

    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_separate_t, NULL, separate_line, NULL);
    pthread_create(&plus_replace_t, NULL, replace_double_plus, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);

    pthread_join(input_t, NULL);
    pthread_join(line_separate_t, NULL);
    pthread_join(plus_replace_t, NULL);
    pthread_join(output_t, NULL);

    return EXIT_SUCCESS;
}

void *get_input(void *args)
{
    for (int i = 0; i < MAX_LINES; i++)
    {
        char input_line[MAX_INPUT_LINE];
        fgets(input_line, MAX_INPUT_LINE, stdin);
    
        for (int i = 0; i < strlen(input_line); i++)
        {            
            if (is_stop_line(input_line))
            {
                stop_processing = true;
                pthread_cond_signal(&buf_1_full);
                return NULL;
            }
            put_buf_1(input_line[i]);
        }
    }
    return NULL;
}


void *separate_line(void *args)
{
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_1();

        if (ch == 0)
            break;
        else if (ch == NEWLINE)
            put_buf_2(SPACE);
        else
            put_buf_2(ch);
    }
    return NULL;
}

void *replace_double_plus(void *args)
{
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_2();

        if (ch == 0)
            break;
        else if (ch == PLUS_SIGN && next_char_is_plus())
        {
            read_buf_2();
            put_buf_3(CARET);
        }
        else
            put_buf_3(ch);
    }
    return NULL;
}

void *write_output(void *args)
{
    char output_line[OUTPUT_LINE_LEN + 1];
    char ch;

    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_3();

        if (ch == 0)
            break;
        else if (i < OUTPUT_LINE_LEN)
        {
            output_line[i] = ch;
            if (i == OUTPUT_LINE_LEN - 1)
                printf("%s\n", output_line);
        }
        else
        {
            output_line[i % OUTPUT_LINE_LEN] = ch;
            if ((i + 1) % OUTPUT_LINE_LEN == 0)
                printf("%s\n", output_line);
        }
    }
    return NULL;
}

void put_buf_1(char item)
{
  pthread_mutex_lock(&buf_1_mutex);
  buffer_1[buf_1_prod_idx++] = item;
  buf_1_count++;
  pthread_cond_signal(&buf_1_full);
  pthread_mutex_unlock(&buf_1_mutex);
}

char read_buf_1(void)
{
    pthread_mutex_lock(&buf_1_mutex);
    while (buf_1_count == 0)
    {
        if (stop_processing)
        {
            buf_1_closed = true;
            pthread_cond_signal(&buf_2_full);
            return 0;
        }
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


char read_buf_2(void)
{
    pthread_mutex_lock(&buf_2_mutex);
    while (buf_2_count == 0)
    {
        if (buf_1_closed)
        {
            buf_2_closed = true;
            pthread_cond_signal(&buf_3_full);
            return 0;
        }
        pthread_cond_wait(&buf_2_full, &buf_2_mutex);
    }
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

char read_buf_3(void)
{
    pthread_mutex_lock(&buf_3_mutex);
    while (buf_3_count == 0)
    {
        if (buf_2_closed)
            return 0;
        pthread_cond_wait(&buf_3_full, &buf_3_mutex);
    } 
    char ch = buffer_3[buf_3_cons_idx++];
    buf_3_count--;
    pthread_mutex_unlock(&buf_3_mutex);
    return ch;
}

bool is_stop_line(char input_line[])
{
    char stop_line[6] = "STOP\n";
    if (strlen(input_line) < strlen(stop_line))
        return false;
    for (int i = 0; i < strlen(stop_line); i++)
        if (input_line[i] != stop_line[i])
            return false;
    return true;
}

bool next_char_is_plus(void)
{
    pthread_mutex_lock(&buf_2_mutex);
    bool next_char_is_plus = buffer_2[buf_2_cons_idx] == PLUS_SIGN ? true : false;
    pthread_mutex_unlock(&buf_2_mutex);
    return next_char_is_plus;
}

