
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
	TestingNode *node;
};

struct _TestingNode
{
	Token idk;
	Token lolz;
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT

#define TABLE_WIDTH 6
#define ENTRY_POINT 0

static char table[] = 
{
	-1, -1, -91, 4, 13, 1, 
	-1, -1, -91, 2, 2, 21, 
	-1, -1, -91, -1, -1, -91, 
	-1, -1, -91, 4, 8, -91, 
	5, 10, 21, -1, -1, -91, 
	-1, -1, -91, -1, -1, -91, 
};

#include <stdio.h>

#define STATE_WIDTH 3

#define COMMAND_NOP         0b0000
#define COMMAND_PUSH        0b1000
#define COMMAND_CALL        0b0100
#define COMMAND_RETURN      0b0010
#define COMMAND_PUSH_SUB    0b0001

void tinyparse_parse(
    LexerStream *lex)
{
    // State
    int state;
    int next_state, next_commands;
    int next_arg, next_index;
    int call_command;

    // Stacks
    int call_stack_pointer;
    int call_stack[80];

    state = ENTRY_POINT;
    call_stack_pointer = 2;
    call_stack[0] = 0;
    call_stack[1] = COMMAND_PUSH;

    while (!lex->eof_flag)
    {
        // Get next state
        next_index = state * TABLE_WIDTH + lex->look.type * STATE_WIDTH;
        next_state = table[next_index];
        next_commands = table[next_index + 1];
        next_arg = table[next_index + 2];
        printf("%i -- %s --> %i \t\t{ %i %i }\n", state, 
            lex->look.type_name, next_state, next_commands, next_arg);

        // There was a syntax error
        if (next_state == -1)
        {
            lexer_error(lex, lex->look, 
                "Unexpected token");
            testproject_next(lex);
            return;
        }

        // Set next state and run command
        if (next_commands & COMMAND_CALL) 
        {
            call_stack[call_stack_pointer] = next_arg;
            call_stack[call_stack_pointer + 1] = 0;
            if (next_commands & COMMAND_PUSH_SUB)
            {
                printf("Push sub node %s\n", lex->look.type_name);
                call_stack[call_stack_pointer + 1] |= COMMAND_PUSH;
            }
            call_stack_pointer += 2;
        }

        if (next_commands & COMMAND_RETURN) 
        {
            call_command = call_stack[--call_stack_pointer];
            next_state = call_stack[--call_stack_pointer];
            if (call_command & COMMAND_PUSH)
                printf("Push sub node now of size %i\n", next_arg);
        }

        if (next_commands & COMMAND_PUSH) 
            printf("Push token %s\n", lex->look.type_name);
        
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
