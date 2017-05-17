#include <gc.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <termios.h>

struct termios frl_orig_termios;
struct termios frl_cur_termios;
char* cur_frl_input      = NULL;
size_t cur_frl_input_len = 0;

char* frl_prompt          = NULL;
int   frl_prompt_len      = 0;
int   cur_frl_line_indent = 0;
int   cur_frl_input_lines = 0;
int   cur_cursor_offs     = 0; // offset into the current input line that the cursor is located at
int   cur_cursor_line     = 0; // what line of the input the cursor is on
int   cur_cursor_col      = 0; // what screen column the cursor is on
bool   done_display       = false;

// represents keys as raw single characters read from stdin
enum RAW_KEYCHARS {
     RAW_CTRL_C    = 3, // this and ctrl-d do not have a logical assignment, because they're handled directly in handle_raw_char
     RAW_CTRL_D    = 4,
     RAW_TAB       = 9,
     RAW_ENTER     = 13,
     RAW_ESC       = 27,
     RAW_BACKSPACE = 127,
};

// represents logical keys
enum LOGICAL_KEYS {
     LOGICAL_KEY_UNKNOWN   = -2, // unknown logical key - should be ignored
     LOGICAL_KEY_INVALID   = -1, // invalid logical key - used for signalling errors
     LOGICAL_KEY_ASCII     = 0,  // just ASCII input - this obviously requires knowing the raw character too
     LOGICAL_KEY_TAB       = 1,  // autocomplete
     LOGICAL_KEY_ENTER     = 2,  // go onto a newline (duh), finish input if parsed successfully
     LOGICAL_KEY_DEL       = 3,  // delete character after cursor (or NOP if string is empty)
     LOGICAL_KEY_BACKSPACE = 4,  // delete character before cursor
     LOGICAL_KEY_UP        = 5,  // go up a line
     LOGICAL_KEY_DOWN      = 6,  // go down a line
     LOGICAL_KEY_LEFT      = 7,  // move cursor left, go to previous line if it exists and we're at the first column
     LOGICAL_KEY_RIGHT     = 8,  // move cursor right
     LOGICAL_KEY_HOME      = 9,  // go to start of expression
     LOGICAL_KEY_END       = 10, // go to end of expression
     LOGICAL_KEY_CTRL_L    = 11, // clear screen (this will NOT clear the current expression if one is being edited)
};

void idle() {
}

void init_fancy_readline() {
     setvbuf(stdout, NULL, _IONBF, BUFSIZ);
     tcgetattr(STDIN_FILENO,&frl_orig_termios);

     frl_cur_termios = frl_orig_termios;
     cfmakeraw(&frl_cur_termios);

     frl_cur_termios.c_cc[VMIN]  = 1;
     frl_cur_termios.c_cc[VTIME] = 0;

     tcsetattr(STDIN_FILENO,TCSAFLUSH,&frl_cur_termios);

     printf("\n");
}

void display_main_frl_prompt() {
     printf("%s> ",frl_prompt);
     fflush(stdout);
}

void fancy_readline_redisplay() {
     int i=0;
     if(done_display) {
        printf("\r");
        if(cur_frl_input_lines>0) {
           printf("\x1b[%dA\r",cur_frl_input_lines);
        }
        display_main_frl_prompt();
        fflush(stdout);
     }
     
     if(cur_frl_input_len==0) return;
     
     for(i=0; i<strlen(cur_frl_input); i++) {
         if(cur_frl_input[i]=='\n') {
            printf("\n\r\x1b[0K");
            printf("%*s %s", frl_prompt_len-3,"","... ");
         } else {
            printf("%c",cur_frl_input[i]);
         }
     }
     done_display = true;
     if(cur_cursor_col >= (frl_prompt_len+2)) printf("\r\x1b[%dC",cur_cursor_col);
     fflush(stdout);
}

void start_fancy_readline(char* prompt) {
     frl_prompt     = strdup(prompt);
     frl_prompt_len = strlen(prompt);
     cur_cursor_col = frl_prompt_len+2;
     display_main_frl_prompt();
}

void shutdown_fancy_readline() {
     tcsetattr(STDIN_FILENO,TCSAFLUSH,&frl_orig_termios); // restore original terminal settings
     printf("\n"); // print a newline so we don't have weirdness
}

