#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

void generate_c(FILE *output, 
    LexerStream *lex, Parser *parser);

#endif // GENERATOR_H
