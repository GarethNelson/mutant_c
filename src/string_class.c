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

#include <mutant/gc_allocator_class.h>
#include <mutant/string_class.h>
#include <mutant/curry_func.h>
#include <mutant/librope/rope.h>

#include <string.h>
#include <stdio.h>

void string_class_init   (string_class_t* this, gc_allocator_class_t* allocator);
void string_class_destroy(string_class_t* this, gc_allocator_class_t* allocator);

void string_class_set  (string_class_t* this, char* s);
void string_class_print(string_class_t* this);

string_class_t string_class_base = {
    .Parent = {
        .instance_size = sizeof(string_class_t),
        .init          = (ClassInit)string_class_init,
    },

    .s_val = NULL,
};

void string_class_init(string_class_t* this, gc_allocator_class_t* allocator) {
     this->Parent._allocator = allocator;

     this->Parent.destroy = curry_func(string_class_destroy, this);

     this->set   = curry_func(string_class_set,   this);
     this->print = curry_func(string_class_print, this);

}

void string_class_destroy(string_class_t* this, gc_allocator_class_t* allocator) {
     if(this->s_val != NULL) {
        allocator->free(this->s_val);
        this->s_val = NULL;
     }

     free_curry(this->set);
     this->set = NULL; // good practice to set pointers to NULL after freeing

     free_curry(this->print);
     this->print = NULL;

     // TODO: add something in gc_allocator_class_t to free destroy() curry
}

void string_class_set(string_class_t* this, char* s) {
     // first we just free the old s_val - there's a more efficient way to do this that will be done later
     if(this->s_val != NULL) {
        this->Parent._allocator->free(this->s_val);
     }

     // now we allocate a new s_val and then set the value
     this->s_val = this->Parent._allocator->alloc_atomic(strlen(s)+1); // always add one byte to account for NULL terminator
     snprintf(this->s_val,strlen(s)+1,"%s",s);
}

void string_class_print(string_class_t* this) {
     // this is stupidly simple
     if(this->s_val != NULL) printf("%s",this->s_val);
}