int fancy_readline_read_esc() {
     char seq_buf[4];
     read(STDIN_FILENO,seq_buf,1);
     read(STDIN_FILENO,seq_buf+1,1);
     switch(seq_buf[0]) {
          case '[':
               if(seq_buf[1] >= '0' && seq_buf[1] <= '9') {
                  return LOGICAL_KEY_INVALID; // TODO - extended escape sequences here
               }
               switch(seq_buf[1]) {
                  case 'A':
                     return LOGICAL_KEY_UP;
                  break;
                  case 'B':
                     return LOGICAL_KEY_DOWN;
                  break;
                  case 'C':
                     return LOGICAL_KEY_RIGHT;
                  break;
                  case 'D':
                     return LOGICAL_KEY_LEFT;
                  break;
                  case 'H':
                     return LOGICAL_KEY_HOME;
                  break;
                  case 'F':
                     return LOGICAL_KEY_END;
                  break;
                  default:
                     return LOGICAL_KEY_UNKNOWN;
                  break;
               }
          break;

          case 'O':
               switch(seq_buf[1]) {
                   case 'H':
                        return LOGICAL_KEY_HOME;
                   break;
                   case 'F':
                        return LOGICAL_KEY_END;
                   break;
                   default:
                        return LOGICAL_KEY_UNKNOWN; // this shouldn't happen on a sane terminal
                   break;
               }
          break;

          default:
               return LOGICAL_KEY_UNKNOWN;
          break;
     }

     return LOGICAL_KEY_UNKNOWN;
}

void fancy_readline_addchar(char c) {
     if(cur_cursor_offs == cur_frl_input_len) {
        cur_frl_input = realloc(cur_frl_input,cur_frl_input_len+2); // always remember the NULL terminator
        cur_frl_input[cur_frl_input_len]   = c;
        cur_frl_input[cur_frl_input_len+1] = 0;
     }
     cur_frl_input_len++;
     cur_cursor_offs++;
     cur_cursor_col++; // TODO - take width of terminal into account here and wrap or something
     fprintf(stderr,"Cursor offset: %d, input length: %d\n",cur_cursor_offs,(int)cur_frl_input_len);
}

void fancy_readline_addstr(char* s) {
     int new_len = cur_frl_input_len+strlen(s)+1;
     cur_frl_input = realloc(cur_frl_input,new_len);
     strncpy(cur_frl_input+cur_frl_input_len,s,new_len);
     cur_frl_input_len = new_len;
     cur_cursor_offs += strlen(s);
     cur_cursor_col  += strlen(s);
}

// calculates the indent (number of spaces) in the last line of string s
int fancy_readline_calc_indent(char* s) {
    char* last_line = rindex(s,'\n');
    fprintf(stderr,"LAST LINE: %s\n\r",last_line);
    if(last_line == NULL) { // there's only one line in the string
       return strspn((const char*)s," ");
    } else {
       return strspn((const char*)last_line+1, " ");
    }
}

void fancy_readline_backspace() {
     if(cur_cursor_col <= (frl_prompt_len+2)) return;
     if(cur_frl_input_len <= 0) return;
     cur_frl_input[cur_cursor_offs-1] = 0;
     cur_cursor_offs--;
     cur_cursor_col--;
     cur_frl_input_len--;
     printf("\b \b");
     fancy_readline_redisplay();
}

void fancy_readline_moveleft() {
     if(cur_cursor_col <= (frl_prompt_len+2)) return; // TODO: handle going to the previous line
     cur_cursor_offs--;
     cur_cursor_col--;
     printf("\b");
     fancy_readline_redisplay();
}

void fancy_readline_moveright() {
     if(cur_cursor_offs == cur_frl_input_len) return; // TODO: take screen width into account here
     cur_cursor_offs++;
     cur_cursor_col++;
     printf("\x1b[%dC",1);
     fancy_readline_redisplay();
}

