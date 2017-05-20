#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <malloc.h>
#include <termios.h>

#include "utlist.h"

struct termios orig_termios;
struct termios cur_termios;

char *exprbuf = NULL;
char *tokens  = NULL;

typedef enum token_type_t {
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SYMBOL,
    TOKEN_STRING,
} token_type_t;

typedef struct token_list_t {
    token_type_t type;
    char* s_val;
    struct token_list_t* next;
} token_list_t;

token_list_t *parsed_tokens = NULL;

void setraw() {
     setvbuf(stdout, NULL, _IONBF, BUFSIZ);
     tcgetattr(STDIN_FILENO,&orig_termios);

     cur_termios = orig_termios;
     cfmakeraw(&cur_termios);

     cur_termios.c_cc[VMIN]  = 1;
     cur_termios.c_cc[VTIME] = 0;

     tcsetattr(STDIN_FILENO,TCSAFLUSH,&cur_termios);

}

void emit_token(token_type_t type, char* val) {
     token_list_t* new_token = calloc(sizeof(token_list_t),1);
     new_token->type = type;
     new_token->s_val = strdup(val);
     LL_APPEND(parsed_tokens,new_token);
}

typedef enum {
    START     = 0,
    IN_STRING = 1,
    IN_SYMBOL = 2,
    IN_STRING_ESCAPE = 3,
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
        fflush(stdout);
        switch(cur_state) {        
           case START:
                switch(in_char) {
                   case '(':
                       emit_token(TOKEN_LPAREN,"(");
                   break;
                   case ')':
                       emit_token(TOKEN_RPAREN,")");
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
                       s_val_buf[strlen(s_val_buf)+1]=0;
                   break;
                }
           break;
           case IN_STRING_ESCAPE:
                s_val_buf[strlen(s_val_buf)]=in_char;
                s_val_buf[strlen(s_val_buf)+1]=0;
                cur_state = IN_STRING;
           break;
           case IN_STRING:
                switch(in_char) {
                   case '\\':
                        cur_state = IN_STRING_ESCAPE;
                   break;
                   case '"':
                        emit_token(TOKEN_STRING,s_val_buf);
                        free(s_val_buf);
                        cur_state = START;
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
                        emit_token(TOKEN_SYMBOL,s_val_buf);
                        free(s_val_buf);
                        emit_token(TOKEN_RPAREN,")");
                        cur_state = START;
                 break;
                 case ' ':
                        emit_token(TOKEN_SYMBOL,s_val_buf);
                        free(s_val_buf);
                        cur_state = START;
                break;
                default:
                        s_val_buf[strlen(s_val_buf)]=in_char;
                        s_val_buf[strlen(s_val_buf)+1]=0;
                break;
               }
           break;
        }
        fflush(stdout);
        printf("\n\r");
        token_list_t* t;
        LL_FOREACH(parsed_tokens,t) {
           switch(t->type) {
              case TOKEN_LPAREN:
                   printf(" ( ");
              break;
              case TOKEN_RPAREN:
                   printf(" ) ");
              break;
              case TOKEN_STRING:
                   printf(" STRING(\"%s\") ",t->s_val);
              break;
              case TOKEN_SYMBOL:
                   printf(" SYMBOL(%s) ", t->s_val);
              break;
           }
        }
        printf("\n\r%s",exprbuf);
        fflush(stdout);
    }
}
