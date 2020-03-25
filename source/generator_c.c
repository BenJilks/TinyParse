#include "generator.h"
#include "c_header.h"
#include "c_implement.h"
#include "FSM.h"
#include <ctype.h>

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

typedef int (*func_t)(int);

static char *str_convert(
    char *str, func_t func)
{
    int len, i;
    char *out;

    len = strlen(str);
    out = malloc(len + 1);
    out[len] = '\0';

    for (i = 0; i < len; i++)
        out[i] = func(str[i]);
    return out;
}

static char *to_lower(char *str) { return str_convert(str, tolower); }
static char *to_upper(char *str) { return str_convert(str, toupper); }

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
	char *name_lower;

	name_lower = to_lower(parser->project_name);
    fprintf(output, "\n");
	fprintf(output, "#define DEBUG_TABLE_NAME %s_debug_table\n", name_lower);
	fprintf(output, "#define PARSE_NAME	%s_parse\n", name_lower);
	fprintf(output, "#define FREE_DOCUMENT_NAME	%s_free_document\n", name_lower);
	free(name_lower);

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
    fputs(label_name, output);
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

static void generate_type_optional(
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

    generate_type_expression(output, node->child, 
        lex, parser, tab);
}

static void generate_type_sub_expression(
    FILE *output,
    RuleNode *node,
    LexerStream *lex,
    Parser *parser,
    int tab)
{
    OUT(output, tab, "struct \n");
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
        
        case RULE_OPTIONAL: generate_type_optional(
            output, node, lex, parser, tab); break;

        default: break;
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
        fprintf(output, "\nstruct _%sNode\n{\n", name);
        generate_type_expression(output, root, lex, parser, 1);
        fprintf(output, "};\n");
    });
}

static void generate_command_code(
    FILE *output,
    LexerStream *lex,
    Parser *parser,
    Command command)
{
    char node[80];
    char attr[80];
    lexer_read_string(lex, command.node, node);

    if (command.flags & FLAG_MARK_TYPE)
    {
        lexer_read_string(lex, command.attr, attr);
        fprintf(output, "((%sNode*)(value + value_pointer))->%s "
            "= lex->look.type;", node, attr);
        fprintf(output, "ignore_flag = 1;");

        fprintf(output, "printf(\"Mark type, \");");
    }

    if (command.flags & FLAG_SET_FLAG)
    {
        lexer_read_string(lex, command.attr, attr);
        fprintf(output, "((%sNode*)(value + value_pointer))->%s "
            "= 1;", node, attr);
        fprintf(output, "ignore_flag = 1;");

        fprintf(output, "printf(\"Set flag, \");");
    }

    if (command.flags & FLAG_UNSET_FLAG)
    {
        lexer_read_string(lex, command.attr, attr);
        fprintf(output, "((%sNode*)(value + value_pointer))->%s "
            "= 0;", node, attr);
        fprintf(output, "ignore_flag = 1;");

        fprintf(output, "printf(\"Unset flag, \");");
    }

    if (command.flags & FLAG_NULL)
    {
        fprintf(output, "ignore_flag = 1;");
    }

    if (command.flags & FLAG_CALL)
    {
        int to_state;

        to_state = parser->rules[command.to_rule].start_index;
        fprintf(output, "call_stack[call_stack_pointer++] = state;");
        fprintf(output, "call_stack[call_stack_pointer++] = value_pointer;");
        fprintf(output, "state = %i;", to_state);
        fprintf(output, "value_pointer += sizeof(%sNode);", node);
        fprintf(output, "ignore_flag = 1;");

        fprintf(output, "printf(\"Call %s, \");", 
            parser->rules[command.to_rule].name);
    }
    
    if (command.flags & FLAG_SET)
    {
        lexer_read_string(lex, command.attr, attr);
        fprintf(output, "((%sNode*)(value + value_pointer))->%s"
            " = lex->look;", node, attr);
    }

    if (command.flags & FLAG_PUSH_SUB)
    {
        lexer_read_string(lex, command.attr, attr);
        fprintf(output, "((%sNode*)(value + value_pointer))->%s = "
            "push(&alloc, value + value_pointer + sizeof(%sNode), sizeof(%sNode));",
            node, attr, node,
            parser->rules[command.to_rule].name); 
        fprintf(output, "ignore_flag = 1;");
    }

    if (command.flags & FLAG_RETURN)
    {
        fprintf(output, "value_pointer = call_stack[--call_stack_pointer];");
        fprintf(output, "state = call_stack[--call_stack_pointer];");
        fprintf(output, "ignore_flag = 1;");

        fputs("printf(\"Return to: %i, \", state);", output);
    }
}

static void generate_commands(
    FILE *output,
    LexerStream *lex,
    Parser *parser)
{
    Command command;
    int i;

    fprintf(output, "static int command_flags[] = {");
    for (i = 0; i < parser->command_count; i++)
    {
        command = parser->commands[i];
        fprintf(output, "%i, ", command.flags);
        fprintf(output, "%i, ", command.to_rule);
    }
    fprintf(output, "};\n");

    fprintf(output, "#define EXEC_COMMAND(command) \\\n");
    fprintf(output, "{ \\\n");
    fprintf(output, "\tswitch(command) \\\n");
    fprintf(output, "\t{ \\\n");
    for (i = 0; i < parser->command_count; i++)
    {
        fprintf(output, "\t\tcase %i: ", i);
        command = parser->commands[i];
        generate_command_code(output, lex, parser, command);

        fprintf(output, " break; \\\n");
    }
    fprintf(output, "\t} \\\n");
    fprintf(output, "}\n");
}

void generate_implement(
    FILE* output,
    LexerStream *lex,
    Parser *parser)
{
    int i, j;
    char *title_lower;

    title_lower = to_lower(parser->project_name);
    fprintf(output, "\n#define TABLE_WIDTH %i\n", parser->table_width);
    fprintf(output, "#define TABLE_SIZE %i\n", parser->table_size);
    fprintf(output, "#define ENTRY_POINT %i\n", parser->entry_index);
    fprintf(output, "#define LEXER_NEXT %s_next\n", title_lower);
    free(title_lower);

    fprintf(output, "\nstatic char parser_table[] = \n{\n");
    for (i = 0; i < parser->table_size; i++)
    {
        fprintf(output, "\t");
        for (j = 0; j < parser->table_width; j++)
        {
            fprintf(output, "%i, ", 
                parser->table[i * parser->table_width + j]);
        }
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
	char *name_upper;
    fputs(template_c_header, output);

    fprintf(output, "\n#ifndef TINYPARSER_H\n");
    fprintf(output, "#define TINYPARSER_H\n");
    generate_headers(output, parser);
    generate_data_types(output, lex, parser);
    fprintf(output, "\n#endif // TINYPARSER_H\n");

	name_upper = to_upper(parser->project_name);
    fprintf(output, "\n#ifdef %s_IMPLEMENT\n", name_upper);
    generate_commands(output, lex, parser);
    generate_implement(output, lex, parser);
    fprintf(output, "\n#endif // %s_IMPLEMENT\n", name_upper);
}
