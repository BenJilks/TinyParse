
int count = 0;

void tr_entry(EntryNode *entry)
{
	count += 1;
	if (entry->has_next)
		tr_entry(entry->next);
	else
		LOG("Count: %i", count);
}

