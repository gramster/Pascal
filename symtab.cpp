#include <assert.h>
#include "error.h"
#include "namestore.h"
#include "symtab.h"

symtab_t::symtab_t()
{
	level = -1;
	NewBlock();
}

symtab_t::~symtab_t()
{
}

void symtab_t::NewBlock(symtab_entry_t *se)
{
	level++;
	assert(level < MAX_LEVELS); // should be error, not assert
	scope_tbl[level] = se;
	tempsize[level] = maxtempsize[level] = 0;
}

void symtab_t::ExitBlock()
{
	symtab_entry_t *e;
	assert(level>0);
	level--;
}

void symtab_t::Push(int cnt)
{
	tempsize[level] += cnt;
	if (tempsize[level] > maxtempsize[level])
		maxtempsize[level] = tempsize[level];
}

void symtab_t::Pop(int cnt)
{
	tempsize[level] -= cnt;
}

symtab_entry_t* symtab_t::IsDefined(int index_in, int lvl)
{
	for (symtab_entry_t *e = scope_tbl[lvl] ; e ; e = e->next)
	{
		if (e->index == index_in)
			return e;
	}
	return NULL;
}

symtab_entry_t *symtab_t::Define(symtab_entry_t *e)
{
	assert(e);
	e->next = scope_tbl[level];
	scope_tbl[level] = e;
	return e;
}

symtab_entry_t *symtab_t::Define(int index_in, entry_class_t _class_in)
{
	if (!IsDefined(index_in, level))
	{
		symtab_entry_t *e = new symtab_entry_t(_class_in, index_in);
		e->next = scope_tbl[level];
		scope_tbl[level] = e;
		return e;
	}
	ErrorHandler->Error(ERR_REDEFINE, NameStore->Name(index_in));
	return NULL;
}

symtab_entry_t *symtab_t::Find(int index_in)
{
	symtab_entry_t *rtn;
	int lvl = level;
	while (lvl>=0)
	{
		if ((rtn = IsDefined(index_in, lvl)) != NULL)
			return rtn;
		lvl--;
	}
	ErrorHandler->Error(ERR_UNDEFINED, NameStore->Name(index_in));
	return Define(index_in, UNDEFINED); // so we don't report it undefined again
}

fieldtype_t *symtab_t::FindField(recordtype_t *rt, int id)
{
	for (symtab_entry_t *f = rt->LastField() ; f ; f = f->Next() )
	{
		assert(f->Class() == FIELD);
		if (f->Index() == id)
			return (fieldtype_t *)f;
	}
	return NULL;
}

int symtab_entry_t::Size()
{
	if (Class() == STANDARDTYPE)
		return 1;
	else if (Class() == ARRAYTYPE)
		return ((arraytype_t *)this)->Size();
	else
	{
		assert(Class() == RECORDTYPE);
		return ((recordtype_t *)this)->Size();
	}
}

void symtab_entry_t::AllocateAddress(entry_class_t cls, int offset)
{
	switch(Class())
	{
		case VARIABLE:
			if (cls == VARIABLE)
				((variable_t *)this)->AllocateAddress(cls, offset);
		case VALUEPARAM:
			if (cls == VALUEPARAM)
				((valueparam_t *)this)->AllocateAddress(cls, offset);
		case REFPARAM:
			if (cls == VALUEPARAM)
				((refparam_t *)this)->AllocateAddress(cls, offset);
		case FIELD:
			if (cls == FIELD)
				((fieldtype_t *)this)->AllocateAddress(cls, offset);
	}
}

void variable_t::AllocateAddress(entry_class_t cls, int offset)
{
	level = SymTab->Level();
	displ = offset - Type()->Size();
	Next()->AllocateAddress(cls, displ);
}

void valueparam_t::AllocateAddress(entry_class_t cls, int offset)
{
	level = SymTab->Level();
	displ = offset - Type()->Size();
	Next()->AllocateAddress(cls, displ);
}

void refparam_t::AllocateAddress(entry_class_t cls, int offset)
{
	level = SymTab->Level();
	displ = offset - 1;
	Next()->AllocateAddress(cls, displ);
}

void fieldtype_t::AllocateAddress(entry_class_t cls, int offset)
{
	displ = offset - Type()->Size();
	Next()->AllocateAddress(cls, displ);
}

procedure_t::procedure_t(int index_in, int label_in,
		symtab_entry_t *lastparam_in)
	: symtab_entry_t(PROCEDURE, index_in)
{
	lastparam = lastparam_in;
	level = SymTab->Level();
	label = label_in;
}

