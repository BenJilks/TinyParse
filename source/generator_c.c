#include "generator.h"
#include "c_header.h"
#include "FSM.h"

#define FOR_EACH_RULE(item, do) \
{ \
    int __rule_index; \
    char *name; \
    Rule item; \
     \
    for (__rule_index = 0; __rule_index < parser->rule_count; __rule_index++) \
    { \
        item = parser->rules[__rule_index]; \
        name = rule.name; \
        do \
    } \
}

static void generate_headers(
    FILE *output, 
    Parser *parser)
{
    FOR_EACH_RULE(rule, 
    {
        fprintf(output, "\ntypedef struct _%sNode %sNode;\n", name, name);
        fprintf(output, "%sNode *parse_%s(LexerStream *lex);\n", 
            name, name);
    })
}

static void generate_label_definition(
    FILE *output, 
    RuleNode *node,
    LexerStream *lex,
    Parser *parser)
{
    Token label;
    Rule *rule;
    char token_name[80];

    label = node->label;
    lexer_read_string(lex, node->value, token_name);

    rule = parser_find_rule(parser, token_name);
    if (rule)
        fprintf(output, "\t%sNode *", token_name);
    else
        fprintf(output, "\tToken ");

    fwrite(lexer_read_buffer(lex, label.data_index, label.len), 
        sizeof(char), label.len, output);
    fprintf(output, ";\n");
}

static void generate_data_types(
    FILE *output, 
    LexerStream *lex,
    Parser *parser)
{
    RuleNode *node;
    int i;

    FOR_EACH_RULE(rule, 
    {
        fprintf(output, "\nstruct _%sNode\n", name);
        fprintf(output, "{\n");
        for (i = 0; i < rule.label_count; i++)
        {
            node = rule.labels[i];
            generate_label_definition(
                output, node, lex, parser);
        }
        fprintf(output, "};\n");
    })
}

static void generate_keyword(
    FILE *output, 
    RuleNode *node, 
    LexerStream *lex,
    Parser *parser,
    int indent)
{
    int i;
    char name[80];
    Token name_token;

    name_token = node->value;
    lexer_read_string(lex, name_token, name);

    for (i = 0; i < indent + 1; i++)
        fprintf(output, "\t");
    
    fprintf(output, "%s_match(%s_%s, \"%s\");\n",
        parser->project_name, parser->project_name,
        name, name);
}

void generate_implement(
    FILE* output,
    LexerStream *lex,
    Parser *parser)
{
    int i, j;

    for (i = 0; i < parser->table_size; i++)
    {
        printf("%i: ", i);
        for (j = 0; j < parser->table_width; j++)
            printf("%i, ", parser->table[i * parser->table_width + j]);
        printf("\n");
    }
}

void generate_c(
    FILE *output, 
    LexerStream *lex,
    Parser *parser)
{
    fputs(template_c_header, output);

    fprintf(output, "\n#ifndef TINYPARSER_H\n");
    fprintf(output, "#define TINYPARSER_H\n");
    generate_headers(output, parser);
    generate_data_types(output, lex, parser);
    fprintf(output, "\n#endif // TINYPARSER_H\n");

    fprintf(output, "\n#ifdef TINYPARSE_IMPLEMENT\n");
    generate_implement(output, lex, parser);
    fprintf(output, "\n#endif // TINYPARSE_IMPLEMENT\n");
}
