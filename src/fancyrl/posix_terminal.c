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

#include <fancyrl/posix_terminal.h>
#include <mutant/base_class.h>
#include <mutant/string_class.h>
#include <mutant/curry_func.h>
#include <mutant/gc_allocator_class.h>

#include <termkey.h>

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <termios.h>
#include <termcap.h>

void posix_terminal_class_init   (posix_terminal_class_t* this, gc_allocator_class_t* allocator);
void posix_terminal_class_destroy(posix_terminal_class_t* this, gc_allocator_class_t* allocator);

void posix_terminal_class_setup_term  (posix_terminal_class_t* this);
void posix_terminal_class_restore_term(posix_terminal_class_t* this);

void posix_terminal_class_sig_handle(posix_terminal_class_t* this, int signum);

posix_keyinput_t posix_terminal_class_read_key(posix_terminal_class_t* this);

void                  posix_terminal_class_write_char   (posix_terminal_class_t* this, char c);
void                  posix_terminal_class_write_cstr   (posix_terminal_class_t* this, char* s);

posix_terminal_size_t posix_terminal_class_get_term_size(posix_terminal_class_t* this);

posix_terminal_class_t posix_terminal_class_base = {
    .Parent = {
        .instance_size = sizeof(posix_terminal_class_t),
        .init          = (ClassInit)posix_terminal_class_init,
    },

};

void posix_terminal_class_init(posix_terminal_class_t* this, gc_allocator_class_t* allocator) {
     this->Parent._allocator = allocator;

     this->Parent.destroy = curry_func(posix_terminal_class_destroy, this);

     this->setup_term    = curry_func(posix_terminal_class_setup_term,    this);
     this->restore_term  = curry_func(posix_terminal_class_restore_term,  this);
     this->sig_handle    = curry_func(posix_terminal_class_sig_handle,    this);
     this->read_key      = curry_func(posix_terminal_class_read_key,      this);
     this->write_char    = curry_func(posix_terminal_class_write_char,    this);
     this->write_cstr    = curry_func(posix_terminal_class_write_cstr,    this);
     this->get_term_size = curry_func(posix_terminal_class_get_term_size, this);
}

// WARNING: this does NOT restore the terminal back to original settings, it's only a memory deallocation thing
void posix_terminal_class_destroy(posix_terminal_class_t* this, gc_allocator_class_t* allocator) {
     free_curry(this->setup_term);
     this->setup_term = NULL;

     free_curry(this->restore_term);
     this->restore_term = NULL;

     free_curry(this->sig_handle);
     this->sig_handle = NULL;

     free_curry(this->get_term_size);
     this->get_term_size = NULL;

     free_curry(this->read_key);
     this->read_key = NULL;

     free_curry(this->write_char);
     this->write_char = NULL;

     free_curry(this->write_cstr);
     this->write_cstr = NULL;

     termkey_destroy(this->tk);
}

void posix_terminal_class_setup_term(posix_terminal_class_t* this) {
     printf("\n"); // assuming normal terminal settings, this will get us to a newline so we can assume we're at column 0

     // we setup termcap
     char *termtype = getenv("TERM");
     char buf[2048];
     tgetent(buf,termtype);

     this->output_speed = cfgetospeed(&(this->cur_stdout_termios)); // we store this in an instance member in case of future weirdness

     // the below is needed by termcap's tputs and uses some magic that may not be portable outside of GNU
     char* term_pc = NULL;
     term_pc = tgetstr("pc",&term_pc);
     if(term_pc == NULL) {
        PC = 0;
     } else {
        PC = term_pc[0];
     }
     

     // configure signal handler for terminal size (and get the current terminal size)
     this->cur_size = this->get_term_size();
     this->orig_sig_handler = signal(SIGWINCH,this->sig_handle); // TODO: implement locking (so we don't read in the middle of signal handler processing)

     // turn off buffering for stdout
     setbuf(stdout, NULL); // alas, this can not be restored by restore_term, but that should not have any serious effects

     // save current settings for stdout and then copy them
     tcgetattr(STDOUT_FILENO,&(this->orig_stdout_termios));
     this->cur_stdout_termios = this->orig_stdout_termios;

     // if we wanted to modify stdout settings, we'd do so here and then switch to them with tcsetattr

     // save current settings for stdin and then copy them
     tcgetattr(STDIN_FILENO,&(this->orig_stdin_termios));
     this->cur_stdin_termios = this->orig_stdin_termios;

     // configure raw mode for stdin and switch to the new settings
     cfmakeraw(&(this->cur_stdin_termios));
     this->cur_stdin_termios.c_cc[VMIN]  = 1; // we read a byte at a time
     this->cur_stdin_termios.c_cc[VTIME] = 0; // we read in blocking mode
     tcsetattr(STDIN_FILENO,TCSAFLUSH,&(this->cur_stdin_termios));

     // start termkey
     this->tk = termkey_new_abstract(termtype,TERMKEY_FLAG_NOTERMIOS|TERMKEY_FLAG_CTRLC);


}

posix_terminal_size_t posix_terminal_class_get_term_size(posix_terminal_class_t* this) {
     posix_terminal_size_t retval = {.rows = tgetnum("li"), .cols = tgetnum("co")};
     return retval;

}

