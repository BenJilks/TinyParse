
#include <stdio.h>

#ifndef LEXER_H
#define LEXER_H

typedef struct _Token
{
    int data_index;
    int len;
    char type;

    int line_index;
    int line_no, column_no;
    const char *type_name;
} Token;

typedef struct _LexerStream
{
    FILE *file;
    int index, line_index;
    int line, column;
    int eof_flag;

    char *cache;
    Token look;
} LexerStream;

LexerStream lexer_stream_open(const char *file_path);
void lexer_stream_close(LexerStream *stream);
char lexer_read_char(LexerStream *lex, int index);
char *lexer_read_buffer(LexerStream *lex, int index, int len);
void lexer_error(LexerStream *lex, Token token, const char *error);

#endif // LEXER_H

#ifndef TINYPARSE_H
#define TINYPARSE_H

void tinyparse_parse(LexerStream *lex);

#endif // TINYPARSE_H

#ifndef TINYPARSER_H
#define TINYPARSER_H

typedef struct _FunctionNode FunctionNode;
FunctionNode *parse_Function(LexerStream *lex);

typedef struct _TestingNode TestingNode;
TestingNode *parse_Testing(LexerStream *lex);

struct _FunctionNode
{
};

struct _TestingNode
{
	Token lolz;
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT

#define TABLE_WIDTH 4
#define ENTRY_POINT 0

static char table[] = 
{
	-1, -1, 4, 2, 
	-1, -1, 2, 0, 
	-1, -1, -1, -1, 
	-1, -1, 4, 0, 
	5, 4, -1, -1, 
	-1, -1, -1, -1, 
};

#include <stdio.h>

#define COMMAND_NOP     0b000
#define COMMAND_PUSH    0b001
#define COMMAND_CALL    0b010
#define COMMAND_RETURN  0b100

void tinyparse_parse(
    LexerStream *lex)
{
    // State
    int state;
    int next_state, next_commands;
    int next_index;

    // Stacks
    int call_stack_pointer;
    int call_stack[80];

    state = ENTRY_POINT;
    call_stack_pointer = 1;
    while (!lex->eof_flag)
    {
        // Get next state
        next_index = state * TABLE_WIDTH + lex->look.type * 2;
        next_state = table[next_index];
        next_commands = table[next_index + 1];
        printf("%i -- %s --> %i \t\t{ %i %i }\n", state, 
            lex->look.type_name, next_state, next_commands, COMMAND_RETURN);

        // There was a syntax error
        if (next_state == -1)
        {
            lexer_error(lex, lex->look, 
                "Unexpected token");
            testproject_next(lex);
            return;
        }

        // Set next state and run command
        if (next_commands & COMMAND_CALL) call_stack[call_stack_pointer++] = state + 1;
        if (next_commands & COMMAND_RETURN) next_state = call_stack[--call_stack_pointer];
        state = next_state;
        testproject_next(lex);

        // If a return command has been called in the 
        // outer most scope, this is an accepting state
        if (call_stack_pointer <= 0)
            break;
    }

    printf("Accepted!!!\n");
}

#endif // TINYPARSE_IMPLEMENT
