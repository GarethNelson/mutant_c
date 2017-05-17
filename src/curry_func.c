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

#include <sys/mman.h>
#include <string.h>
#include <stdint.h>

// currently only works with a single param being passed to the curried function because i'm a lazy fucker
// as seen before, also only works on x86-64, at some point i'll add support for 32-bit x86 and possibly ARM
// i am however, a lazy fucker
static unsigned char curry_tramp_x86_64_code[] = {
   0x48, 0x89, 0xfe, // mov rsi,rdi
   0x48, 0xbf,       // movabs rdi
   0xef, 0xbe, 0xad, 0xde, 0xce, 0xfa, 0xed, 0xfe, // 0xFEEDFACEDEADBEEF - the curried param
   0x48, 0xb8,       // movabs rax
   0xef, 0xbe, 0xad, 0xde, 0xce, 0xfa, 0xed, 0xfe, // 0xFEEDFACEDEADBEEF - function
   0xff, 0xe0        // ret
};

#define CURRY_TRAMP                curry_tramp_x86_64_code
#define CURRY_TRAMP_PARAM_OFFSET   5
#define CURRY_TRAMP_FUNCPTR_OFFSET 15
#define CURRY_TRAMP_PARAMPTR_TYPE  uint64_t
#define CURRY_TRAMP_FUNCPTR_TYPE   uint64_t

void* curry_func(void* func, void* param) {
      void* curry_mem            = mmap(NULL, sizeof(CURRY_TRAMP), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      unsigned char *mapped_code = (unsigned char*)curry_mem;

      memcpy(curry_mem, CURRY_TRAMP, sizeof(CURRY_TRAMP));

      CURRY_TRAMP_PARAMPTR_TYPE *func_param_ptr  = (CURRY_TRAMP_PARAMPTR_TYPE *)&(mapped_code[CURRY_TRAMP_PARAM_OFFSET]);
      func_param_ptr[0]                          = (CURRY_TRAMP_PARAMPTR_TYPE)param;

      CURRY_TRAMP_FUNCPTR_TYPE *func_addr_ptr    = (CURRY_TRAMP_FUNCPTR_TYPE *)&(mapped_code[CURRY_TRAMP_FUNCPTR_OFFSET]);
      func_addr_ptr[0]                           = (CURRY_TRAMP_FUNCPTR_TYPE)func;

      mprotect(curry_mem, sizeof(CURRY_TRAMP), PROT_READ | PROT_EXEC); // https://www.openbsd.org/lyrics.html#33
      return curry_mem;

}

void free_curry(void* func) {
     munmap(func, sizeof(CURRY_TRAMP));
}
