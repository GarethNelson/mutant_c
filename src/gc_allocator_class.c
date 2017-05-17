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

#include <gc.h>

#include <mutant/base_class.h>
#include <mutant/gc_allocator_class.h>
#include <mutant/curry_func.h>

#include <stdbool.h>

static bool done_gc_init = false;

// these 2 functions should not be confused with new/delete - these are init/destroy for the allocator itself, NOT for objects it allocates
void gc_allocator_class_init   (gc_allocator_class_t* this, gc_allocator_class_t* allocator);
void gc_allocator_class_destroy(gc_allocator_class_t* this, gc_allocator_class_t* allocator);

// curried functions
void  gc_allocator_class_prealloc    (gc_allocator_class_t* this, size_t s, size_t num);
void* gc_allocator_class_alloc       (gc_allocator_class_t* this, size_t s);
void* gc_allocator_class_alloc_atomic(gc_allocator_class_t* this, size_t s);
void  gc_allocator_class_free        (gc_allocator_class_t* this, void* obj);

base_class_t* gc_allocator_class_new   (gc_allocator_class_t* this, base_class_t* obj_base);
void          gc_allocator_class_delete(gc_allocator_class_t* this, base_class_t* obj);

// utility function to initialise a root allocator, basically by calling init() on gc_allocator_class_base and then returning base
gc_allocator_class_t* init_root_allocator();

gc_allocator_class_t* init_root_allocator() {
     if(!done_gc_init) {
        GC_INIT(); // this should be done once and only once
        done_gc_init = true;
     }
     gc_allocator_class_init(&gc_allocator_class_base, &gc_allocator_class_base);
     return &gc_allocator_class_base;
}

gc_allocator_class_t gc_allocator_class_base = {
     .Parent = {
         .instance_size = sizeof(gc_allocator_class_t),
         ._allocator    = &gc_allocator_class_base,
         .init          = (ClassInit)gc_allocator_class_init,
     },
};


void gc_allocator_class_init(gc_allocator_class_t* this, gc_allocator_class_t* allocator) {
     // we save the allocator here in case a future algorithm for some reason needs a hierarchy of allocators for some reason
     this->Parent._allocator = allocator;
     
     // destroy() needs to be curried in most classes, here it doesn't really do anything but we still curry it just in case it makes sense later, and as an example
     this->Parent.destroy = curry_func(gc_allocator_class_destroy, this);

     // curry the actual methods
     this->prealloc     = curry_func(gc_allocator_class_prealloc,     this);
     this->alloc        = curry_func(gc_allocator_class_alloc,        this);
     this->alloc_atomic = curry_func(gc_allocator_class_alloc_atomic, this);
     this->free         = curry_func(gc_allocator_class_free,         this);
     this->new          = curry_func(gc_allocator_class_new,          this);
     this->delete       = curry_func(gc_allocator_class_delete,       this);
}

void gc_allocator_class_destroy(gc_allocator_class_t* this, gc_allocator_class_t* allocator) {
     // all we do here is uncurry stuff
     free_curry(this->Parent.destroy);
     this->Parent.destroy = NULL;

     free_curry(this->prealloc);
     this->prealloc = NULL;

     free_curry(this->alloc);
     this->alloc = NULL;

     free_curry(this->free);
     this->free = NULL;

     free_curry(this->new);
     this->free = NULL;

     free_curry(this->delete);
     this->free = NULL;
}

void gc_allocator_class_prealloc(gc_allocator_class_t* this, size_t s, size_t num) {
     // currently a NOP
}

void* gc_allocator_class_alloc(gc_allocator_class_t* this, size_t s) {
     // currently the this param is ignored, but it's left in place here both as an example and to leave space for future implementations of GC algorithms
     return GC_MALLOC(s);
}

void gc_allocator_class_free(gc_allocator_class_t* this, void* obj) {
     GC_FREE(obj);
}

base_class_t* gc_allocator_class_new(gc_allocator_class_t* this, base_class_t* obj_base) {
     base_class_t* retval = this->alloc(obj_base->instance_size);
     obj_base->init(retval, this);
     return retval;
}

void gc_allocator_class_delete(gc_allocator_class_t* this, base_class_t* obj) {
     obj->destroy(this);
     free_curry(obj->destroy);
     obj->destroy = NULL;
     this->free(obj);
}

void* gc_allocator_class_alloc_atomic(gc_allocator_class_t* this, size_t s) {
     return GC_MALLOC_ATOMIC(s);
}
