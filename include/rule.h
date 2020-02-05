#ifndef RULE_H
#define RULE_H

#include "tinylex.h"

typedef enum _RuleType
{
    RULE_KEYWORD,
    RULE_VALUE,
    RULE_EXPRESSION,
    RULE_OR
} RuleType;

typedef struct _RuleNode
{
    struct _RuleNode *child;
    struct _RuleNode *next;

    Token value, label;
    RuleType type;
    int has_label;
} RuleNode;

typedef struct _Rule
{
    char name[80];
    RuleNode *root;
} Rule;

Rule rule_parse(LexerStream *lex);
void rule_free(Rule *rule);

#endif // RULE_H
