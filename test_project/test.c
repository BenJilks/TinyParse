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

void create_visual(
    const char *file_path)
{
    FILE *file;
    int i, x, y;
    int state_distance = 300;

    file = fopen(file_path, "w");
    fprintf(file, "<svg width=\"10000\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    
    x = 100;
    y = 350;
    for (i = 0; i < TABLE_SIZE; i++)
    {
        char name[80];
        int j;

        sprintf(name, "S%i", i);
        draw_circle(file, x, y, 50);
        draw_text(file, x, y, name);

        int groups[TABLE_SIZE][TABLE_WIDTH];
        int group_len[TABLE_SIZE];
        memset(group_len, 0, TABLE_SIZE * sizeof(int));
        for (j = 0; j < TABLE_WIDTH; j += 2)
        {
            int to, command;

            to = parser_table[i * TABLE_WIDTH + j];
            command = parser_table[i * TABLE_WIDTH + j + 1];
            if (to != -1)
            {
                groups[to][group_len[to]] = j/2;
                groups[to][group_len[to]+1] = command;
                group_len[to] += 2;

                if (command != -1 && command_flags[command*2] & FLAG_CALL)
                {
                    int call_to;

                    call_to = command_flags[command*2+1];
                    groups[call_to][group_len[call_to]] = j/2;
                    group_len[call_to] += 2;
                }
            }
        }

        for (j = 0; j < TABLE_SIZE; j++)
        {
            int to, len;
            int top, gap;

            to = j;
            len = group_len[j];
            if (len >= 1)
            {
                char label[1024];
                int k, to_x, to_y, arc;
                char *ptr;

                if (len == TOKEN_COUNT)
                {
                    sprintf(label, "*");
                }
                else
                {
                    ptr = label;
                    for (k = 0; k < len; k += 2)
                    {
                        sprintf(ptr, "%s", type_names[groups[to][k]]); ptr += strlen(ptr);
                        if (k < len - 2)
                        {
                            sprintf(ptr, ", "); 
                            ptr += strlen(ptr);
                        }
                    }
                }

                top = j % 2 != 0;
                gap = abs(j - i);
                to_x = 100 + (state_distance * to);
                to_y = y + (top? -50 : 50);
                arc = (top? 1 : -1) * gap * 100 + gap * 40;
                draw_path(file, 
                    x, to_y, 
                    to_x, to_y, 
                    arc);
                
                draw_text(file, 
                    ((to_x - x) / 2) + x, 
                    to_y - arc/2 - (top ? 1 : -1)*10,
                    label);
            }
        }

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
//    doc = tinyparse_parse(&lex);

//    tinyparse_free_document(&doc);
    lexer_stream_close(&lex);

    return 0;
}
