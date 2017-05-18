#include <stdio.h>
#include <stdlib.h>
#include <mutant/gc_allocator_class.h>
#include <mutant/string_class.h>
#include <fancyrl/posix_terminal.h>

int main(int argc, char** argv) {
    gc_allocator_class_t* my_allocator = init_root_allocator();
    
    posix_terminal_class_t* term = my_allocator->new(&posix_terminal_class_base);

    term->setup_term();
    posix_terminal_size_t term_size = term->get_term_size();
    printf("Terminal is %dX%d\n", term_size.rows, term_size.cols);
    term->restore_term();

    my_allocator->delete(term);
}
