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

// the below is NOT a class
typedef struct posix_terminal_size_t posix_terminal_size_t;
typedef struct posix_terminal_size_t {
    unsigned int rows;
    unsigned int cols;
    // TODO - add info on scrollback if available?
} posix_terminal_size_t;

typedef struct posix_terminal_cur_pos_t posix_terminal_cur_pos_t;
typedef struct posix_terminal_cur_pos_t {
    unsigned int cur_row;
    unsigned int cur_col;
} posix_terminal_cur_pos_t;

// the below IS a class, but not meant for use outside of fancyrl
typedef struct posix_terminal_class_t posix_terminal_class_t;
typedef struct posix_terminal_class_t {
    base_class_t Parent;

    posix_terminal_size_t cur_size;        // currently known terminal size, don't access directly
    posix_terminal_cur_pos_t cur_curpos;   // currently known cursor position, don't access directly
    posix_terminal_cur_pos_t saved_curpos; // saved cursor position - this is SCREEN position, not relative to input string (cos this class doesn't know about input string)

    struct termios orig_termios; // original terminal settings
    struct termios cur_termios;  // current terminal settings (used by fancyrl)

    posix_terminal_size_t    (*get_term_size)();

    posix_terminal_cur_pos_t (*get_cur_pos)();
    void                     (*set_cur_pos)(posix_terminal_cur_pos_t new_pos);
    void                     (*save_cur_pos)();
    void                     (*restore_cur_pos)();

    void (*clear_line)();       // clears the current line - this will also move the cursor to column 0
    void (*clear_terminal)();   // clears the terminal - this will also move the cursor to 0,0

    void (*write_char)(char c);           // writes a character at the current cursor position
    void (*write_str)(string_class_t* s); // writes a string - this is done without first converting to a C string, so should be a lot faster
    void (*write_cstr)(char *s);          // writes a C string (a char*)

    void (*setup_term)();   // this configures the terminal for use with fancyrl
    void (*restore_term)(); // restores original terminal settings
    

} posix_terminal_class_t;

posix_terminal_class_t posix_terminal_class_base;