void posix_terminal_class_restore_term(posix_terminal_class_t* this) {
     signal(SIGWINCH,this->orig_sig_handler); // restore original signal handler

     // restore original terminal settings
     tcsetattr(STDIN_FILENO,TCSAFLUSH,&(this->orig_stdin_termios));
     tcsetattr(STDOUT_FILENO,TCSANOW,&(this->orig_stdout_termios));

     termkey_stop(this->tk);
     printf("\n");
}

void posix_terminal_class_sig_handle(posix_terminal_class_t* this, int signum) {
     if(this->orig_sig_handler != NULL) this->orig_sig_handler(signum);
     this->cur_size = this->get_term_size();
}

void posix_terminal_class_write_char(posix_terminal_class_t* this, char c) {
     // this function does NOT just write the character blindly, it first translates it if appropriate

     // TODO: track cursor movements here

     switch(c) {
         case '\n':
              write(STDOUT_FILENO,&c,1); // TODO - do this properly via termcap

         break;
         case '\r':
              write(STDOUT_FILENO,&c,1);
         break;
         case 32 ... 126:
              write(STDOUT_FILENO,&c,1); // basic ASCII, just dump it out
         break;
     }
}

typedef int (*tputs_car_callback)(int);
void posix_terminal_class_write_cstr(posix_terminal_class_t* this, char* s) {
     ospeed = this->output_speed;
     // TODO - calculate nlines in a saner way - currently this is calculated as just the length of the string, which is both inefficient and probably inaccurate
     // of course, by using strlen(s) we ensure that nlines is never too low, but it'll usually be too high
     // see https://www.gnu.org/software/termutils/manual/termcap-1.3/html_mono/termcap.html#SEC11
     tputs(s,strlen(s),(tputs_car_callback)this->write_char);
}

posix_logical_key_t termkey_sym_to_logicalkey(TermKeySym s) {
     switch(s) {
           case TERMKEY_SYM_UNKNOWN:
                return LOGICAL_KEY_UNKNOWN;
           break;
           case TERMKEY_SYM_TAB:
                return LOGICAL_KEY_TAB;
           break;
           case TERMKEY_SYM_ENTER:
                return LOGICAL_KEY_ENTER;
           break;
           case TERMKEY_SYM_DEL:
                return LOGICAL_KEY_DEL;
           break;
           case TERMKEY_SYM_BACKSPACE:
                return LOGICAL_KEY_BACKSPACE;
           break;
           case TERMKEY_SYM_UP:
                return LOGICAL_KEY_UP;
           break;
           case TERMKEY_SYM_DOWN:
                return LOGICAL_KEY_DOWN;
           break;
           case TERMKEY_SYM_LEFT:
                return LOGICAL_KEY_LEFT;
           break;
           case TERMKEY_SYM_RIGHT:
                return LOGICAL_KEY_RIGHT;
           break;
           case TERMKEY_SYM_HOME:
                return LOGICAL_KEY_HOME;
           break;
           case TERMKEY_SYM_END:
                return LOGICAL_KEY_END;
           break;
     }
     return LOGICAL_KEY_UNKNOWN;
}

posix_keyinput_t posix_terminal_class_read_key(posix_terminal_class_t* this) {
     char raw_c = 0;
     read(STDIN_FILENO, &raw_c, 1);
     posix_keyinput_t retval;
     retval.raw_char = raw_c;

     if( (raw_c >= 32) && (raw_c <= 126)) {
         retval.logical_key = LOGICAL_KEY_ASCII;
         return retval;
     }

     char buf[100];

     termkey_push_bytes(this->tk,(const char *)&raw_c, 1); 
     TermKeyResult tk_result;
     TermKeyKey    tk_key;
     tk_result = termkey_getkey(this->tk,&tk_key);

     retval.logical_key = LOGICAL_KEY_UNKNOWN;

     while(tk_result==TERMKEY_RES_AGAIN) {
        read(STDIN_FILENO, &raw_c, 1);
        termkey_push_bytes(this->tk,(const char*)&raw_c, 1);
        tk_result = termkey_getkey(this->tk,&tk_key);
     }
     switch(tk_key.type) {
         case TERMKEY_TYPE_UNICODE:
              if((tk_key.code.codepoint == 'l') && (tk_key.modifiers & TERMKEY_KEYMOD_CTRL)) {
                 retval.logical_key = LOGICAL_KEY_CTRL_L;
              }
              if((tk_key.code.codepoint == 'c') && (tk_key.modifiers & TERMKEY_KEYMOD_CTRL)) {
                 retval.logical_key = LOGICAL_KEY_CTRL_C;
              }              
         break;
         case TERMKEY_TYPE_FUNCTION:
              fprintf(stderr,"FUNCTION");
         break;
         case TERMKEY_TYPE_KEYSYM:
              retval.logical_key = termkey_sym_to_logicalkey(tk_key.code.sym);
         break;
         case TERMKEY_TYPE_POSITION:
              fprintf(stderr,"POSITION");
         break;
     }
     return retval;
}
