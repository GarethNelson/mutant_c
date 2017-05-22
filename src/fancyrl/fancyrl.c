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
#include <fancyrl/fancyrl.h>
#include <mutant/string_class.h>
#include <mutant/curry_func.h>

#include <string.h>
#include <stdio.h>

void fancyrl_class_init   (fancyrl_class_t* this, gc_allocator_class_t* allocator);
void fancyrl_class_destroy(fancyrl_class_t* this, gc_allocator_class_t* allocator);

fancyrl_class_t fancyrl_class_base = {
    .Parent = {
        .instance_size = sizeof(fancyrl_class_t),
        .init          = (ClassInit)fancyrl_class_init,
    },
};

void fancyrl_class_init(fancyrl_class_t* this, gc_allocator_class_t* allocator) {
     this->Parent._allocator = allocator;

     this->Parent.destroy = curry_func(fancyrl_class_destroy, this);
}

void fancyrl_class_destroy(fancyrl_class_t* this, gc_allocator_class_t* allocator) {

}