void fancy_readline_handle_logical_char(int logical_key, char raw_c) {
     int i=0;
/*
 LOGICAL_KEY_UNKNOWN   = -2, // unknown logical key - should be ignored
     LOGICAL_KEY_INVALID   = -1, // invalid logical key - used for signalling errors
     LOGICAL_KEY_ASCII     = 0,  // just ASCII input - this obviously requires knowing the raw character too
     LOGICAL_KEY_TAB       = 1,  // autocomplete
     LOGICAL_KEY_ENTER     = 2,  // go onto a newline (duh), finish input if parsed successfully
     LOGICAL_KEY_DEL       = 3,  // delete character after cursor (or NOP if string is empty)
     LOGICAL_KEY_BACKSPACE = 4,  // delete character before cursor
     LOGICAL_KEY_UP        = 5,  // go up a line
     LOGICAL_KEY_DOWN      = 6,  // go down a line
     LOGICAL_KEY_LEFT      = 7,  // move cursor left, go to previous line if it exists and we're at the first column
     LOGICAL_KEY_RIGHT     = 8,  // move cursor right
     LOGICAL_KEY_HOME      = 9,  // go to start of expression
     LOGICAL_KEY_END       = 10, // go to end of expression
     LOGICAL_KEY_CTRL_L    = 11, // clear screen (this will NOT clear the current expression if one is being edited)

*/
     switch(logical_key) {
        case LOGICAL_KEY_UNKNOWN:
             // TODO - log unknowns and possibly give a "not bound" error message
        break;
        case LOGICAL_KEY_ASCII:
             fancy_readline_addchar(raw_c);
             fancy_readline_redisplay();
        break;
        case LOGICAL_KEY_BACKSPACE:
             fancy_readline_backspace();

        break;
        case LOGICAL_KEY_LEFT:
             fancy_readline_moveleft();
        break;
        case LOGICAL_KEY_RIGHT:
             fancy_readline_moveright();
        break;
        case LOGICAL_KEY_ENTER:
             cur_frl_input_lines++;
             cur_frl_line_indent = fancy_readline_calc_indent(cur_frl_input);
             fancy_readline_addchar('\n');
             cur_cursor_col = frl_prompt_len+2;
             for(i=0; i<cur_frl_line_indent; i++) fancy_readline_addchar(' '); // TODO - make this more efficient
             // TODO - check if we need to do complete callback here
             printf("\n\r");
             fancy_readline_redisplay();
             break;
        break;
     }
}

bool fancy_readline_handle_raw_char(char c) {
     // here we turn raw characters into logical keys if required, or we just directly append them to input
     // if the key was ctrl-c or ctrl-d we return false to indicate reading should finish
     int log_char = LOGICAL_KEY_ASCII; // if we don't see it as a special key below, this is the default
     switch(c) {
        case RAW_CTRL_C:
             return false; // don't bother with further processing, just return false
        break;

        case RAW_CTRL_D:
             return false; // same as ctrl-c
        break;

        case RAW_ESC:
             log_char = fancy_readline_read_esc();
        break;

        case RAW_ENTER:
             log_char = LOGICAL_KEY_ENTER;
        break;
       
        case RAW_BACKSPACE:
             log_char = LOGICAL_KEY_BACKSPACE;
        break;
 
        default:
             if(c >= 32 && c <= 126) {
                log_char = LOGICAL_KEY_ASCII;
             } else {
                log_char = LOGICAL_KEY_UNKNOWN;
             }
        break;
        
     };
     if(log_char == LOGICAL_KEY_INVALID) return false;
     fancy_readline_handle_logical_char(log_char, c);
     return true;
}

void do_fancy_readline() {
     start_fancy_readline("Fancy");
     char c;
     fd_set fds;
     struct timeval tv;
     int r;
     while(!feof(stdin)) {
        tv.tv_sec  = 0;
        tv.tv_usec = 200000.0; // 5 times per second - should be enough time for doing garbage collection etc
        FD_ZERO (&fds);
        FD_SET (STDIN_FILENO, &fds);
        r = select (FD_SETSIZE, &fds, NULL, NULL, &tv);
        if( r < 0) { 
           continue;
        }
        if(FD_ISSET(STDIN_FILENO,&fds)) {
           if(read(0, &c, 1)==1) {
              if(!fancy_readline_handle_raw_char(c)) return;
           }
        } else {
           if(r==0) idle(); // if we timed out while waiting for input, run idle() - this should be hooked for stuff like garbage collection etc
        }
        
     }
}

int main(int argc, char** argv) {
    init_fancy_readline();
    do_fancy_readline();
    shutdown_fancy_readline();
    return 0;
}
