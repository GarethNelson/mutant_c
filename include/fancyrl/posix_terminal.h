//-----------------------------------------------------------------------------
//
// Copyright (C) 2017 by Gareth Nelson (gareth@garethnelson.com)
//
// This file is part of MutantC.
//
// MutantC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// MutantC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MutantC.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#pragma once

#include <mutant/base_class.h>
#include <mutant/string_class.h>

#include <termios.h>
#include <signal.h>

// the below is NOT a class
typedef struct posix_terminal_size_t {
    unsigned int rows;
    unsigned int cols;
    // TODO - add info on scrollback if available?
} posix_terminal_size_t;

typedef struct posix_terminal_cur_pos_t {
    unsigned int cur_row;
    unsigned int cur_col;
} posix_terminal_cur_pos_t;

// TODO: add colour support and stuff
typedef enum {
    MUTANT_ATTR_A_NORMAL,
    MUTANT_ATTR_A_DIM,
    MUTANT_ATTR_A_BOLD,
    MUTANT_ATTR_A_UNDERLINE,
} posix_terminal_attr_t;

// represents logical keys
typedef enum {
     LOGICAL_KEY_UNKNOWN   = -2, // unknown logical key - should be ignored
     LOGICAL_KEY_INVALID   = -1, // invalid logical key - used for signalling errors
     LOGICAL_KEY_ASCII     = 0,  // just ASCII input - this obviously requires knowing the raw character too
     LOGICAL_KEY_TAB       = 1,  // autocomplete
     LOGICAL_KEY_ENTER     = 2,  // go onto a newline (duh), finish input if parsed successfully
     LOGICAL_KEY_DEL       = 3,  // delete character after cursor (or NOP if string is empty)
     LOGICAL_KEY_BACKSPACE = 4,  // delete character before cursor
     LOGICAL_KEY_UP        = 5,  // go up a line
     LOGICAL_KEY_DOWN      = 6,  // go down a line
     LOGICAL_KEY_LEFT      = 7,  // move cursor left, go to previous line if it exists and we're at the first column
     LOGICAL_KEY_RIGHT     = 8,  // move cursor right
     LOGICAL_KEY_HOME      = 9,  // go to start of expression
     LOGICAL_KEY_END       = 10, // go to end of expression
     LOGICAL_KEY_CTRL_L    = 11, // clear screen (this will NOT clear the current expression if one is being edited)
     LOGICAL_KEY_CTRL_C    = 12, // ctrl-c - we need our own special handler for this in raw mode
} posix_logical_key_t;

typedef struct posix_keyinput_t {
    char                raw_char;
    posix_logical_key_t logical_key;
} posix_keyinput_t;

// the below IS a class, but not meant for use outside of fancyrl
typedef struct posix_terminal_class_t posix_terminal_class_t;
typedef struct posix_terminal_class_t {
    base_class_t Parent;

    posix_terminal_size_t cur_size;        // currently known terminal size, don't access directly
    posix_terminal_cur_pos_t cur_curpos;   // currently known cursor position, don't access directly
    posix_terminal_cur_pos_t saved_curpos; // saved cursor position - this is SCREEN position, not relative to input string (cos this class doesn't know about input string)

    struct termios orig_stdin_termios; // original terminal settings
    struct termios orig_stdout_termios;

    struct termios cur_stdin_termios;         // current terminal settings (used by fancyrl)
    struct termios cur_stdout_termios;

    speed_t output_speed; // terminal output speed - this IS important, believe it or not

    void (*orig_sig_handler)(int signum); // original signal handler (so we can restore it in restore_term())

    void                     (*sig_handle)(int signum); // not meant to actually be called directly, signal handler for window resize etc
    posix_terminal_size_t    (*get_term_size)();

    posix_terminal_cur_pos_t (*get_cur_pos)();
    void                     (*set_cur_pos)(posix_terminal_cur_pos_t new_pos);
    void                     (*save_cur_pos)();
    void                     (*restore_cur_pos)();

    void (*clear_line)();       // clears the current line - this will also move the cursor to column 0
    void (*clear_terminal)();   // clears the terminal - this will also move the cursor to 0,0

    // it is up to the client of the library to check string lengths before writing them
    void (*write_char)(char c);            // writes a character at the current cursor position
    void (*write_str) (string_class_t* s); // writes a string - this is done without first converting to a C string, so should be a lot faster
    void (*write_cstr)(char *s);           // writes a C string (a char*)

    void (*write_char_attr)(char c, posix_terminal_attr_t attr);            // write a single character with the selected attribute - this will NOT set the attribute as on for further writes
    void (*write_str_attr) (string_class_t* s, posix_terminal_attr_t attr); // write a string with the selected attribute
    void (*write_cstr_attr)(char* s, posix_terminal_attr_t attr);           // write a C string with the selected attribute

    posix_keyinput_t (*read_key)(); // reads a key (duh) - this will block if no input is available

    void (*setup_term)();   // this configures the terminal for use with fancyrl
    void (*restore_term)(); // restores original terminal settings
    

} posix_terminal_class_t;

posix_terminal_class_t posix_terminal_class_base;
