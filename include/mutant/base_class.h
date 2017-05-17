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

typedef struct gc_allocator_class_t gc_allocator_class_t; // actual definition in gc_allocator_class
typedef struct base_class_t base_class_t;

typedef void* (*ClassInit)(base_class_t* this, gc_allocator_class_t* allocator);
typedef void  (*ClassDestroy)(gc_allocator_class_t* allocator);


typedef struct base_class_t {
    size_t instance_size;
    gc_allocator_class_t* _allocator;
    ClassInit init;       // NOT curried
    ClassDestroy destroy; // curried by init
} base_class_t;
