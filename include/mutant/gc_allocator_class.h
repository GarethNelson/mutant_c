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

#include <stddef.h>

#include <mutant/base_class.h>

typedef struct gc_allocator_class_t gc_allocator_class_t;
typedef struct gc_allocator_class_t {
    base_class_t Parent;
    void          (*prealloc)(size_t s, size_t num);
    void*         (*alloc)   (size_t s);
    void          (*free)    (void* obj);
    void*         (*new)(void* obj_base);
    void          (*delete)(void* obj);
} gc_allocator_class_t;

gc_allocator_class_t gc_allocator_class_base;

gc_allocator_class_t* init_root_allocator(); 
