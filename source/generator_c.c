#include "generator.h"
#include "c_header.h"
#include "c_implement.h"
#include "FSM.h"

#define FOR_EACH_RULE(item, name, do) \
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

#define OUT(output, tab, ...) \
{ \
    output_tabs(output, tab); \
    fprintf(output, __VA_ARGS__); \
}

static void output_tabs(
    FILE *output, 
    int tab)
{
    int i;
    for (i = 0; i < tab; i++)
        fprintf(output, "\t");
}

static void generate_headers(
    FILE *output, 
    Parser *parser)
{
    fprintf(output, "\n");
    FOR_EACH_RULE(rule, name,
    {
        fprintf(output, "typedef struct _%sNode %sNode;\n", 
            name, name);
    })
}

static void generate_label_definition(
    FILE *output, 
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    Token label;
    Rule *rule;
    char token_name[80];
    char label_name[80];

    label = node->label;
    lexer_read_string(lex, node->value, token_name);

    rule = parser_find_rule(parser, token_name);
    if (rule)
    {
        OUT(output, tab, "%sNode *", token_name);
    }
    else
    {
        OUT(output, tab, "Token ");
    }

    lexer_read_string(lex, label, label_name);
    fprintf(output, label_name);
    fprintf(output, ";\n");
}

static void generate_type_expression(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab);

static void generate_type_value(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    generate_label_definition(output, node, 
        lex, parser, tab);
}

static void generate_type_or(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    // If it's named, generate a type label
    if (node->has_label)
    {
        char label[80];

        lexer_read_string(lex, node->label, label);
        OUT(output, tab, "int %s;\n", label);
    }

    OUT(output, tab, "union\n");
    OUT(output, tab, "{\n");
    generate_type_expression(output, node->child, 
        lex, parser, tab + 1);
    OUT(output, tab, "};\n");
}

static void generate_type_sub_expression(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    OUT(output, tab, "struct __attribute__((__packed__))\n");
    OUT(output, tab, "{\n");
    generate_type_expression(output, node->child, 
        lex, parser, tab + 1);
    OUT(output, tab, "};\n");
}

static void generate_type_expression(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    switch (node->type)
    {
        case RULE_VALUE: generate_type_value(
            output, node, lex, parser, tab); break;

        case RULE_EXPRESSION: generate_type_sub_expression(
            output, node, lex, parser, tab); break;

        case RULE_OR: generate_type_or(
            output, node, lex, parser, tab); break;
    }

    if (node->next != NULL)
    {
        generate_type_expression(output, node->next, 
            lex, parser, tab);
    }
}

static void generate_data_types(
    FILE *output, 
    LexerStream *lex,
    Parser *parser)
{
    RuleNode *root;

    FOR_EACH_RULE(rule, name,
    {
        root = rule.root;
        fprintf(output, "\nstruct __attribute__((__packed__)) _%sNode\n{\n", name);
        generate_type_expression(output, root, lex, parser, 1);
        fprintf(output, "};\n");
    });
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

    fprintf(output, "\n#define TABLE_WIDTH %i\n", parser->table_width);
    fprintf(output, "#define ENTRY_POINT %i\n", parser->entry_index);

    fprintf(output, "\nstatic char table[] = \n{\n");
    for (i = 0; i < parser->table_size; i++)
    {
        fprintf(output, "\t");
        for (j = 0; j < parser->table_width; j++)
            fprintf(output, "%i, ", parser->table[i * parser->table_width + j]);
        fprintf(output, "\n");
    }
    fprintf(output, "};\n");

    fputs(template_c_implement, output);
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
