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
    node->has_label = 0;

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
    node->has_label = 1;

    // Parse rule with label
    tinyparse_match(lex, TinyParse_OpenNode, "<");
    node->value = tinyparse_next(lex);
    tinyparse_match(lex, TinyParse_Of, ":");
    node->label = tinyparse_next(lex);
    tinyparse_match(lex, TinyParse_CloseNode, ">");

    return node;
}

static RuleNode *parse_expression(
    Rule *rule,
    LexerStream *lex);

static RuleNode *parse_node(
    Rule *rule,
    LexerStream *lex)
{
    switch (lex->look.type)
    {
        case TinyParse_Name: 
        case TinyParse_Symbol: return parse_keyword(lex);
        case TinyParse_OpenNode: return parse_token(rule, lex);
        case TinyParse_Open: return parse_expression(rule, lex);
    }

    lexer_error(lex, lex->look, "Unexpected token");
    tinyparse_next(lex);
    return NULL;
}

static RuleNode *parse_or(
    Rule *rule,
    LexerStream *lex,
    RuleNode *left)
{
    RuleNode *node;
    Token name;

    // Parse each branch in the operation
    node = malloc(sizeof(RuleNode));
    node->next = NULL;
    node->child = left;
    node->type = RULE_OR;
    node->has_label = 0;
    while (lex->look.type == TinyParse_Or)
    {
        tinyparse_next(lex);
        left->next = parse_node(rule, lex);
        left = left->next;
    }

    // Parse a name if it has one
    if (lex->look.type == TinyParse_Of)
    {
        tinyparse_match(lex, TinyParse_Of, ":");
        name = tinyparse_match(lex, TinyParse_Name, "Or Name");
        node->label = name;
        node->has_label = 1;
    }

    return node;
}

static RuleNode *parse_optional(
    Rule *rule,
    LexerStream *lex,
    RuleNode *left)
{
    RuleNode *node;

    tinyparse_match(lex, TinyParse_Optional, "?");
    node = malloc(sizeof(RuleNode));
    node->next = NULL;
    node->child = left;
    node->type = RULE_OPTIONAL;
    node->has_label = 0;

    if (lex->look.type == TinyParse_Of)
    {
        tinyparse_match(lex, TinyParse_Of, ":");
        node->label = tinyparse_match(lex, TinyParse_Name, "Label");
        node->has_label = 1;
    }
    return node;
}

static RuleNode *parse_term(
    Rule *rule,
    LexerStream *lex)
{
    RuleNode *node;

    node = parse_node(rule, lex);
    switch (lex->look.type)
    {
        case TinyParse_Or: return parse_or(rule, lex, node);
        case TinyParse_Optional: return parse_optional(rule, lex, node);
        default: return node;
    }
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
        next = parse_term(rule, lex);
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

    rule.name_token = name;
    rule.root = parse_expression(&rule, lex);
    return rule;
}

void rule_free(
    Rule *rule)
{
    free_node(rule->root);
}
