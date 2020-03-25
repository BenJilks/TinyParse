
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

void DEBUG_TABLE_NAME();
Document PARSE_NAME(LexerStream *lex);
void FREE_DOCUMENT_NAME(Document *doc);

#endif // TINYPARSE_H

#ifndef TINYPARSER_H
#define TINYPARSER_H

#define DEBUG_TABLE_NAME testproject_debug_table
#define PARSE_NAME	testproject_parse
#define FREE_DOCUMENT_NAME	testproject_free_document
typedef struct _AddOpNode AddOpNode;
typedef struct _MulOpNode MulOpNode;
typedef struct _TermNode TermNode;
typedef struct _FactorNode FactorNode;
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

struct _MulOpNode
{
	struct 
	{
		int type;
		union
		{
		};
	};
};

struct _TermNode
{
	struct 
	{
		int type;
		union
		{
			Token i;
			Token f;
			struct 
			{
				FunctionNode *sub;
			};
		};
	};
};

struct _FactorNode
{
	struct 
	{
		TermNode *left;
		int has_next;
		struct 
		{
			MulOpNode *op;
			FactorNode *next;
		};
	};
};

struct _FunctionNode
{
	struct 
	{
		FactorNode *left;
		int has_next;
		struct 
		{
			AddOpNode *op;
			FunctionNode *next;
		};
	};
};

#endif // TINYPARSER_H

#ifdef TESTPROJECT_IMPLEMENT
static int command_flags[] = {16, 4357344, 8, 0, 16, 4357344, 8, 0, 1, -19120, 1, -19120, 4, 4, 2, 4, 16, 0, 8, 0, 4, 2, 2, 2, 32, 2123744, 64, 2, 4, 1, 2, 1, 4, 3, 2, 3, 8, 4234722, 4, 3, 2, 3, 32, 2123744, 64, 2, 4, 0, 2, 0, 4, 4, 2, 4, 8, 4234722, };
#define EXEC_COMMAND(command) \
{ \
	switch(command) \
	{ \
		case 0: ((AddOpNode*)(value + value_pointer))->type = lex->look.type;ignore_flag = 1;printf("Mark type, "); break; \
		case 1: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 2: ((MulOpNode*)(value + value_pointer))->type = lex->look.type;ignore_flag = 1;printf("Mark type, "); break; \
		case 3: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 4: ((TermNode*)(value + value_pointer))->i = lex->look; break; \
		case 5: ((TermNode*)(value + value_pointer))->f = lex->look; break; \
		case 6: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 29;value_pointer += sizeof(TermNode);ignore_flag = 1;printf("Call Function, "); break; \
		case 7: ((TermNode*)(value + value_pointer))->sub = push(&alloc, value + value_pointer + sizeof(TermNode), sizeof(FunctionNode));ignore_flag = 1; break; \
		case 8: ((TermNode*)(value + value_pointer))->type = lex->look.type;ignore_flag = 1;printf("Mark type, "); break; \
		case 9: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 10: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 10;value_pointer += sizeof(FactorNode);ignore_flag = 1;printf("Call Term, "); break; \
		case 11: ((FactorNode*)(value + value_pointer))->left = push(&alloc, value + value_pointer + sizeof(FactorNode), sizeof(TermNode));ignore_flag = 1; break; \
		case 12: ((FactorNode*)(value + value_pointer))->has_next = 1;ignore_flag = 1;printf("Set flag, "); break; \
		case 13: ((FactorNode*)(value + value_pointer))->has_next = 0;ignore_flag = 1;printf("Unset flag, "); break; \
		case 14: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 5;value_pointer += sizeof(FactorNode);ignore_flag = 1;printf("Call MulOp, "); break; \
		case 15: ((FactorNode*)(value + value_pointer))->op = push(&alloc, value + value_pointer + sizeof(FactorNode), sizeof(MulOpNode));ignore_flag = 1; break; \
		case 16: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 19;value_pointer += sizeof(FactorNode);ignore_flag = 1;printf("Call Factor, "); break; \
		case 17: ((FactorNode*)(value + value_pointer))->next = push(&alloc, value + value_pointer + sizeof(FactorNode), sizeof(FactorNode));ignore_flag = 1; break; \
		case 18: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 19: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 19;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call Factor, "); break; \
		case 20: ((FunctionNode*)(value + value_pointer))->left = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(FactorNode));ignore_flag = 1; break; \
		case 21: ((FunctionNode*)(value + value_pointer))->has_next = 1;ignore_flag = 1;printf("Set flag, "); break; \
		case 22: ((FunctionNode*)(value + value_pointer))->has_next = 0;ignore_flag = 1;printf("Unset flag, "); break; \
		case 23: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 0;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call AddOp, "); break; \
		case 24: ((FunctionNode*)(value + value_pointer))->op = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(AddOpNode));ignore_flag = 1; break; \
		case 25: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 29;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call Function, "); break; \
		case 26: ((FunctionNode*)(value + value_pointer))->next = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(FunctionNode));ignore_flag = 1; break; \
		case 27: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
	} \
}

