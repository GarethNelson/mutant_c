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
#include <stdio.h>

#include <gc.h>

#include <limits.h>
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

// how this works in brief:
//     function calls come into this code with params passed in registers (like any other C function) RDI, RSI, RDX, RCX, R8, R9
//     we want to move the params around so that the first param is the curried param (for example, the "this" param for class methods)
//     we'll call the curried param 0xFEEDFACEDEADBEEF in the explanation below, because steak is delicious

//     so we need to set things up like this:
//        rdi becomes 0xFEEDFACEDEADBEEF
//        rsi becomes the old value of rdi
//        rdx becomes the old value of rsi
//        rcx becomes the old value of rdx
//        r8  becomes the old value of rcx
//        r9  becomes the old value of r8

//     because of registers being altered though (we lose the old value) we have to do this backwards:
//        r9  is set to the value of r8
//        r8  is set to the value of rcx
//        rcx is set to the value of rdx
//        rdx is set to the value of rsi
//        rsi is set to the value of rdi
//        rdi is set to 0xFEEDFACEDEADBEEF
//
//     after that is all setup, we set the value of RAX to the address of the original function and then do a JMP RAX
//     the original function can now execute (with remapped params), and since we never touched the stack, it'll return to the caller of this trampoline code
//
// the code to do all this is stored here as x86-64 plain machine code with literal values for 0xFEEDFACEDEADBEEF, this is copied into a new mmap()-allocated buffer and modified

static unsigned char curry_tramp_x86_64_code[] = {
   0x4d, 0x89, 0xc1, // mov r9, r8
   0xf9, 0x89, 0xc8, // mov r8, rcx
   0x48, 0x89, 0xd1, // mov rcx, rdx
   0x48, 0x89, 0xf2, // mov rdx, rsi
   0x48, 0x89, 0xfe, // mov rsi, rdi
   0x48, 0xbf,       // movabs rdi
   0xef, 0xbe, 0xad, 0xde, 0xce, 0xfa, 0xed, 0xfe, // 0xFEEDFACEDEADBEEF - the curried param
   0x48, 0xb8,       // movabs rax
   0xef, 0xbe, 0xad, 0xde, 0xce, 0xfa, 0xed, 0xfe, // 0xFEEDFACEDEADBEEF - function
   0xff, 0xe0        // jmp rax
};

#define CURRY_TRAMP                curry_tramp_x86_64_code
#define CURRY_TRAMP_PARAM_OFFSET   17
#define CURRY_TRAMP_FUNCPTR_OFFSET 27
#define CURRY_TRAMP_PARAMPTR_TYPE  uint64_t
#define CURRY_TRAMP_FUNCPTR_TYPE   uint64_t

void* curry_alloc() {
      void* retval = NULL;
      retval = GC_MALLOC(sizeof(CURRY_TRAMP)+PAGESIZE);
      uint64_t ptr  = ((uint64_t)retval+PAGESIZE) & ~ (uint64_t)PAGESIZE;
      mprotect((void*)ptr, sizeof(CURRY_TRAMP), PROT_READ | PROT_WRITE);
      return (void*)ptr;
}

void* curry_func(void* func, void* param) {
      void* curry_mem = curry_alloc();
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
     void* base = GC_base(func);
     uint64_t ptr  = ((uint64_t)base+PAGESIZE) & ~ (uint64_t)PAGESIZE;
     mprotect((void*)ptr, sizeof(CURRY_TRAMP), PROT_READ | PROT_WRITE);
     GC_FREE(base);
}
