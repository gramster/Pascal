//-----------------------------------------------------------------------
// namestore.cpp - identifier name store class of the Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#include <string.h>
#include <assert.h>

#include "error.h"
#include "namestore.h"

// Compute hash key for identifier

const int namestore_t::Key(const char *name)
{
	int sum=0;
	while (*name)
	   sum += *name++;
	return sum % MAX_KEY;
}

// Look up a name in the hash table; return index or 0 if not present

const int namestore_t::Index(char *name)
{
	strlwr(name);
	for (hash_entry_t *e = hash[Key(name)] ; e ; e = e->next)
		if (strcmp(name, Name(e->index)) == 0)
			return e->index;
	return 0;
}

// Look up a name in the name store, adding it if not yet present

int namestore_t::Lookup(char *name)
{
	int rtn = Index(name); // is it already present?
	if (rtn == 0) // no, this is a new name
	{
		if (next >= MAX_IDENTIFIERS)
			ErrorHandler->Fatal(ERR_MAXIDENTS);
		int key = Key(name);
		// prepend a new hash entry to list for this key
		hash_entry_t *e = new hash_entry_t;
		e->index = next;
		e->next = hash[key];
		hash[key] = e;
		map[next - NUM_RESERVED_WORDS] = top;
		while (*name) store[top++] = *name++;
		store[top++] = '\0';
		rtn = next++;
	}
	return rtn;
}

// Get a name given its index

const char *namestore_t::Name(const int index) const
{
	assert(index >= NUM_RESERVED_WORDS && index < next);
	return (const char *)(store+map[index - NUM_RESERVED_WORDS]);
}

// create a name store

namestore_t::namestore_t()
{
	for (int i = 0; i < MAX_KEY; i++)
		hash[i] = NULL;
	next = NUM_RESERVED_WORDS;
	top = 0;
}

// destroy a name store

namestore_t::~namestore_t()
{
	for (int i = 0; i < MAX_KEY; i++)
		while (hash[i])
		{
			hash_entry_t *e = hash[i];
			hash[i] = hash[i]->next;
			delete e;
		}
}


