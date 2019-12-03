#ifndef RULE_H
#define RULE_H

#include "tinylex.h"

typedef enum _RuleType
{
    RULE_KEYWORD,
    RULE_VALUE,
    RULE_EXPRESSION
} RuleType;

typedef struct _RuleNode
{
    struct _RuleNode *child;
    struct _RuleNode *next;

    Token value, label;
    RuleType type;
} RuleNode;

typedef struct _Rule
{
    char name[80];
    
    RuleNode *root;
    RuleNode **labels;
    int label_buffer_size;
    int label_count;
} Rule;

Rule rule_parse(LexerStream *lex);
void rule_free(Rule *rule);

#endif // RULE_H