#define TABLE_WIDTH 16
#define TABLE_SIZE 39
#define ENTRY_POINT 29
#define LEXER_NEXT testproject_next

static char parser_table[] = 
{
	-1, -1, -1, -1, 3, 0, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, 
	4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 
	4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 
	-1, -1, -1, -1, 1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, 8, 2, 8, 2, -1, -1, -1, -1, 
	9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 
	9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 9, 3, 
	-1, -1, -1, -1, -1, -1, -1, -1, 6, -1, 7, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	17, 8, 17, 8, -1, -1, -1, -1, -1, -1, -1, -1, 17, 8, -1, -1, 
	18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 
	18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 
	14, 6, 14, 6, -1, -1, -1, -1, -1, -1, -1, -1, 14, 6, -1, -1, 
	15, 7, 15, 7, 15, 7, 15, 7, 15, 7, 15, 7, 15, 7, 15, 7, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16, -1, 
	18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 18, 9, 
	11, 4, 12, 5, -1, -1, -1, -1, -1, -1, -1, -1, 13, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	20, 10, 20, 10, -1, -1, -1, -1, -1, -1, -1, -1, 20, 10, -1, -1, 
	21, 11, 21, 11, 21, 11, 21, 11, 21, 11, 21, 11, 21, 11, 21, 11, 
	22, 12, 22, 12, 22, 12, 22, 12, 22, 12, 22, 12, 22, 12, 22, 12, 
	23, 13, 23, 13, 23, 13, 23, 13, 24, 14, 24, 14, 23, 13, 23, 13, 
	28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 
	25, 15, 25, 15, 25, 15, 25, 15, 25, 15, 25, 15, 25, 15, 25, 15, 
	26, 16, 26, 16, -1, -1, -1, -1, -1, -1, -1, -1, 26, 16, -1, -1, 
	27, 17, 27, 17, 27, 17, 27, 17, 27, 17, 27, 17, 27, 17, 27, 17, 
	28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 28, 18, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	30, 19, 30, 19, -1, -1, -1, -1, -1, -1, -1, -1, 30, 19, -1, -1, 
	31, 20, 31, 20, 31, 20, 31, 20, 31, 20, 31, 20, 31, 20, 31, 20, 
	32, 21, 32, 21, 32, 21, 32, 21, 32, 21, 32, 21, 32, 21, 32, 21, 
	33, 22, 33, 22, 34, 23, 34, 23, 33, 22, 33, 22, 33, 22, 33, 22, 
	38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 
	35, 24, 35, 24, 35, 24, 35, 24, 35, 24, 35, 24, 35, 24, 35, 24, 
	36, 25, 36, 25, -1, -1, -1, -1, -1, -1, -1, -1, 36, 25, -1, -1, 
	37, 26, 37, 26, 37, 26, 37, 26, 37, 26, 37, 26, 37, 26, 37, 26, 
	38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 38, 27, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
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

void DUBUG_TABLE_NAME()
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

Document PARSE_NAME(
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

void FREE_DOCUMENT_NAME(
    Document *doc)
{
    free_memory_list(doc->memory);
    free(doc->root);
}

#endif // TESTPROJECT_IMPLEMENT
