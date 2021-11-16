/**
 * @file line_processor.c
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 4
 * 
 * This program creates four threads to process input from stdin using the Producer-Consumer model:
 *  Thread 1 (input thread): reads lines of characters from stdin
 *  Thread 2 (line separator thread): replaces every line separator with a space
 *  Thread 3 (plus sign thread): replaces every pair of plus signs with a caret
 *  Thread 4 (output thread): writes processed data to stdout as 80-character lines
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "line_processor.h"

bool stop_processing = false; // Whether the stop processing line has occurred

// Buffer 1 variables
// Input thread produces; line separator thread consumes
char buffer_1[MAX_INPUT_LINE * MAX_LINES];                  // Buffers are large enough for 50 lines of 1000 characters so buffers can be unbounded
pthread_mutex_t buf_1_mutex = PTHREAD_MUTEX_INITIALIZER;    // To indicate when a critical section of code needs exclusive access
pthread_cond_t buf_1_full = PTHREAD_COND_INITIALIZER;       // Signal to indicate to consumer that the buffer is empty
bool buf_1_closed = false;                                  // Whether buffer is closed or not
int buf_1_count = 0;                                        // Number of characters in buffer
int buf_1_prod_idx = 0;                                     // Location producer inserts a character to
int buf_1_cons_idx = 0;                                     // Location consumer reads a character from

// Buffer 2 variables
// Line separator thread produces; plus sign thread consumes
char buffer_2[MAX_INPUT_LINE * MAX_LINES];
pthread_mutex_t buf_2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_2_full = PTHREAD_COND_INITIALIZER;
bool buf_2_closed = false;
int buf_2_count = 0;
int buf_2_prod_idx = 0;
int buf_2_cons_idx = 0;

// Buffer 3 variables
// Plus sign thread produces; output thread consumes
char buffer_3[MAX_INPUT_LINE * MAX_LINES];
pthread_mutex_t buf_3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_3_full = PTHREAD_COND_INITIALIZER;
int buf_3_count = 0;
int buf_3_prod_idx = 0;
int buf_3_cons_idx = 0;

int main(void)
{
    pthread_t input_t;          // Input thread
    pthread_t line_separate_t;  // Line separator thread
    pthread_t plus_replace_t;   // Plus sign thread
    pthread_t output_t;         // Output thread

    // Create threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_separate_t, NULL, separate_line, NULL);
    pthread_create(&plus_replace_t, NULL, replace_double_plus, NULL);
    pthread_create(&output_t, NULL, write_output, NULL);

    // Close threads
    pthread_join(input_t, NULL);
    pthread_join(line_separate_t, NULL);
    pthread_join(plus_replace_t, NULL);
    pthread_join(output_t, NULL);

    return EXIT_SUCCESS;
}

void *get_input(void *args)
{
    for (int i = 0; i < MAX_LINES; i++)                 // At most, 50 lines will be read
    {
        char input_line[MAX_INPUT_LINE];                // Allocate enough space for a 1,000 character line
        fgets(input_line, MAX_INPUT_LINE, stdin);       // Read a line from stdin
    
        if (is_stop_line(input_line))                   // Check if the line is the stop processing line
            return stop_input();                        // Stop processing line found, so stop reading input

        for (int i = 0; i < strlen(input_line); i++)    // Walk through each character in the input line
            put_buf_1(input_line[i]);                   // Write the character to buffer one
    }
    return NULL;
}


void *separate_line(void *args)
{
    // Process at most 50 lines of 1,000 characters
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_1();          // Read the next character from buffer 1
        if (ch == 0)                // ch is 0 if buffer 1 is empty and the stop processing line has been given
            break;                  // Stop processing characters
        else if (ch == NEWLINE)     // Check if the character is the line separator
            put_buf_2(SPACE);       // Write a space to buffer 2 instead of the line separator
        else
            put_buf_2(ch);          // Write the character to buffer 2
    }
    return NULL;
}

void *replace_double_plus(void *args)
{
    // Process at most 50 lines of 1,000 characters
    char ch;
    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_2();                                  // Read the next character from buffer 2
        if (ch == 0)                                        // ch is 0 if buffer 2 is empty and buffer 1 is closed
            break;                                          // Stop processing characters
        else if (ch == PLUS_SIGN && next_char_is_plus())    // Check if the current character and the next character are both plus signs
        {
            read_buf_2();                                   // Skip the next plus sign in buffer 2
            put_buf_3(CARET);                               // Write a caret to buffer 3 instead of the plus signs
        }
        else
            put_buf_3(ch);                                  // Write the character to buffer 3
    }
    return NULL;
}

void *write_output(void *args)
{
    // Allocate enough space for 80 characters + a line separator
    char output_line[OUTPUT_LINE_LEN + 1];
    char ch;

    // Process at most 50 lines of 1,000 characters
    for (int i = 0; i < MAX_INPUT_LINE * MAX_LINES; i++)
    {
        ch = read_buf_3();                          // Read the next character from buffer 3
        if (ch == 0)                                // ch is 0 if buffer 3 is empty and buffer 2 is closed
            break;                                  // Stop processing characters
        else
        {                                           
            output_line[i % OUTPUT_LINE_LEN] = ch;  // Copy the character to the output line
            if ((i + 1) % OUTPUT_LINE_LEN == 0)     // Check if there are enough characters for an output line
                printf("%s\n", output_line);        // Write the output line to stdout
        }
    }
    return NULL;
}

void put_buf_1(char ch)
{
  pthread_mutex_lock(&buf_1_mutex);     // Lock the mutex to ensure exclusive access
  buffer_1[buf_1_prod_idx++] = ch;      // Write the character to buffer one and increment the producer index
  buf_1_count++;                        // Increment the number of characters in buffer one
  pthread_cond_signal(&buf_1_full);     // Signal thread two that buffer one is not empty
  pthread_mutex_unlock(&buf_1_mutex);   // Unlock the mutex
}

char read_buf_1(void)
{
    pthread_mutex_lock(&buf_1_mutex);                   // Lock the mutex to ensure exclusive access
    while (buf_1_count == 0)                            // Check whether buffer one is empty
    {
        if (stop_processing)                            // Check if the stop processing line has been given
        {
            buf_1_closed = true;                        // Close buffer one
            pthread_cond_signal(&buf_2_full);           // Signal thread three
            return 0;                                   // Tell thread two to shut down
        }
        pthread_cond_wait(&buf_1_full, &buf_1_mutex);   // Wait for the buffer one full signal to read a character
    }
    char ch = buffer_1[buf_1_cons_idx++];               // Read the next character from buffer one
    buf_1_count--;                                      // Decrement the number of characters in buffer one
    pthread_mutex_unlock(&buf_1_mutex);                 // Unlock the mutex
    return ch;                                          // Return the character read from buffer one
}

void put_buf_2(char ch)
{
    pthread_mutex_lock(&buf_2_mutex);   // Lock the mutex to ensure exclusive access
    buffer_2[buf_2_prod_idx++] = ch;    // Write the character to buffer two and increment the producer index
    buf_2_count++;                      // Increment the number of characters in buffer two
    pthread_cond_signal(&buf_2_full);   // Signal thread three that buffer two is not empty
    pthread_mutex_unlock(&buf_2_mutex); // Unlock the mutex
}


char read_buf_2(void)
{
    pthread_mutex_lock(&buf_2_mutex);                   // Lock the mutex to ensure exclusive access
    while (buf_2_count == 0)                            // Check whether buffer two is empty
    {
        if (buf_1_closed)                               // Check whether buffer one is closed
        {
            buf_2_closed = true;                        // Close buffer two
            pthread_cond_signal(&buf_3_full);           // Signal buffer three
            return 0;                                   // Tell buffer three to shut down
        }
        pthread_cond_wait(&buf_2_full, &buf_2_mutex);   // Wait for the buffer two full signal to read a character
    }
    char ch = buffer_2[buf_2_cons_idx++];               // Read the next character from buffer two
    buf_2_count--;                                      // Decrement the number of characterfs in buffer two
    pthread_mutex_unlock(&buf_2_mutex);                 // Unlock the mutex
    return ch;                                          // Return the character read from buffer two
}

void put_buf_3(char ch)
{
    pthread_mutex_lock(&buf_3_mutex);    // Lock the mutex to ensure exclusive access
    buffer_3[buf_3_prod_idx++] = ch;     // Write the character to buffer three and increment the producer index
    buf_3_count++;                       // Increment the number of characters in buffer three
    pthread_cond_signal(&buf_3_full);    // Signal thread four that buffer three is not empty
    pthread_mutex_unlock(&buf_3_mutex);  // Unlock the mutex
}

char read_buf_3(void)
{
    pthread_mutex_lock(&buf_3_mutex);                   // Lock the mutex to ensure exclusive access
    while (buf_3_count == 0)                            // Check whether buffer three is empty
    {
        if (buf_2_closed)                               // Check whether buffer two is closed
            return 0;                                   // Tell thread four to shut down
        pthread_cond_wait(&buf_3_full, &buf_3_mutex);   // Wait for the buffer three full signal to read a character
    } 
    char ch = buffer_3[buf_3_cons_idx++];               // Read the next character from buffer three
    buf_3_count--;                                      // Decrement the number of characters in buffer three
    pthread_mutex_unlock(&buf_3_mutex);                 // Unlock the mutex
    return ch;                                          // Return the character read from buffer three
}

void *stop_input(void)
{
    pthread_mutex_lock(&buf_1_mutex);    // Lock the buffer one mutex
    stop_processing = true;              // Indicate that the stop processing line has been given
    pthread_cond_signal(&buf_1_full);    // Signal thread two
    pthread_mutex_unlock(&buf_1_mutex);  // Unlock the mutex
    return NULL;                         // Tell get_input to shut down
}

bool is_stop_line(char input_line[])
{
    char stop_line[6] = "STOP\n"; // The stop line

    // If the input line doesn't start with 'S' or isn't six characters long, it's not a stop line
    if (input_line[0] != 'S' || strlen(input_line) < strlen(stop_line))
        return false;

    for (int i = 0; i < strlen(stop_line); i++)     // Compare every character in the input line to every character in the stop line
        if (input_line[i] != stop_line[i])          // If any are different
            return false;                           // The input line is not the stop line

    return true;                                    // If we get here, the input line is the stop line
}

bool next_char_is_plus(void)
{
    pthread_mutex_lock(&buf_2_mutex);                                                   // Lock the buffer two mutex
    bool next_char_is_plus = buffer_2[buf_2_cons_idx] == PLUS_SIGN ? true : false;      // Check whether the next character is a plus sign
    pthread_mutex_unlock(&buf_2_mutex);                                                 // Unlock the mutex
    return next_char_is_plus;                                                           // Return whether the next character is a plus sign or not
}

