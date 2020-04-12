
void tr_entry(EntryNode *entry)
{
	LOG("Entry");
	START_SCOPE
	switch(entry->type)
	{
		case Test_Test: LOG("Test"); break;
		case Test_Func: LOG("Func"); break;
		case Test_For: LOG("For"); break;
		case Test_If: LOG("If"); break;
		case Test_TinyLex: LOG("TinyLex"); break;
	}
	END_SCOPE;
}

