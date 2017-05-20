#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <malloc.h>
#include <termios.h>

struct termios orig_termios;
struct termios cur_termios;

char *exprbuf = NULL;
char *tokens  = NULL;

void setraw() {
     setvbuf(stdout, NULL, _IONBF, BUFSIZ);
     tcgetattr(STDIN_FILENO,&orig_termios);

     cur_termios = orig_termios;
     cfmakeraw(&cur_termios);

     cur_termios.c_cc[VMIN]  = 1;
     cur_termios.c_cc[VTIME] = 0;

     tcsetattr(STDIN_FILENO,TCSAFLUSH,&cur_termios);

}

void emit_token(char* token) {
     size_t old_s_size = strlen(tokens);
     char* new_tokens = calloc(sizeof(char),old_s_size+strlen(token)+2);
     snprintf(new_tokens,old_s_size+strlen(token)+2,"%s %s ",tokens,token);
     free(tokens);
     tokens = new_tokens;
}

typedef enum {
    START     = 0,
    IN_STRING = 1,
    IN_SYMBOL = 2,
} parse_state_t;


int main(int argc, char** argv) {
    setraw();
    char in_char;
    size_t oldsize;
    parse_state_t cur_state = START;

    char* s_val_buf = NULL;

    tokens  = calloc(1,1);
    exprbuf = calloc(1,1);

    char *tmpbuf = NULL;

    for(;;) {
        in_char = getchar();
        oldsize = strlen(exprbuf);
        exprbuf = realloc(exprbuf,oldsize+2);
        exprbuf[oldsize] = in_char;
        exprbuf[oldsize+1] = 0;
        printf("\033[2J\033[H");

        switch(cur_state) {        
           case START:
                switch(in_char) {
                   case '(':
                       emit_token("LPAREN");
                   break;
                   case ')':
                       emit_token("RPAREN");
                   break;
                   case ' ':
                   break;
                   case '"':
                       cur_state = IN_STRING;
                       s_val_buf = calloc(sizeof(char),1024);
                   break;
                   default:
                       cur_state = IN_SYMBOL;
                       s_val_buf = calloc(sizeof(char),1024);
                       s_val_buf[0] = in_char;
                   break;
                }
           break;
           case IN_STRING:
                switch(in_char) {
                   case '"':
                        tmpbuf = (char*)calloc(sizeof(char),strlen(s_val_buf)+12);
                        snprintf(tmpbuf,strlen(s_val_buf)+12,"STRING(\"%s\")",s_val_buf);
                        cur_state = START;
                        emit_token(tmpbuf);
                        free(tmpbuf);
                        free(s_val_buf);
                   break;
                   default:
                        s_val_buf[strlen(s_val_buf)]=in_char;
                        s_val_buf[strlen(s_val_buf)+1]=0;
                   break;
                }
           break;
           case IN_SYMBOL:
                switch(in_char) {
                 case ')':
                        tmpbuf = (char*)calloc(sizeof(char),strlen(s_val_buf)+12);
                        snprintf(tmpbuf,strlen(s_val_buf)+12,"SYMBOL(\"%s\")",s_val_buf);
                        cur_state = START;
                        emit_token(tmpbuf);
                        free(tmpbuf);
                        free(s_val_buf);
                        emit_token("RPAREN");
                 break;
                 case ' ':
                        tmpbuf = (char*)calloc(sizeof(char),strlen(s_val_buf)+12);
                        snprintf(tmpbuf,strlen(s_val_buf)+12,"SYMBOL(\"%s\")",s_val_buf);
                        cur_state = START;
                        emit_token(tmpbuf);
                        free(tmpbuf);
                        free(s_val_buf);
                break;
                default:
                        s_val_buf[strlen(s_val_buf)]=in_char;
                        s_val_buf[strlen(s_val_buf)+1]=0;
                break;
               }
           break;
        }
        fflush(stdout);
        printf("%s\n\r%s",tokens,exprbuf);
        fflush(stdout);
    }
}
