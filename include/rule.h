#ifndef RULE_H
#define RULE_H

#include "tinylex.h"

#define FLAG_NOP            0b0000000000
#define FLAG_SET            0b0000000001
#define FLAG_PUSH_SUB       0b0000000010
#define FLAG_CALL           0b0000000100
#define FLAG_RETURN         0b0000001000
#define FLAG_MARK_TYPE      0b0000010000
#define FLAG_MARK_NODE_TYPE 0b0000100000
#define FLAG_SET_FLAG       0b0001000000
#define FLAG_UNSET_FLAG     0b0010000000
#define FLAG_PEEK           0b0100000000
#define FLAG_NULL           0b1000000000

typedef struct _Peek
{
    int count;
    int to_state;
    int command_id;
    int token_type;
} Peek;

typedef struct _Command
{
    int flags;
    Token node;
    Token attr;
    int to_rule;

    Peek peeks[80];
    int peek_count;
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
