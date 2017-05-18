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
//TODO - look into using gc/cord instead of librope
#include <mutant/librope/rope.h>

typedef struct string_class_t string_class_t;
typedef struct string_class_t {
    base_class_t Parent;
    
    rope *s_val;

    size_t (*get_length)();
    void   (*insert_cstr_at) (size_t pos, char* s); // sadly we can't do polymorphism on native C types
    void   (*append_cstr)    (char* s);
    void   (*set)            (char* s);
    void   (*print)();

} string_class_t;

string_class_t string_class_base;
