#ifndef __SYMTAB_H
#define __SYMTAB_H

#include <stdlib.h> 	// for NULL
#include "pcode.h"

#define MAX_LEVELS		16

class symtab_entry_t;

extern symtab_entry_t
	*unknown_type,
	*integer_type,
	*boolean_type,
	*true_literal,
	*false_literal,
	*read_procedure,
	*write_procedure;

extern int no_name, write_proc_idx, read_proc_idx;

typedef enum
{
	CONSTANT, STANDARDTYPE, ARRAYTYPE, RECORDTYPE, FIELD, VARIABLE,
	VALUEPARAM, REFPARAM, PROCEDURE, STANDARDPROC, UNDEFINED
} entry_class_t;

class symtab_entry_t
{
private:
	symtab_entry_t	*type;
	symtab_entry_t	*next;
	entry_class_t	_class;
	int				index;
	friend symtab_t;
public:
	symtab_entry_t(entry_class_t class_in, int idx,
		symtab_entry_t *type_in = unknown_type)
	{
		_class = class_in;
		index = idx;
		next = NULL;
		type = type_in;
	}
	entry_class_t Class()			{ return _class;	}
	int Index()						{ return index;		}
	symtab_entry_t *Next()			{ return next;		}
	symtab_entry_t *Type()			{ return type;		}
	void Type(symtab_entry_t *t)	{ type = t; 		}
	int Size();
	void AllocateAddress(entry_class_t cls, int offset);
};

//---------------------------------------------------------------
// Specialisations of symbol table entries

class standardtype_t : public symtab_entry_t
{
public:
	standardtype_t(int idx)
		: symtab_entry_t(STANDARDTYPE, idx)
	{ }
};

class constant_t : public symtab_entry_t
{
private:
	int             value;
public:
	constant_t(int idx, symtab_entry_t *typ, int val)
		: symtab_entry_t(CONSTANT, idx, typ)
	{
		value = val;
	}
	int Value()						{ return value;		}
};

class variable_t : public symtab_entry_t
{
private:
	int	level, displ;
public:
	variable_t(int idx, symtab_entry_t *typ = unknown_type)
		: symtab_entry_t(VARIABLE, idx, typ)
	{ }
	void AllocateAddress(entry_class_t cls, int offset);
	int Level()						{ return level;		}
	int Displ()						{ return displ;		}
};

class valueparam_t : public symtab_entry_t
{
private:
	int	level, displ;
public:
	valueparam_t(int idx, symtab_entry_t *typ = unknown_type)
		: symtab_entry_t(VALUEPARAM, idx, typ)
	{ }
	void AllocateAddress(entry_class_t cls, int offset);
	int Level()						{ return level;		}
	int Displ()						{ return displ;		}
};

class refparam_t : public symtab_entry_t
{
private:
	int	level, displ;
public:
	refparam_t(int idx, symtab_entry_t *typ = unknown_type)
		: symtab_entry_t(REFPARAM, idx, typ)
	{ }
	void AllocateAddress(entry_class_t cls, int offset);
	int Level()						{ return level;		}
	int Displ()						{ return displ;		}
};

class arraytype_t : public symtab_entry_t
{
private:
	int             lower, upper;
	symtab_entry_t *indextype;
public:
	arraytype_t(int index_in, symtab_entry_t *ityp,  symtab_entry_t *etyp,
		int bottom, int top)
		: symtab_entry_t(ARRAYTYPE, index_in, etyp)
	{
		lower = bottom;
		upper = top;
		indextype = ityp;
	}
	symtab_entry_t *IndexType()			{ return indextype;			}
	symtab_entry_t *ElementType()		{ return Type();			}
	int Lower()							{ return lower;				}
	int Upper()							{ return upper;				}
	int Length()						{ return upper - lower + 1;	}
	int Size()
	{
		return Length() * ElementType()->Size();
	}
};

class recordtype_t : public symtab_entry_t
{
private:
	symtab_entry_t *lastfield;
	int				size;
public:
	recordtype_t(int index_in, int size_in, symtab_entry_t *lastfield_in)
		: symtab_entry_t(RECORDTYPE, index_in)
	{
		lastfield = lastfield_in;
		size = size_in;
	}
	symtab_entry_t *LastField()			{ return lastfield;		}
	int Size()							{ return size;			}
};

class fieldtype_t : public symtab_entry_t
{
private:
	int displ;
public:
	fieldtype_t(int index_in, symtab_entry_t *type_in)
		: symtab_entry_t(FIELD, index_in, type_in)
	{ }
	void AllocateAddress(entry_class_t cls, int offset);
	int Displ()						{ return displ;		}
};

class procedure_t : public symtab_entry_t
{
private:
	symtab_entry_t *lastparam;
	int label;
	int level;
public:
	procedure_t(int index_in, int label_in,
		symtab_entry_t *lastparam_in = NULL);
	symtab_entry_t *LastParam()			{ return lastparam;		}
	void LastParam(symtab_entry_t *lp)	{ lastparam = lp;		}
	int Label()							{ return label;			}
	int Level()							{ return level;			}
};

class standardproc_t : public symtab_entry_t
{
public:
	standardproc_t(int index_in)
		: symtab_entry_t(STANDARDPROC, index_in)
	{
	}
};

//-------------------------------------------------------------------
// The Symbol Table

class symtab_t
{
private:
	symtab_entry_t		*scope_tbl[MAX_LEVELS];
	int					tempsize[MAX_LEVELS];
	int					maxtempsize[MAX_LEVELS];
	int					level;
public:
	symtab_t();
	~symtab_t();
	void NewBlock(symtab_entry_t *last = NULL);
	void ExitBlock();
	symtab_entry_t *IsDefined(int index_in, int lvl);
  symtab_entry_t *Define(symtab_entry_t *e);
	symtab_entry_t *Define(int index_in, entry_class_t _class_in);
	symtab_entry_t *Find(int index_in);
	fieldtype_t *FindField(recordtype_t *rt, int id);
	void Push(int cnt);
	void Pop(int cnt);
	int Level()
	{
		return level;
	}
	symtab_entry_t *Last()
	{
		return scope_tbl[level];
	}
	int MaxTemp()
	{
		return maxtempsize[level];
	}
};

extern symtab_t *SymTab;

#endif
