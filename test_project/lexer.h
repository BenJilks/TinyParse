
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

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
    int eof_flag, len;

    int cache_start, cache_end;
    char *cache;
    Token look;
} LexerStream;

LexerStream lexer_stream_open(const char *file_path);
void lexer_stream_close(LexerStream *stream);
char lexer_read_char(LexerStream *lex, int index);
char *lexer_read_buffer(LexerStream *lex, int index, int len);
void lexer_read_string(LexerStream *lex, Token token, char *out);
void lexer_error(LexerStream *lex, Token token, const char *error);

#endif // LEXER_H

#ifdef TINYLEX_IMPLEMENT

#define CACHE_SIZE      1024
#define RED             "\033[1;31m"
#define ORANGE          "\033[01;33m"
#define WHITE           "\033[37m"
#define RESET           "\033[0m"

#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)

static void populate_cache(
    LexerStream *lex)
{
    int len;

    len = lex->cache_end - lex->cache_start;
    fseek(lex->file, lex->cache_start, SEEK_SET);
    fread(lex->cache, sizeof(char), len, lex->file);

//    printf("!!! Populated cache !!!\n");
}

char lexer_read_char(
    LexerStream *lex, 
    int index)
{
    if (index >= lex->len)
        return EOF;

    if (index < lex->cache_start 
        || index > lex->cache_end)
    {
        lex->cache_start = MAX(index - CACHE_SIZE/2, 0);
        lex->cache_end = MIN(lex->cache_start + CACHE_SIZE, lex->len);
        populate_cache(lex);
    }

    return lex->cache[index - lex->cache_start];
}

char *lexer_read_buffer(
    LexerStream *lex,
    int index, 
    int len)
{
    if (len > CACHE_SIZE)
    {
        printf("Error: Token too large to access\n");
        return NULL;
    }

    // If the buffer is not within the cache, 
    // reposition it so it is
    if (index < lex->cache_start 
        || index + len > lex->cache_end)
    {
        lex->cache_start = MAX(index - CACHE_SIZE/2 - len/2, 0);
        lex->cache_end = MIN(lex->cache_start + CACHE_SIZE, lex->len);
        populate_cache(lex);
    }

    return lex->cache + index - lex->cache_start;
}

static void read_len(
    LexerStream *lex)
{
    fseek(lex->file, 0L, SEEK_END);
    lex->len = ftell(lex->file);
    rewind(lex->file);
}

LexerStream lexer_stream_open(
    const char *file_path)
{
    LexerStream stream;
    stream.file = fopen(file_path, "rb");
    if (!stream.file)
    {
        printf(RED "Error: could not open file '%s'\n", 
            file_path);
        
        stream.eof_flag = 1;
        stream.cache = NULL;
        return stream;
    }

    stream.eof_flag = 0;
    stream.index = 0;
    stream.line_index = 0;
    stream.column = 0;
    stream.line = 0;
    read_len(&stream);

    stream.cache_start = 0;
    stream.cache_end = CACHE_SIZE;
    stream.cache = (char*)malloc(CACHE_SIZE);
    populate_cache(&stream);

    return stream;
}

void lexer_stream_close(
    LexerStream *stream)
{
    if (stream->file)
        fclose(stream->file);
    
    if (stream->cache)
        free(stream->cache);
}

void lexer_read_string(
    LexerStream *lex, 
    Token token, 
    char *out)
{
    char *buffer;

    buffer = lexer_read_buffer(lex, 
        token.data_index, token.len);
    memcpy(out, buffer, token.len);
    out[token.len] = '\0';
}

char *lexer_read_line(
    LexerStream *lex, 
    int index, 
    int *len)
{
    char c;

    *len = 0;
    do
    {
        c = lexer_read_char(lex, index + *len);
        *len += 1;
    } while (c != '\n' && c != EOF);
    *len -= 1;
    
    return lexer_read_buffer(lex, index, *len);
}

