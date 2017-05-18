#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include <mutant/gc_allocator_class.h>
#include <mutant/string_class.h>
#include <fancyrl/posix_terminal.h>

void idle(posix_terminal_class_t* term) {
     // we use this idle function to print dots to the output while waiting on the user
     // in real applications idle functions should be used for stuff like garbage collection etc
     term->write_char('.');
}

char char_read(posix_terminal_class_t* term) {
     // this is in fact a sane way to do things and does not belong in the terminal class
     // the reason it doesn't belong in the class is because the client app's event loop might be implemented in various ways
     // there's also some excessively unrealistic error handling for stuff like stdin not being valid
     fd_set fds;
     int r;
     struct timeval tv;
     posix_keyinput_t in_key;
     for(;;) {
         tv.tv_sec  = 0;
         tv.tv_usec = 200000; // this enables us to run an idle() function 5 times per second
         FD_ZERO(&fds);
         FD_SET(STDIN_FILENO, &fds);
         r = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
         switch(r) {
            case 0:
                 idle(term); // if the return value is 0, we have nothing to process so run an idle function
            break;
            case -1:
                 switch(errno) {
                     case EBADF:
                          fprintf(stderr,"THIS SHOULD NEVER HAPPEN! GOT EBADF IN select()\n");
                     break;
                     case EINTR:
                          // do nothing here, it doesn't matter if we get a signal
                     break;
                     case EINVAL:
                          fprintf(stderr,"THIS SHOULD NEVER HAPPEN! GOT EINVAL IN select()\n");
                     break;
                     case ENOMEM:
                          fprintf(stderr,"Get more RAM, noob - ENOMEM\n");
                     break;
                 }
            break;
            case 1:
                 in_key = term->read_key();
                 // here we spit out some info about the key and then return the raw character code
                 switch(in_key.logical_key) {
                     case LOGICAL_KEY_UNKNOWN:
                          term->write_cstr("\nI don't know what key that was!\n");
                     break;
                     case LOGICAL_KEY_INVALID:
                          term->write_cstr("\nSomething went wrong when I read that key\n");
                     break;
                     case LOGICAL_KEY_ASCII:
                          term->write_cstr("\nThat was just a normal plain old ASCII character, it was this one: ");
                          term->write_char(in_key.raw_char);
                          term->write_char('\n');
                     break;
                     case LOGICAL_KEY_TAB:
                          term->write_cstr("\nYou pressed tab\n");
                     break;
                     case LOGICAL_KEY_ENTER:
                          term->write_cstr("\nYou pressed enter\n");
                     break;
                     case LOGICAL_KEY_DEL:
                          term->write_cstr("\nYou pressed delete\n");
                     break;
                     case LOGICAL_KEY_BACKSPACE:
                          term->write_cstr("\nYou pressed backspace\n");
                     break;
                     case LOGICAL_KEY_UP:
                          term->write_cstr("\nYou pressed the up arrow\n");
                     break;
                     case LOGICAL_KEY_DOWN:
                          term->write_cstr("\nYou pressed the down arrow\n");
                     break;
                     case LOGICAL_KEY_LEFT:
                          term->write_cstr("\nYou pressed the left arrow\n");
                     break;
                     case LOGICAL_KEY_RIGHT:
                          term->write_cstr("\nYou pressed the right arrow\n");
                     break;
                     case LOGICAL_KEY_HOME:
                          term->write_cstr("\nYou pressed home\n");
                     break;
                     case LOGICAL_KEY_END:
                          term->write_cstr("\nMy only friend\n");
                     break;
                     case LOGICAL_KEY_CTRL_L:
                          term->write_cstr("\nYou pressed Ctrl-L\n");
                     break;
                     case LOGICAL_KEY_CTRL_C:
                          term->write_cstr("\nYou pressed Ctrl-C\n");
                     break;
                 }
                 return in_key.raw_char;
            break;
         }
         
     }
}

int main(int argc, char** argv) {
    gc_allocator_class_t* my_allocator = init_root_allocator();
    
    posix_terminal_class_t* term = my_allocator->new(&posix_terminal_class_base);

    term->setup_term();
    posix_terminal_size_t term_size = term->get_term_size();
    char* outbuf = my_allocator->alloc_atomic(128);
    snprintf(outbuf,128,"Terminal is %dX%d\n\r", term_size.rows, term_size.cols);
    term->write_cstr(outbuf);

    printf("Press a key, any key");

    char in_char = char_read(term);

    snprintf(outbuf,128,"You pressed %d\n",in_char); 
    term->write_cstr(outbuf);

    term->restore_term();
    my_allocator->delete(term);
}
