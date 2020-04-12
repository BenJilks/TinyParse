
void tr_keywords(KeywordsNode *kw)
{
	LOG("Keywords");
}

void tr_or(OrNode *or)
{
	LOG("Or");
	START_SCOPE;
	LOG("%s", type_names[or->type]);
	END_SCOPE;
}

void tr_optional(OptionalNode *op)
{
	LOG("Optional");
	START_SCOPE;
	LOG("%i", op->has_func);
	END_SCOPE;
}

void tr_entry(EntryNode *entry)
{
	LOG("Entry");
	
	START_SCOPE;
	tr_keywords(entry->keywords);
	tr_or(entry->or_a);
	tr_or(entry->or_b);
	tr_optional(entry->optional_a);
	tr_optional(entry->optional_b);
	END_SCOPE;
}

