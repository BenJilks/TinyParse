#ifndef RULE_H
#define RULE_H

#include "tinylex.h"

#define FLAG_NOP        0b00000000
#define FLAG_SET        0b00000001
#define FLAG_PUSH_SUB   0b00000010
#define FLAG_CALL       0b00000100
#define FLAG_RETURN     0b00001000
#define FLAG_MARK_TYPE  0b00010000
#define FLAG_SET_FLAG   0b00100000
#define FLAG_UNSET_FLAG 0b01000000
#define FLAG_NULL       0b10000000

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
    RULE_OR,
    RULE_OPTIONAL
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
