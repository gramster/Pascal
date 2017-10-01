//-----------------------------------------------------------------------
// namestore.h - Header file for the identifier name store class
//	of the Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#ifndef __NAMESTORE_H
#define __NAMESTORE_H

#define NUM_RESERVED_WORDS	64	// First 64 name indices are reserved
#define MAX_KEY				631	// Number of hash table entries
#define MAX_IDENTIFIERS		200
#define MAX_STORESPACE		(MAX_IDENTIFIERS * 15)

// Hash table linked list entry

class namestore_t; // forward declaration

class hash_entry_t
{
	int				index;
	hash_entry_t	*next;
	friend			namestore_t;
public:
};

class namestore_t
{
private:
	hash_entry_t *hash[MAX_KEY];	// hash table
	int		map[MAX_IDENTIFIERS-NUM_RESERVED_WORDS];// indices => store offsets
	char	store[MAX_STORESPACE];	// holds the actual names
	int		top;					// top of store
	int		next;					// next index to use
	const int Key(const char *name);
public:
	namestore_t();
	~namestore_t();
	const	int Index(char *name);
	int		Lookup(char *name);
	const	char* Name(const int index) const;
};

extern namestore_t *NameStore;

#endif

