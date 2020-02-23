#ifndef RULE_H
#define RULE_H

#include "tinylex.h"

#define FLAG_NOP        0b0000
#define FLAG_SET        0b0001
#define FLAG_PUSH_SUB   0b0010
#define FLAG_CALL       0b0100
#define FLAG_RETURN     0b1000

typedef struct _Command
{
    int flags;
    Token node;
    Token attr;
    int to_rule;
} Command;

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

    int start_index;
    Token name_token;
    RuleNode *root;
} Rule;

Rule rule_parse(LexerStream *lex);
void rule_free(Rule *rule);

#endif // RULE_H
