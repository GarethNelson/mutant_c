#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutant/gc_allocator_class.h>
#include <mutant/string_class.h>

int main(int argc, char** argv) {
    gc_allocator_class_t* my_allocator = init_root_allocator();

    string_class_t* string_a = my_allocator->new(&string_class_base);
    string_class_t* string_b = my_allocator->new(&string_class_base);

    string_a->set("TEST 1\n");
    string_a->print();

    string_a->set("TEST 2\n");
    string_a->print();

    string_b->set("TEST 3\n");
    string_b->print();

    my_allocator->delete(string_a);
    string_b->set("TEST 4\n");
    string_b->print();
    my_allocator->delete(string_b);

    string_class_t* string_c = my_allocator->new(&string_class_base);
    string_c->set("Hi");
    string_c->insert_cstr_at(1,"ello world, th");
    string_c->append_cstr("s is a test\n");
    string_c->print();

    printf("The above string is %d characters long\n",string_c->get_length());
    my_allocator->delete(string_c);

}
