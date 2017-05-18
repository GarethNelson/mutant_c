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

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <termcap.h>

void posix_terminal_class_init   (posix_terminal_class_t* this, gc_allocator_class_t* allocator);
void posix_terminal_class_destroy(posix_terminal_class_t* this, gc_allocator_class_t* allocator);

void                  posix_terminal_class_setup_term   (posix_terminal_class_t* this);
void                  posix_terminal_class_sig_handle   (posix_terminal_class_t* this, int signum);
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
     this->sig_handle    = curry_func(posix_terminal_class_sig_handle,    this);
     this->get_term_size = curry_func(posix_terminal_class_get_term_size, this);
}

// WARNING: this does NOT restore the terminal back to original settings, it's only a memory deallocation thing
void posix_terminal_class_destroy(posix_terminal_class_t* this, gc_allocator_class_t* allocator) {
     free_curry(this->setup_term);
     this->setup_term = NULL;

     free_curry(this->sig_handle);
     this->sig_handle = NULL;

     free_curry(this->get_term_size);
     this->get_term_size = NULL;
}

void posix_terminal_class_setup_term(posix_terminal_class_t* this) {
     this->cur_size = this->get_term_size();
     signal(SIGWINCH,this->sig_handle);
}

posix_terminal_size_t posix_terminal_class_get_term_size(posix_terminal_class_t* this) {
     char *termtype = getenv("TERM");
     char buf[2048];
     tgetent(buf,termtype);
     posix_terminal_size_t retval = {.rows = tgetnum("li"), .cols = tgetnum("co")};
     return retval;

}

void posix_terminal_class_sig_handle(posix_terminal_class_t* this, int signum) {
     this->cur_size = this->get_term_size();
}
