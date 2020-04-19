
void tr_entry(EntryNode *entry)
{
	LOG("Entry");
	START_SCOPE
	switch(entry->type)
	{
		case NODE_A: LOG("A"); break;
		case NODE_B: LOG("B"); break;
		case NODE_C: LOG("C"); break;
		case NODE_D: LOG("D"); break;
	}
	END_SCOPE;
}

