#include "tinylex_intergration.h"
#include "debug.h"

#define TINYLEXINTERGRATION_IMPLEMENT
#include "tinylex_intergration_lexer.h"

void parse_tinylex(
	Parser *parser, 
	const char *file_path)
{
	LexerStream lex;
	
	lex = lexer_stream_open(file_path);
	tinylexintergration_init(&lex);
	while (!lex.eof_flag)
	{
		Token name_tk;
		Token value_tk;
		char name[80], value[80];

		// Read the next name-value pair
		name_tk = tinylexintergration_next(&lex);
		value_tk = tinylexintergration_next(&lex);
		lexer_read_string(&lex, name_tk, name);
		lexer_read_string(&lex, value_tk, value);
		LOG("%s(%s) => %s(%s)\n", 
			name, name_tk.type_name, 
			value, value_tk.type_name);

		// Ignore the project name
		if (!strcmp(name, "project"))
			continue;

		if (value_tk.type == TinyLexIntergration_Name)
		{
			char alias[80];
			int i, alias_len;

			alias_len = 0;
			for (i = 0; i < strlen(value); i++)
			{
				if (value[i] == '\\' && i < strlen(value) - 1)
				{
					if (value[i+1] != '(' && value[i+1] != ')')
						i += 1;
				}
				alias[alias_len] = value[i];
				alias_len += 1;
			}
			alias[alias_len] = '\0';
			parser_add_token(parser, alias, name);
		}
		else
		{
			parser_add_token(parser, name, name);
		}
	}
}

