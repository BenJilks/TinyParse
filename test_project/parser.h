
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

typedef struct _MemoryList
{
    char *data;
    int buffer_size;
    struct _MemoryList *next;
} MemoryList;

typedef struct _Document
{
    MemoryList *memory;
    void *root;
} Document;

void tinyparse_debug_table();
Document tinyparse_parse(LexerStream *lex);
void tinyparse_free_document(Document *doc);

#endif // TINYPARSE_H

#ifndef TINYPARSER_H
#define TINYPARSER_H

typedef struct _AddOpNode AddOpNode;
typedef struct _FunctionNode FunctionNode;

struct _AddOpNode
{
	struct 
	{
		int type;
		union
		{
		};
	};
};

struct _FunctionNode
{
	struct 
	{
		Token left;
		int has_right;
		struct 
		{
			FunctionNode *right;
		};
	};
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT
static int command_flags[] = {16, 4413976, 8, 0, 1, -16496, 32, 2121136, 64, 1, 4, 0, 4, 1, 2, 1, 8, 0, };
#define EXEC_COMMAND(command) \
{ \
	switch(command) \
	{ \
		case 0: ((AddOpNode*)(value + value_pointer))->type = lex->look.type;ignore_flag = 1;printf("Mark type, "); break; \
		case 1: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 2: ((FunctionNode*)(value + value_pointer))->left = lex->look; break; \
		case 3: ((FunctionNode*)(value + value_pointer))->has_right = 1;ignore_flag = 1;printf("Set flag, "); break; \
		case 4: ((FunctionNode*)(value + value_pointer))->has_right = 0;ignore_flag = 1;printf("Unset flag, "); break; \
		case 5: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 0;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call AddOp, "); break; \
		case 6: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 5;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call Function, "); break; \
		case 7: ((FunctionNode*)(value + value_pointer))->right = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(FunctionNode));ignore_flag = 1; break; \
		case 8: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
	} \
}

#define TABLE_WIDTH 6
#define TABLE_SIZE 13
#define ENTRY_POINT 5
#define LEXER_NEXT testproject_next

static char parser_table[] = 
{
	1, 0, 1, 0, 1, 0, 
	-1, -1, 2, -1, 3, -1, 
	4, 1, 4, 1, 4, 1, 
	4, 1, 4, 1, 4, 1, 
	-1, -1, -1, -1, -1, -1, 
	6, 2, -1, -1, -1, -1, 
	7, 3, 7, 3, 7, 3, 
	9, 5, 9, 5, 9, 5, 
	12, 8, 12, 8, 12, 8, 
	10, 6, -1, -1, -1, -1, 
	11, 7, 11, 7, 11, 7, 
	12, 8, 12, 8, 12, 8, 
	-1, -1, -1, -1, -1, -1, 
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
#define ALLOCATION_BUFFER 1024

#define FLAG_NOP        0b00000000
#define FLAG_SET        0b00000001
#define FLAG_PUSH_SUB   0b00000010
#define FLAG_CALL       0b00000100
#define FLAG_RETURN     0b00001000
#define FLAG_MARK_TYPE  0b00010000
#define FLAG_SET_FLAG   0b00100000
#define FLAG_UNSET_FLAG 0b01000000
#define FLAG_NULL       0b10000000

typedef struct _Allocator
{
    MemoryList *memory;
    MemoryList *curr;
    char *ptr;
} Allocator;

static Allocator create_allocator()
{
    Allocator alloc;
    MemoryList *mem = (MemoryList*)malloc(sizeof(MemoryList));

    mem->data = malloc(ALLOCATION_BUFFER);
    mem->next = NULL;
    mem->buffer_size = ALLOCATION_BUFFER;
    alloc.memory = mem;
    alloc.curr = mem;
    alloc.ptr = mem->data;
    return alloc;
}

static void check_buffer_size(
    Allocator *alloc,
    int size)
{
    // Check if there's enough memory available
    while ((alloc->ptr - alloc->curr->data) + size >= alloc->curr->buffer_size)
    {
        MemoryList *mem = (MemoryList*)malloc(sizeof(MemoryList));
        mem->buffer_size = size + ALLOCATION_BUFFER;
        mem->data = (char*)malloc(mem->buffer_size);
        mem->next = NULL;

        alloc->curr->next = mem;
        alloc->curr = mem;
        alloc->ptr = mem->data;
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
    ptr = (void*)alloc->ptr;
    memcpy(ptr, data, size);

    // Find the next space to allocate
    alloc->ptr += size;
    return ptr;
}

static void free_memory_list(
    MemoryList *mem)
{
    if (mem->next != NULL)
        free_memory_list(mem->next);
    
    free(mem->data);
    free(mem);
}

static void free_allocator(
    Allocator *alloc)
{
    free_memory_list(alloc->memory);
}

void tinyparse_debug_table()
{
    int i, j;

    for (i = 0; i < TABLE_SIZE; i++)
    {
        for (j = 0; j < TABLE_WIDTH; j += 2)
        {
            int to, command;
            const char *name;
            
            to = parser_table[i * TABLE_WIDTH + j + 0];
            command = parser_table[i * TABLE_WIDTH + j + 1];
            name = type_names[j / 2];
            if (to != -1)
            {
                printf("%i -%s-> %i (%i)\n", i, name, to, command);
            }
        }
    }
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
    char *value;
    int *call_stack;
    int call_stack_pointer;
    int value_pointer;

    state = ENTRY_POINT;
    call_stack_pointer = 2;
    call_stack = malloc(sizeof(int) * 1024);
    call_stack[0] = 0;              // Return state
    call_stack[1] = 0;              // Value pointer
    alloc = create_allocator();
    value_pointer = 0;
    value = malloc(1024);

    for (;;)
    {
        // Get next state
        next_index = state * TABLE_WIDTH + lex->look.type * STATE_WIDTH;
        next_state = parser_table[next_index];
        next_commands = parser_table[next_index + 1];

        printf("%i -- %s(%i) --> %i ( %i ", state, 
            lex->look.type_name, lex->look.type, next_state, next_commands);

        // There was a syntax error
        if (next_state == -1)
        {
            lexer_error(lex, lex->look, 
                "Unexpected token");
            LEXER_NEXT(lex);

            break;
        }

        // Execute command and go to the next state
        ignore_flag = 0;
        state = next_state;
        EXEC_COMMAND(next_commands);
        if (!ignore_flag) LEXER_NEXT(lex);
        else printf("ignore ");
        printf(")\n");

        // If a return command has been called in the 
        // outer most scope, this is an accepting state
        if (call_stack_pointer <= 0)
            break;
    }
    free(call_stack);

    Document doc;
    doc.memory = alloc.memory;
    doc.root = value;
    return doc;
}

void tinyparse_free_document(
    Document *doc)
{
    free_memory_list(doc->memory);
    free(doc->root);
}

#endif // TINYPARSE_IMPLEMENT
