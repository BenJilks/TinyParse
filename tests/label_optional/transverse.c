
void tr_entry(EntryNode *entry)
{
	LOG("Entry");
	START_SCOPE;
	LOG("A: %i", entry->has_a);
	LOG("B: %i", entry->has_b);
	END_SCOPE;
}

