/**
 * @file line_processor.h
 * @author Cody Ray <rayc2@oregonstate.edu>
 * @version 1.0
 * @section DESCRIPTION
 *
 * For OSU CS 344
 * Assignment 4
 * 
 * Macros and function declarations for line_processor.c
 */

#ifndef LINE_PROCESSOR
#define LINE_PROCESSOR

// Input/output sizes
#define MAX_INPUT_LINE 1000 // Maximum length of an input line (including newline)
#define MAX_LINES 50        // Maximum number of input lines (including STOP line)
#define OUTPUT_LINE_LEN 80  // Fixed length for all output lines

// Important characters
#define NEWLINE '\n'        // Line separator
#define SPACE ' '           // Any instance of a line separator is replaces with a space
#define CARET '^'           // Caret character
#define PLUS_SIGN '+'       // Any instance of two plus signs is replaced with a caret

/**
 * Thread one. Reads lines of characters from stdin, checks for the stop 
 * processing line, and writes characters to buffer one.
 * 
 * @param  args required but not used
 * @return required but not used
 */
void *get_input(void *);

/**
 * Thread two. Reads characters from buffer one, replaces all newlines
 * with spaces, and writes characters to buffer two.
 * 
 * @param  args required but not used
 * @return required but not used
 */
void *separate_line(void *);

/**
 * Thread three. Reads characters from buffer two, replaces all instances
 * of two plusses ('++') with a caret ('^'), and writes characters to
 * buffer three.
 * 
 * @param  args required but not used
 * @return required but not used
 */
void *replace_double_plus(void *);

/**
 * Thread four. Reads characters from buffer three and writes eighty character
 * lines to stdout when and if there are eighty characters available to write.
 * 
 * @param  args required but not used
 * @return required but not used
 */
void *write_output(void *);

/**
 * Locks the buffer one mutex, writes the given character to buffer one, and
 * signals thread two that buffer one is not empty.
 * 
 * @param  ch a character to write to buffer one
 */
void put_buf_1(char);

/**
 * Locks the buffer one mutex and tries to read the next character from buffer
 * one. If buffer one is empty, waits for the buffer one full signal. If
 * buffer one is empty and the stop processing line has been given, returns 0
 * to indicate that thread two should shut down. Otherwise returns the next 
 * character from buffer one.
 * 
 * @return the next character from buffer one. Returns 0 if buffer one is empty
 *         and the stop processing line has been given.
 */
char read_buf_1(void);

/**
 * Locks the buffer two mutex, writes the given character to buffer two, and
 * signals thread three that buffer two is not empty.
 * 
 * @param  ch a character to write to buffer two
 */
void put_buf_2(char);

/**
 * Locks the buffer two mutex and tries to read the next character from buffer
 * two. If buffer two is empty, waits for the buffer two full signal. If
 * buffer two is empty and buffer one is closed, returns 0 to indicate that
 * thread two should shut down. Otherwise returns the next character
 * from buffer two.
 * 
 * @return the next character from buffer two. Returns 0 if buffer two is empty
 *         and buffer one is closed.
 */
char read_buf_2(void);

/**
 * Locks the buffer three mutex, writes the given character to buffer three, and
 * signals thread four that buffer three is not empty.
 * 
 * @param  ch a character to write to buffer three
 */
void put_buf_3(char);

/**
 * Locks the buffer three mutex and tries to read the next character from buffer
 * three. If buffer three is empty, waits for the buffer three full signal. If
 * buffer three is empty and buffer two is closed, returns 0 to indicate that
 * thread three should shut down. Otherwise returns the next character
 * from buffer three.
 * 
 * @return the next character from buffer three. Returns 0 if buffer three is empty
 *         and buffer two is closed.
 */
char read_buf_3(void);

/**
 * Locks the buffer one mutex, switches the stop_processing variable to true, and
 * signal thread two to take over.
 * 
 * @return NULL to tell get_input to stop processing input
 */
void *stop_input(void);

/**
 * Checks if an input line is the stop processing line ('STOP\n')
 * 
 * @param  input_line an input line represented as a character array
 * @return true if the input line is the stop processing line, otherwise false
 */
bool is_stop_line(char []);

/**
 * Locks the buffer two mutex and peeks at the next character in buffer two to see if
 * it is a plus sign.
 * 
 * @return true if the next character in buffer two is a plus sign, otherwise false
 */
bool next_char_is_plus(void);

#endif