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

// curried functions can only be allocated using mmap() - this is not very efficient at present but perhaps there are some performance hacks that could be done
// maybe pre-allocating a bunch of curried functions at startup, marking some as used and some as free instead of using mmap() for every invocation

void* curry_func(void* func, void* param);
void free_curry(void* func);
