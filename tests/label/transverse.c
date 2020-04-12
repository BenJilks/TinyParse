
void tr_entry(EntryNode *entry)
{
	char str[80];

	LOG("Entry");

	START_SCOPE;
	lexer_read_string(&lex, entry->test, str);
	LOG("%s", str);
	
	lexer_read_string(&lex, entry->func, str);
	LOG("%s", str);
	END_SCOPE;
}

