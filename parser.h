#ifndef __PARSER_H
#define __PARSER_H

#include "pcode.h"
#include "symtab.h"

//---------------------------------------------------------------
// Token set types (still in C; it's more terse)
//---------------------------------------------------------------

#define MAX_SET_ELTS	(T_UNDEFINED+1)
#define MAX_SET_LONGS	((MAX_SET_ELTS+31)/32)

typedef struct
{
	unsigned long v[MAX_SET_LONGS];
} set_t;

//----------------------------------------------------------------

class parser_t
{
private:
	symtab_entry_t *CheckTypes(symtab_entry_t *t1, symtab_entry_t *t2);
	void TypeError(symtab_entry_t *e, char *class_name);
	void Synchronise(set_t stop);
	void Expect(token_t t, set_t stop);
	void Block(int vars, int temps, int start, pword_t scopeindex, set_t stop);
	void ConstantDefinitions(set_t stop);
	void ConstantDefinition(set_t stop);
	void TypeDefinitions(set_t stop);
	void TypeDefinition(set_t stop);
	void NewType(int id, set_t stop);
	void NewArrayType(int id, set_t stop);
	symtab_entry_t *IndexRange(int &lower, int &upper, set_t stop);
	void NewRecordType(int id, set_t stop);
	symtab_entry_t *FieldList(int &size, set_t stop);
	symtab_entry_t *RecordSection(symtab_entry_t* &last, int &cnt, set_t stop);
	int VariableDefinitions(set_t stop);
	symtab_entry_t *VariableDefinition(symtab_entry_t* &last, int &cnt, set_t stop);
	symtab_entry_t *TypeID(set_t stop);
	symtab_entry_t *VariableGroup(entry_class_t cls,
		symtab_entry_t* &last, int &cnt, set_t stop);
	void ProcedureDefinition(set_t stop);
	void ProcedureBlock(procedure_t *p, set_t stop);
	symtab_entry_t *FormalParameters(int &size, set_t stop);
	symtab_entry_t *ParameterDefinition(int &size, set_t stop);
	void Statement(set_t stop);
	void AssignmentStmt(set_t stop);
	void ProcedureStmt(set_t stop);
	void StandardProcStmt(set_t stop);
	int  ActualParameters(symtab_entry_t *last_param, set_t stop);
	void IfStmt(set_t stop);
	void WhileStmt(set_t stop);
	void CompoundStmt(set_t stop);
	symtab_entry_t *Expression(set_t stop);
	symtab_entry_t *SimpleExpr(set_t stop);
	symtab_entry_t *Term(set_t stop);
	symtab_entry_t *Factor(set_t stop);
	symtab_entry_t *VariableAccess(set_t stop);
	symtab_entry_t *Selector(symtab_entry_t *type, set_t stop);
	symtab_entry_t *IndexSelector(arraytype_t *array_type, set_t stop);
	symtab_entry_t *FieldSelector(recordtype_t *record_type, set_t stop);
	void Constant(int &val, symtab_entry_t* &type, set_t stop);
	int	 Ident(set_t stop);
public:
	void Program();
	void StandardBlock();
};

extern parser_t *Parser;

#endif

