
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

typedef struct _FunctionNode FunctionNode;
typedef struct _BlockNode BlockNode;
typedef struct _MultiBlockNode MultiBlockNode;
typedef struct _StatementNode StatementNode;
typedef struct _ArgumentsNode ArgumentsNode;

struct _FunctionNode
{
	struct 
	{
		Token name;
		ArgumentsNode *args;
		BlockNode *block;
	};
};

struct _BlockNode
{
	struct 
	{
		int type;
		union
		{
			StatementNode *statement;
			struct 
			{
				MultiBlockNode *multi_block;
			};
		};
	};
};

struct _MultiBlockNode
{
	struct 
	{
		StatementNode *statement;
		int has_next;
		MultiBlockNode *next;
	};
};

struct _StatementNode
{
	struct 
	{
		Token test;
	};
};

struct _ArgumentsNode
{
	struct 
	{
		Token arg;
		int has_next;
		struct 
		{
			ArgumentsNode *next;
		};
	};
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT
#define EXEC_COMMAND(command) \
{ \
	switch(command) \
	{ \
		case 0: ((FunctionNode*)(value + value_pointer))->name = lex->look; break; \
		case 1: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 29;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call Arguments, "); break; \
		case 2: ((FunctionNode*)(value + value_pointer))->args = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(ArgumentsNode));ignore_flag = 1; break; \
		case 3: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 10;value_pointer += sizeof(FunctionNode);ignore_flag = 1;printf("Call Block, "); break; \
		case 4: ((FunctionNode*)(value + value_pointer))->block = push(&alloc, value + value_pointer + sizeof(FunctionNode), sizeof(BlockNode));ignore_flag = 1; break; \
		case 5: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 6: ((BlockNode*)(value + value_pointer))->type = lex->look.type;ignore_flag = 1;printf("Mark type, "); break; \
		case 7: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 26;value_pointer += sizeof(BlockNode);ignore_flag = 1;printf("Call Statement, "); break; \
		case 8: ((BlockNode*)(value + value_pointer))->statement = push(&alloc, value + value_pointer + sizeof(BlockNode), sizeof(StatementNode));ignore_flag = 1; break; \
		case 9: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 18;value_pointer += sizeof(BlockNode);ignore_flag = 1;printf("Call MultiBlock, "); break; \
		case 10: ((BlockNode*)(value + value_pointer))->multi_block = push(&alloc, value + value_pointer + sizeof(BlockNode), sizeof(MultiBlockNode));ignore_flag = 1; break; \
		case 11: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 12: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 26;value_pointer += sizeof(MultiBlockNode);ignore_flag = 1;printf("Call Statement, "); break; \
		case 13: ((MultiBlockNode*)(value + value_pointer))->statement = push(&alloc, value + value_pointer + sizeof(MultiBlockNode), sizeof(StatementNode));ignore_flag = 1; break; \
		case 14: ((MultiBlockNode*)(value + value_pointer))->has_next = 1;ignore_flag = 1;printf("Set flag, "); break; \
		case 15: ((MultiBlockNode*)(value + value_pointer))->has_next = 0;ignore_flag = 1;printf("Unset flag, "); break; \
		case 16: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 18;value_pointer += sizeof(MultiBlockNode);ignore_flag = 1;printf("Call MultiBlock, "); break; \
		case 17: ((MultiBlockNode*)(value + value_pointer))->next = push(&alloc, value + value_pointer + sizeof(MultiBlockNode), sizeof(MultiBlockNode));ignore_flag = 1; break; \
		case 18: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 19: ((StatementNode*)(value + value_pointer))->test = lex->look; break; \
		case 20: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
		case 21: ((ArgumentsNode*)(value + value_pointer))->arg = lex->look; break; \
		case 22: ((ArgumentsNode*)(value + value_pointer))->has_next = 1;ignore_flag = 1;printf("Set flag, "); break; \
		case 23: ((ArgumentsNode*)(value + value_pointer))->has_next = 0;ignore_flag = 1;printf("Unset flag, "); break; \
		case 24: call_stack[call_stack_pointer++] = state;call_stack[call_stack_pointer++] = value_pointer;state = 29;value_pointer += sizeof(ArgumentsNode);ignore_flag = 1;printf("Call Arguments, "); break; \
		case 25: ((ArgumentsNode*)(value + value_pointer))->next = push(&alloc, value + value_pointer + sizeof(ArgumentsNode), sizeof(ArgumentsNode));ignore_flag = 1; break; \
		case 26: value_pointer = call_stack[--call_stack_pointer];state = call_stack[--call_stack_pointer];ignore_flag = 1;printf("Return to: %i, ", state); break; \
	} \
}

#define TABLE_WIDTH 16
#define TABLE_SIZE 37
#define ENTRY_POINT 0

static char parser_table[] = 
{
	1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 
	5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 6, -1, 5, 2, 
	7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 7, 3, 
	8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 
	9, 5, 9, 5, 9, 5, 9, 5, 9, 5, 9, 5, 9, 5, 9, 5, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	11, 6, 11, 6, 11, 6, 11, 6, 11, 6, 11, 6, 11, 6, 11, 6, 
	11, 6, 12, 7, 11, 6, 14, -1, 11, 6, 11, 6, 11, 6, 11, 6, 
	13, 8, 13, 8, 13, 8, 13, 8, 13, 8, 13, 8, 13, 8, 13, 8, 
	17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 
	-1, -1, 15, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	16, 10, 16, 10, 16, 10, 16, 10, 16, 10, 16, 10, 16, 10, 16, 10, 
	17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 17, 11, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, 19, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	20, 13, 20, 13, 20, 13, 20, 13, 20, 13, 20, 13, 20, 13, 20, 13, 
	21, 14, 21, 14, 21, 14, 21, 14, 21, 14, 21, 14, 21, 14, 21, 14, 
	22, 15, 23, 16, 22, 15, 22, 15, 22, 15, 22, 15, 22, 15, 22, 15, 
	25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 
	24, 17, 24, 17, 24, 17, 24, 17, 24, 17, 24, 17, 24, 17, 24, 17, 
	25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 25, 18, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, 27, 19, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	28, 20, 28, 20, 28, 20, 28, 20, 28, 20, 28, 20, 28, 20, 28, 20, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, 30, 21, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	31, 22, 31, 22, 31, 22, 31, 22, 31, 22, 31, 22, 31, 22, 31, 22, 
	32, 23, 32, 23, 32, 23, 32, 23, 32, 23, 32, 23, 32, 23, 33, -1, 
	36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 
	-1, -1, -1, -1, 34, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	35, 25, 35, 25, 35, 25, 35, 25, 35, 25, 35, 25, 35, 25, 35, 25, 
	36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 36, 26, 
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
#define ALLOCATION_BUFFER 80

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
        printf("Realloc ");
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
    printf("Alloc: %i, ", size);

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
            testproject_next(lex);

            break;
        }

        // Execute command and go to the next state
        ignore_flag = 0;
        state = next_state;
        EXEC_COMMAND(next_commands);
        if (!ignore_flag) testproject_next(lex);
        else printf("ignore ");
        printf(")\n");

        // If a return command has been called in the 
        // outer most scope, this is an accepting state
        if (call_stack_pointer <= 0)
            break;
        
        printf("Value pointer: %i\n", value_pointer);
        printf("Call stack: ");
        for (int i = 0; i < call_stack_pointer; i++)
            printf("%i ", call_stack[i]);
        printf("\n\n");
    }
    printf("Accepted!!!\n");
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
