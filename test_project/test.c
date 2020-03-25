#include <stdio.h>
#define TINYLEX_IMPLEMENT
#define TINYPARSE_IMPLEMENT
#define TESTPROJECT_IMPLEMENT
#include "lexer.h"
#include "parser.h"

void draw_circle(
    FILE *file,
    int x, int y,
    int rad)
{
    fprintf(file, "<circle cx=\"%i\" cy=\"%i\" r=\"%i\" "
        "stroke=\"black\" stroke-width=\"2\" fill=\"white\"/>\n",
        x, y, rad);
}

void draw_text(
    FILE *file, 
    int x, int y, 
    const char *text)
{
    fprintf(file, "<text x=\"%i\" y=\"%i\" "
        "dominant-baseline=\"middle\" "
        "text-anchor=\"middle\">%s</text>\n",
        x, y, text);
}

void draw_path(
    FILE *file,
    int from_x, int from_y,
    int to_x, int to_y,
    int curve)
{
    fprintf(file, "<path d=\"M%i %i q %i %i %i %i\" "
        "fill=\"none\" stroke=\"black\" stroke-width=\"2\" />",
        from_x, from_y, 
        (to_x - from_x) / 2, -curve,
        to_x - from_x, to_y - from_y);
}

void find_groups(
    int state_from, 
    int *groups, 
    int *group_len)
{
    int i;

	memset(group_len, 0, TABLE_SIZE * sizeof(int));
    for (i = 0; i < TABLE_WIDTH; i += 2)
    {
        int to, command;

        to = parser_table[state_from * TABLE_WIDTH + i];
        command = parser_table[state_from * TABLE_WIDTH + i + 1];
        if (to != -1)
        {
            groups[to * TABLE_WIDTH + group_len[to]] = i/2;
            groups[to * TABLE_WIDTH + group_len[to]+1] = command;
            group_len[to] += 2;

            if (command != -1 && command_flags[command*2] & FLAG_CALL)
            {
                int call_to;

                call_to = command_flags[command*2+1];
                groups[call_to * TABLE_WIDTH + group_len[call_to]] = i/2;
                groups[call_to * TABLE_WIDTH + group_len[call_to]+1] = -1;
                group_len[call_to] += 2;
            }
        }
    }
}

#define APPEND(label, ...) \
    { sprintf(label, __VA_ARGS__); label += strlen(label); }

#define PRINT_FLAG(flag, name) \
{ \
    if (command_flags[command*2] & flag) \
    { \
        if (!is_first) APPEND(label, ", "); \
        APPEND(label, name); \
        is_first = 0; \
    } \
}

void print_command(
    char *label, 
    int command)
{
    int is_first;

    APPEND(label, " [");
    is_first = 1;

    PRINT_FLAG(FLAG_CALL, "Call");
    PRINT_FLAG(FLAG_MARK_TYPE, "Mark-Type");
    PRINT_FLAG(FLAG_PUSH_SUB, "Push-Sub");
    PRINT_FLAG(FLAG_RETURN, "Return");
    PRINT_FLAG(FLAG_SET, "Set");
    PRINT_FLAG(FLAG_SET_FLAG, "Set-Flag");
    PRINT_FLAG(FLAG_UNSET_FLAG, "Unset-Flag");
    PRINT_FLAG(FLAG_NULL, "Null");
    PRINT_FLAG(FLAG_NULL, "No-Op");
    APPEND(label, "]");
}

void make_label(
    char *label,
    int len, int to,
    int *groups)
{
    int i;

    for (i = 0; i < len; i += 2)
    {
        int token;
        int command;

        token = groups[to * TABLE_WIDTH + i];
        command = groups[to * TABLE_WIDTH + i + 1];
        APPEND(label, "%s", type_names[token]);

        if (command != -1)
        {
            print_command(label, command);
            label += strlen(label);
        }
        
        if (i < len - 2)
            APPEND(label, ", ");
    }
}

void draw_transition(
    FILE* file,
    char *label,
    int top,
    int x, int y,
    int state_from,
    int state_to,
    int state_distance)
{
    int to_x, to_y;
    int arc, gap;

    gap = abs(state_to - state_from);
    to_x = 100 + (state_distance * state_to);
    to_y = y + (top? -50 : 50);
    arc = (top? 1 : -1) * gap * 70 + gap * 20;
    draw_path(file, 
        x, to_y, 
        to_x, to_y, 
        arc);
    
    draw_text(file, 
        ((to_x - x) / 2) + x, 
        to_y - arc/2 - (top ? 1 : -1)*10,
        label);
}

void draw_groups(
    FILE *file, 
    int *groups, 
    int *group_len, 
    int x, int y, 
    int state_index,
    int state_distance)
{
    int i;

    for (i = 0; i < TABLE_SIZE; i++)
    {
        int to, len;

        to = i;
        len = group_len[i];
        if (len >= 1)
        {
            char label[1024];
            int to_x, to_y, arc;
            int top;

            top = (i % 2 != 0);
            make_label(label, len, to, groups);
            draw_transition(file, label, top, x, y, 
                state_index, i, state_distance);
        }
    }

}

void create_visual(
    const char *file_path)
{
    FILE *file;
    int i, x, y;
    int state_distance = 300;

    file = fopen(file_path, "w");
    fprintf(file, "<svg width=\"10000\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    
    x = 100;
    y = 500;
    for (i = 0; i < TABLE_SIZE; i++)
    {
        char name[80];

        sprintf(name, "S%i", i);
        draw_circle(file, x, y, 50);
        draw_text(file, x, y, name);

        int groups[TABLE_SIZE * TABLE_WIDTH];
        int group_len[TABLE_SIZE];
        find_groups(i, groups, group_len);
        draw_groups(file, groups, group_len, 
            x, y, i, state_distance);
        
        x += state_distance;
    }

    fprintf(file, "</svg>\n");

    fclose(file);
}

int main()
{
    LexerStream lex;
    Document doc;

    create_visual("test.svg");
    lex = lexer_stream_open("test.txt");
    testproject_init(&lex);
    doc = testproject_parse(&lex);

	testproject_free_document(&doc);
    lexer_stream_close(&lex);

    return 0;
}
