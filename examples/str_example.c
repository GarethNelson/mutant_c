#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutant/gc_allocator_class.h>
#include <mutant/string_class.h>

int main(int argc, char** argv) {
    gc_allocator_class_t* my_allocator = init_root_allocator();

    string_class_t* string_a = my_allocator->alloc(sizeof(string_class_t));
    string_class_init(string_a, my_allocator);

    string_a->set("TEST 1\n");
    string_a->print();

/*    string_class_t* string_a = string_class_new();

    char* test_str = "TEST 1\n";
    string_a->setstr(test_str);

    string_a->print();
    string_a->Parent.DELETE(string_a);*/
}
