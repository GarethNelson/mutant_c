#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <malloc.h>
#include <termios.h>

#include <stdbool.h>

#include "utlist.h"

struct termios orig_termios;
struct termios cur_termios;

char *exprbuf = NULL;
char *tokens  = NULL;

typedef enum parsed_val_type_t {
    VAL_CONS,
    VAL_SYMBOL,
    VAL_STR,
    VAL_INT,
} parsed_val_type_t;

typedef struct parsed_val_t parsed_val_t;
typedef struct parsed_val_t {
    parsed_val_type_t type;
    int   i_val;
    char* s_val;

    parsed_val_t* car;
    parsed_val_t* cdr;
} parsed_val_t;

typedef enum token_type_t {
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_INT,
} token_type_t;

typedef struct token_list_t {
    token_type_t type;
    char* s_val;
    int   i_val;
    struct token_list_t* next;
} token_list_t;

token_list_t *parsed_tokens = NULL;
parsed_val_t *parsed_ast    = NULL;

void setraw() {
     setvbuf(stdout, NULL, _IONBF, BUFSIZ);
     tcgetattr(STDIN_FILENO,&orig_termios);

     cur_termios = orig_termios;
     cfmakeraw(&cur_termios);

     cur_termios.c_cc[VMIN]  = 1;
     cur_termios.c_cc[VTIME] = 0;

     tcsetattr(STDIN_FILENO,TCSAFLUSH,&cur_termios);

}

bool is_int(char* s) {
     while(*s) {
       if(!(isdigit(*s))) return false;
       s++;
     }
     return true;
}

void emit_token(token_type_t type, char* val) {
     token_list_t* new_token = calloc(sizeof(token_list_t),1);
     new_token->type = type;
     new_token->s_val = strdup(val);
     if(type == TOKEN_SYMBOL) {
        if(is_int(val)) {
           new_token->type = TOKEN_INT;
           new_token->i_val = atoi(val);
        }
     }
     LL_APPEND(parsed_tokens,new_token);
}

typedef enum {
    START     = 0,
    IN_STRING = 1,
    IN_SYMBOL = 2,
    IN_STRING_ESCAPE = 3,
} parse_state_t;

parsed_val_t* list_last(parsed_val_t* list) {
     parsed_val_t* c = list;
     while(c->cdr != NULL) {
        c = c->cdr;
     }
     return c;
}

parsed_val_t* list_append(parsed_val_t* list, parsed_val_t* v) {
     if(v==NULL) return list;
     if(list->car == NULL) {
        list->car = v;
        return list;
     }
     parsed_val_t* new_cons = calloc(sizeof(parsed_val_t),1);
     new_cons->type = VAL_CONS;
     new_cons->car  = v;
     new_cons->cdr  = NULL;
     list_last(list)->cdr = new_cons;
     return list;
}

parsed_val_t* read_form(token_list_t** t);
parsed_val_t* read_list(token_list_t** t) {
     parsed_val_t* retval = calloc(sizeof(parsed_val_t),1);
     retval->type = VAL_CONS;
     retval->car  = NULL;
     retval->cdr  = NULL;
     while(*t != NULL) {
        if((*t)->type == TOKEN_RPAREN) return retval;
        if(t != NULL) {
           list_append(retval,read_form(t));
        }
        *t = (*t)->next;
     }
     return retval;
}

parsed_val_t* read_form(token_list_t** t) {
     if(t==NULL) return NULL;
     parsed_val_t* retval = calloc(sizeof(parsed_val_t),1);
     switch((*t)->type) {
              case TOKEN_LPAREN:
                   free(retval);
                   *t = (*t)->next;
                   return read_list(t);
              break;
              case TOKEN_RPAREN:
                   fprintf(stderr,"Syntax error!");
                   return NULL;
              break;
              case TOKEN_INT:
                   retval->type = VAL_INT;
                   retval->i_val = (*t)->i_val;
                   return retval;
              break;
              case TOKEN_STRING:
                   retval->type = VAL_STR;
                   retval->s_val = (*t)->s_val;
                   return retval;
              break;
              case TOKEN_SYMBOL:
                   retval->type = VAL_SYMBOL;
                   retval->s_val = (*t)->s_val;
                   return retval;
              break;

     }
     return NULL;
}

void dump_val(parsed_val_t* v) {
     if(v==NULL) printf("nil");
     switch(v->type) {
        case VAL_STR:
           printf("\"%s\"", v->s_val);
        break;
        case VAL_INT:
           printf("%d",v->i_val);
        break;
        case VAL_SYMBOL:
           printf("%s",v->s_val);
        break;
        case VAL_CONS:
           printf("(");
           parsed_val_t* c = v;
           while(c != NULL) {
              if(c->car != NULL) dump_val(c->car);
              if(c->cdr != NULL) printf(" ");
              c = c->cdr;
           }
           printf(")");
        break;
     }
}

void dump_state() {
     token_list_t* t;
     int count;
     printf("\033[2J\033[H");
     fflush(stdout);
     LL_COUNT(parsed_tokens,t,count);
     printf("%d parsed tokens\n\r",count);
     parsed_ast = read_form(&parsed_tokens);
     dump_val(parsed_ast);
}

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
        if(in_char == '\n' || in_char=='\r') {
          dump_state();
        } else {
          oldsize = strlen(exprbuf);
          exprbuf = realloc(exprbuf,oldsize+2);
          exprbuf[oldsize] = in_char;
          exprbuf[oldsize+1] = 0;
          printf("\033[2J\033[H");
          fflush(stdout);
        }

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
                   case '\r':
                   break;
                   case '\n':
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
     /*   token_list_t* t;
        LL_FOREACH(parsed_tokens,t) {
           switch(t->type) {
              case TOKEN_LPAREN:
                   printf(" ( ");
              break;
              case TOKEN_RPAREN:
                   printf(" ) ");
              break;
              case TOKEN_INT:
                   printf(" INT(%d) ",t->i_val);
              break;
              case TOKEN_STRING:
                   printf(" STRING(\"%s\") ",t->s_val);
              break;
              case TOKEN_SYMBOL:
                   printf(" SYMBOL(%s) ", t->s_val);
              break;
           }
        }*/
        printf("\n\r%s",exprbuf);
        fflush(stdout);
    }
}
