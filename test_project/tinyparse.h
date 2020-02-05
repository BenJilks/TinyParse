
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
typedef struct _BlockNode BlockNode;
typedef struct _StatementNode StatementNode;

struct __attribute__((__packed__)) _FunctionNode
{
	struct __attribute__((__packed__))
	{
		Token name;
		BlockNode *block;
	};
};

struct __attribute__((__packed__)) _BlockNode
{
	struct __attribute__((__packed__))
	{
		int type;
		union
		{
			StatementNode *statement;
			struct __attribute__((__packed__))
			{
				StatementNode *multi_block;
			};
		};
	};
};

struct __attribute__((__packed__)) _StatementNode
{
	struct __attribute__((__packed__))
	{
	};
};

#endif // TINYPARSER_H

#ifdef TINYPARSE_IMPLEMENT

#define TABLE_WIDTH 15
#define ENTRY_POINT 0

static char table[] = 
{
	1, 0, -1, 10, 28, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, 2, 32, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 21, 3, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 6, 1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 8, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, 10, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
};

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if 0
#include "c_header.txt"
#define ENTRY_POINT 0
#define TABLE_WIDTH 0
#define testproject_next(...) ;
static char table[] = {};
#endif


#define STATE_WIDTH 3

#define ALLOCATION_BUFFER 80

#define COMMAND_NOP         0b000000
#define COMMAND_PUSH        0b100000
#define COMMAND_CALL        0b010000
#define COMMAND_RETURN      0b001000
#define COMMAND_PUSH_SUB    0b000100
#define COMMAND_PADDING     0b000010
#define COMMAND_MARK_TYPE   0b000001

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

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

Document tinyparse_parse(
    LexerStream *lex)
{
    // State
    int state;
    int next_state, next_commands;
    int next_arg, next_index;
    int call_command, value_stack_pointer;

    // Stacks
    Allocator alloc, value_stack;
    int call_stack[80];
    int call_stack_pointer;
    void *value;

    state = ENTRY_POINT;
    call_stack_pointer = 3;
    call_stack[0] = 0;              // Return state
    call_stack[1] = COMMAND_PUSH;   // Command on return
    call_stack[2] = 0;              // Value stack pointer
    alloc = create_allocator();
    value_stack = create_allocator();

    while (!lex->eof_flag)
    {
        // Get next state
        next_index = state * TABLE_WIDTH + lex->look.type * STATE_WIDTH;
        next_state = table[next_index];
        next_commands = table[next_index + 1];
        next_arg = table[next_index + 2];
        printf("%i -- %s --> %i \t\t{ "BYTE_TO_BINARY_PATTERN" %i }\n", state, 
            lex->look.type_name, next_state, BYTE_TO_BINARY(next_commands), next_arg);

        // There was a syntax error
        if (next_state == -1)
        {
            lexer_error(lex, lex->look, 
                "Unexpected token");
            testproject_next(lex);

            break;
        }

        // Mark type
        if (next_commands & COMMAND_MARK_TYPE)
        {
            printf("Marking type: %s\n", lex->look.type_name);
            push(&value_stack, (void*)&lex->look.type, sizeof(int));
        }

        // Set next state and run command
        if (next_commands & COMMAND_CALL) 
        {
            call_stack[call_stack_pointer] = next_arg;
            call_stack[call_stack_pointer + 1] = 0;
            call_stack[call_stack_pointer + 2] = value_stack.memory_pointer;
            if (next_commands & COMMAND_PUSH_SUB)
            {
                printf("Push sub node %s\n", lex->look.type_name);
                call_stack[call_stack_pointer + 1] |= COMMAND_PUSH;
            }

            call_stack_pointer += 3;
        }

        // Add padding
        if (next_commands & COMMAND_PADDING)
        {
            printf("Add padding: %i\n", next_arg);
            check_buffer_size(&value_stack, next_arg);
            value_stack.memory_pointer += next_arg;
        }

        if (next_commands & COMMAND_PUSH) 
        {
            printf("Push %s\n", lex->look.type_name);
            push(&value_stack, (void*)&lex->look, sizeof(Token));
        }

        if (next_commands & COMMAND_RETURN) 
        {
            value_stack_pointer = call_stack[--call_stack_pointer];
            call_command = call_stack[--call_stack_pointer];
            next_state = call_stack[--call_stack_pointer];
            if (call_command & COMMAND_PUSH)
            {
                printf("Push sub node now of size %i\n", value_stack.memory_pointer - 
                    value_stack_pointer);

                value = push(&alloc, value_stack.memory + 
                    value_stack_pointer, value_stack.memory_pointer - 
                    value_stack_pointer);
                value_stack.memory_pointer = value_stack_pointer;
                push(&value_stack, &value, sizeof(void*));
            }
        }
        
        state = next_state;
        testproject_next(lex);

        // If a return command has been called in the 
        // outer most scope, this is an accepting state
        if (call_stack_pointer <= 0)
            break;
    }

    printf("Accepted!!!\n");
    free_allocator(&value_stack);

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
