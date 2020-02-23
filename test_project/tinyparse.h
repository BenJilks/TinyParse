
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

typedef struct _Document
{
    void *memory;
    void *root;
} Document;

Document tinyparse_parse(LexerStream *lex);
void tinyparse_free_document(Document *doc);

#endif // TINYPARSE_H

#ifndef TINYPARSER_H
#define TINYPARSER_H

typedef struct _FunctionNode FunctionNode;
typedef struct _StatementNode StatementNode;

struct __attribute__((__packed__)) _FunctionNode
{
	struct __attribute__((__packed__))
	{
		Token func;
		StatementNode *test;
	};
};

struct __attribute__((__packed__)) _StatementNode
{
	struct __attribute__((__packed__))
	{
		Token test;
	};
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT
#define EXEC_COMMAND(command) \
{ \
	switch(command) \
	{ \
		case 0: ((FunctionNode*)(value + value_pointer))->func = lex->look; break; \
		case 1: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 5;value_pointer += sizeof(FunctionNode);ignore_flag = 1; break; \
		case 2: ((FunctionNode*)(value + value_pointer))->test = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(StatementNode));ignore_flag = 1; break; \
		case 3: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1; break; \
		case 4: ((StatementNode*)(value + value_pointer))->test = lex->look; break; \
		case 5: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1; break; \
	} \
}

#define TABLE_WIDTH 10
#define ENTRY_POINT 0

static char parser_table[] = 
{
	1, 0, -1, -1, -1, -1, -1, -1, -1, -1, 
	2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 
	3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 
	4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, 6, 4, -1, -1, -1, -1, -1, -1, 
	7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
};

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if 0
#include "c_header.txt"
#define ENTRY_POINT 0
#define TABLE_WIDTH 0
#define testproject_next(...) ;
static char parser_table[] = {};
#endif

#define STATE_WIDTH 2
#define ALLOCATION_BUFFER 80

typedef struct _Allocator
{
    char *memory;
    int memory_buffer;
    int memory_pointer;
} Allocator;

static Allocator create_allocator()
{
    Allocator alloc;
    alloc.memory_pointer = 0;
    alloc.memory_buffer = ALLOCATION_BUFFER;
    alloc.memory = malloc(alloc.memory_buffer);
    return alloc;
}

static void check_buffer_size(
    Allocator *alloc,
    int size)
{
    // Check if there's enough memory available
    if (alloc->memory_pointer + size >= alloc->memory_buffer)
    {
        alloc->memory_buffer += ALLOCATION_BUFFER;
        alloc->memory = realloc(alloc->memory, 
            alloc->memory_buffer);
    }
}

static void *push(
    Allocator *alloc, 
    void *data, 
    int size)
{
    void *ptr;

    // Copy memory into allocation buffer   
    check_buffer_size(alloc, size);
    ptr = alloc->memory + alloc->memory_pointer;
    memcpy(ptr, data, size);

    // Find the next space to allocate
    alloc->memory_pointer += size;
    printf("Alloc: %i\n", size);

    return ptr;
}

static void free_allocator(
    Allocator *alloc)
{
    free(alloc->memory);
}

Document tinyparse_parse(
    LexerStream *lex)
{
    // State
    int state;
    int next_state, next_commands;
    int next_index;
    int call_command, value_stack_pointer;
    int ignore_flag;

    // Stacks
    Allocator alloc;
    int call_stack[80];
    int call_stack_pointer;
    char value[80];
    int value_pointer;

    state = ENTRY_POINT;
    call_stack_pointer = 2;
    call_stack[0] = 0;              // Return state
    call_stack[1] = 0;              // Value pointer
    alloc = create_allocator();
    value_pointer = 0;

    for (;;)
    {
        // Get next state
        next_index = state * TABLE_WIDTH + lex->look.type * STATE_WIDTH;
        next_state = parser_table[next_index];
        next_commands = parser_table[next_index + 1];

        printf("%i -- %s(%i) --> %i (%i)\n", state, 
            lex->look.type_name, lex->look.type, next_state, next_commands);

        // There was a syntax error
        if (next_state == -1)
        {
            lexer_error(lex, lex->look, 
                "Unexpected token");
            testproject_next(lex);

            break;
        }

        // Execute command and go to the next state
        ignore_flag = 0;
        state = next_state;
        EXEC_COMMAND(next_commands);
        if (!ignore_flag) testproject_next(lex);

        // If a return command has been called in the 
        // outer most scope, this is an accepting state
        if (call_stack_pointer <= 0)
            break;
    }
    printf("Accepted!!!\n");

    Document doc;
    doc.memory = alloc.memory;
    doc.root = value;
    return doc;
}

void tinyparse_free_document(
    Document *doc)
{
    free(doc->memory);
}

#endif // TINYPARSE_IMPLEMENT
