#include <stdio.h>
#include <getopt.h>
#include "parser.h"
#include "generator.h"

#define TINYLEX_IMPLEMENT
#define TINYPARSE_IMPLEMENT
#include "tinylex.h"

static struct option cmd_options[] = 
{
    {"out",         required_argument,  0,  'o'},
    {"entry",       required_argument,  0,  'e'},
    {"language",    required_argument,  0,  'l'},
    {"help",        no_argument,        0,  'h'},
};

void show_use()
{
    printf("Usage: tinyparse [options] file...\n");
    printf("Options: \n");
    printf("\t-h/--help\t\t\tShow command help information\n");
    printf("\t-e/--entry <node>\t\tSet the entry point to start parsing at\n");
    printf("\t-l/--language <c,cpp>\t\tSet the output language\n");
    printf("\t-o/--out <file>\t\t\tOutput code into <file>\n");
}

int main(int argc, char *argv[])
{
    char **input_files;
    int input_file_count;

    FILE *output_file;
    int use_output_file;

    char *lang = "c";
    char *entry = NULL;

    // Read arguments
    input_file_count = 0;
    output_file = stdout;
    use_output_file = 0;
    for (;;)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "o:e:l:h", 
            cmd_options, &option_index);
        if (c == -1)
            break;
        
        switch (c)
        {
            // Output file
            case 'o':
                output_file = fopen(optarg, "wb");
                use_output_file = 1;
                break;

            case 'e':
                entry = optarg;
                break;

            case 'l':
                lang = optarg;
                break;
            
            // Display usage
            case 'h':
                show_use();
                return 0;
        }
    }

    // Read input files
    input_files = argv + optind;
    input_file_count = argc - optind;

    if (input_file_count <= 0)
    {
        printf("Error: No input files given\n");
        printf("Try 'tinyparse --help' for more information\n");
        return -1;
    }

    // Check that an entry point was given
    if (entry == NULL)
    {
        printf("Error: No entry point given, please specify one using '-e <node>'\n");
        printf("Type 'tinyparse --help' for more information\n");
        return -1;
    }

    LexerStream lex;
    Parser parser;
    parser = parser_create();

    lex = lexer_stream_open(input_files[0]);
    tinyparse_init(&lex);
    parser_parse(&parser, &lex);
    if (parser_compile_and_link(&parser, &lex, entry) == 0)
        generate_c(output_file, &lex, &parser);

    parser_free(&parser);
    lexer_stream_close(&lex);
    if (use_output_file)
        fclose(output_file);
    return 0;
}