void lexer_error(
    LexerStream *lex,
    Token token,
    const char *error)
{
    int len, i;
    char *line;

    printf(RED "Error(%i:%i): %s\n", 
        token.line_no + 1, token.column_no,
        error);

    line = lexer_read_line(lex, token.line_index, &len);
	printf("\t" WHITE);
    fwrite(line, sizeof(char), len, stdout);

    printf("\n\t");
    for (i = 0; i < token.column_no; i++)
        printf(" ");

    printf(ORANGE);
    for (i = 0; i < token.len; i++)
        printf("~");
    printf("\n" RESET);
}

#endif

#ifndef TESTPROJECT_H
#define TESTPROJECT_H

typedef enum _TestProjectTokenType
{
	TestProject_Int = 0,
	TestProject_Add,
	TestProject_Subtract,
} TestProjectTokenType;

void testproject_init(LexerStream *lex);
Token testproject_next(LexerStream *lex);
Token testproject_match(LexerStream *lex, char type, const char *name);

#endif

#ifdef TESTPROJECT_IMPLEMENT

#define LEXER_NAME testproject
#define INIT_NAME testproject_init
#define NEXT_MATCH_NAME testproject_next_match
#define NEXT_NAME testproject_next
#define MATCH_NAME testproject_match
	#define NONE (1 << (sizeof(char) * 8)) - 1

	static char end_states[] = { -1, 0, 0, -1, 1, -1, 2,  };
	static unsigned char table[] = 
	{
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 4, 255, 6, 255, 255, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 4, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	};

	#define STATE_COUNT 7

	#define TOKEN_COUNT 3
	static const char *type_names[] = { "Int", "Add", "Subtract",  };

typedef enum _MatchStatus
{
    MATCH,
    BLANK
} MatchStatus;

void INIT_NAME(
    LexerStream *lex)
{
    if (!lex->eof_flag)
        NEXT_NAME(lex);
}

#define CHAR_COUNT 128

static MatchStatus NEXT_MATCH_NAME(
    LexerStream *lex)
{
    Token *token = &lex->look;
    unsigned int state, next_state;
    char next_char, type;

    // Assign token info
    token->data_index = lex->index;
    token->line_index = lex->line_index;
    token->line_no = lex->line;
    token->column_no = lex->column;
    token->len = 0;

    state = 0;
    for (;;)
    {
        next_char = lexer_read_char(lex, lex->index);
        if (next_char == -1)
            break;

        next_state = table[state * CHAR_COUNT + next_char];
//        printf("%u -%c-> %u\n", state, next_char, next_state);

        lex->index += 1;
        lex->column += 1;
        if (next_char == '\n')
        {
            lex->line_index = lex->index;
            lex->line += 1;
            lex->column = 0;
        }

        if (next_state == NONE)
        {
            // If the end state is not valid, 
            // there's no match
            type = end_states[state];
//            printf("Ending: %i -> %i\n", state, type);
            if (type == -1)
                break;

            // Otherwise there is one, so return so
            token->type_name = type_names[type];
            token->type = type;
            lex->index -= 1;
            lex->column -= 1;
            return MATCH;
        }

        state = next_state;
        token->len += 1;
    }

    // If there's nothing left, mark so
    if (lexer_read_char(lex, lex->index) == EOF)
        lex->eof_flag = 1;

    // If no types match
    return BLANK;
}

Token NEXT_NAME(
    LexerStream *lex)
{
    Token last = lex->look;

    // Find the next valid token
    MatchStatus status;
    do
    {
        status = NEXT_MATCH_NAME(lex);
    } while (status != MATCH && !lex->eof_flag);

    return last;
}

Token MATCH_NAME(
    LexerStream *lex, 
    char type, 
    const char *name)
{
    char token_data[80];
    char msg[128];
    Token look;

    // Check token type info
    look = lex->look;
    if (look.type != type)
    {
        memcpy(token_data, lexer_read_buffer(
            lex, look.data_index, look.len), 
            look.len);
        token_data[look.len] = '\0';

        sprintf(msg, "Expected '%s', got '%s' instead", 
            name, token_data);

        lexer_error(lex, lex->look, msg);
    }

    // Get the next token
    return NEXT_NAME(lex);
}

#endif
