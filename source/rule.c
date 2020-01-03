#include "rule.h"
#include "debug.h"
#include <stdlib.h>

#define RULE_BUFFER_SIZE 80

static RuleNode *parse_keyword(
    LexerStream *lex)
{
    Token keyword;
    RuleNode *node;

    // Create a new node
    node = malloc(sizeof(RuleNode));
    node->type = RULE_KEYWORD;
    node->child = NULL;
    node->next = NULL;

    // Set node value
    keyword = tinyparse_next(lex);
    node->value = keyword;

    return node;
}

static RuleNode *parse_token(
    Rule *rule,
    LexerStream *lex)
{
    RuleNode *node;

    // Create new node
    node = malloc(sizeof(RuleNode));
    node->type = RULE_VALUE;
    node->child = NULL;
    node->next = NULL;

    // Parse rule with label
    tinyparse_match(lex, TinyParse_OpenNode, "<");
    node->value = tinyparse_next(lex);
    tinyparse_match(lex, TinyParse_Of, ":");
    node->label = tinyparse_next(lex);
    tinyparse_match(lex, TinyParse_CloseNode, ">");

    // Add to label list
    rule->labels[rule->label_count] = node;
    rule->label_count += 1;
    if (rule->label_count >= rule->label_buffer_size)
    {
        rule->label_buffer_size += RULE_BUFFER_SIZE;
        rule->labels = realloc(rule->labels, 
            rule->label_buffer_size * sizeof(RuleNode*));
    }

    return node;
}

static RuleNode *parse_node(
    Rule *rule,
    LexerStream *lex)
{
    switch (lex->look.type)
    {
        case TinyParse_Name: return parse_keyword(lex);
        case TinyParse_OpenNode: return parse_token(rule, lex);
    }

    lexer_error(lex, lex->look, "Unexpected token");
    tinyparse_next(lex);
    return NULL;
}

static RuleNode *parse_expression(
    Rule *rule,
    LexerStream *lex)
{
    RuleNode *node;
    RuleNode *current, *next;

    // Create a new node
    node = malloc(sizeof(RuleNode));
    node->type = RULE_EXPRESSION;
    node->next = NULL;
    node->child = NULL;
    current = NULL;

    tinyparse_match(lex, TinyParse_Open, "(");
    while (lex->look.type != TinyParse_Close && !lex->eof_flag)
    {
        // Parse next node
        next = parse_node(rule, lex);
        if (!next)
            break;

        // If it's the first node, make it the child, 
        // otherwise make it the currents next
        if (current == NULL)
            node->child = next;
        else
            current->next = next;
        
        // Make this the current
        current = next;
    }
    tinyparse_match(lex, TinyParse_Close, ")");

    return node;
}

static void free_node(
    RuleNode *node)
{
    if (node->next != NULL) free_node(node->next);
    if (node->child != NULL) free_node(node->child);
    free(node);
}

Rule rule_parse(
    LexerStream *lex)
{
    Token name;
    Rule rule;

    name = tinyparse_match(lex, TinyParse_Name, "Name");
    lexer_read_string(lex, name, rule.name);
    LOG("Parsing rule '%s'\n", rule.name);

    rule.label_count = 0;
    rule.label_buffer_size = RULE_BUFFER_SIZE;
    rule.labels = malloc(rule.label_buffer_size * sizeof(RuleNode*));
    rule.root = parse_expression(&rule, lex);

    return rule;
}

void rule_free(
    Rule *rule)
{
    free(rule->labels);
    free_node(rule->root);
    rule->label_count = 0;
    rule->label_buffer_size = 0;
}
