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

#include <mutant/string_class.h>
#include <fancyrl/posix_terminal.h>

typedef struct fancyrl_class_t fancyrl_class_t;
typedef struct fancyrl_class_t {
    base_class_t Parent;

    posix_terminal_cur_pos_t toplevel_prompt_pos;

    string_class_t*  prompt; // the prompt text

    string_class_t** lines; // an array of string objects - 1 per line

    void            (*set_prompt)(string_class_t* p); // sets the prompt text
    string_class_t* (*get_input)();                   // returns the whole input string

} fancyrl_class_t;

fancyrl_class_t fancyrl_class_base;
