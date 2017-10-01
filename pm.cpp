#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "error.h"
#include "namestore.h"
#include "lex.h"
#include "parser.h"
#include "symtab.h"
#include "code.h"
#include "pmachine.h"
#include "debugger.h"

error_handler_t *ErrorHandler;
tokeniser_t *Lex;
namestore_t *NameStore;
parser_t *Parser;
symtab_t *SymTab;

symtab_entry_t
	*unknown_type,
	*integer_type,
	*boolean_type,
	*true_literal,
	*false_literal,
	*read_procedure,
	*write_procedure;

int no_name, write_proc_idx, read_proc_idx;

static void useage()
{
	fprintf(stderr,"Useage: pm [-r] [-d] <filename>\n");
	exit(-1);
}

main(int argc, char *argv[])
{
	int run = 0, debug = 0;
	for (int i = 1; i < (argc-1); i++)
	{
		if (argv[i][0]=='-' && argv[i][2]==0)
			switch(argv[i][1])
			{
			case 'r':
				run = 1;
				break;
			case 'd':
				debug = 1;
				break;
			default:
				useage();
			}
		else useage();
	}
	char *ifname;
	if (i == (argc-1)) ifname = argv[i];
	else useage();
	ErrorHandler = new error_handler_t;
	assert(ErrorHandler);
	NameStore = new namestore_t;
	assert(NameStore);
	Lex = new tokeniser_t(ifname);
	assert(Lex);
	Lex->NextToken();
	Parser = new parser_t;
	assert(Parser);
	SymTab = new symtab_t;
	assert(SymTab);
	Code = new codespace_t;
	assert(Code);
	if (debug)
	{
		Debugger = new debugger_t(ifname);
		assert(Debugger);
	}
	else Debugger = NULL;
	// define predefined types, constants and procedures
	Parser->StandardBlock();
	// parse the program
	Parser->Program();
	// should hit the end-of-file at end of parse
	if (Lex->Token() != T_EOF)
		ErrorHandler->Error(ERR_TRAILJUNK);
	// did we succeed?
	if (ErrorHandler->Errors())
	{
		fprintf(stderr,"Compilation failed: %d errors and %d warnings\n",
			ErrorHandler->Errors(),
			ErrorHandler->Warnings());
		run = 0;
	}
	else
	{
		// yes, so optimise and link and then save the code
		Code->Link();
		Code->Write("pm.cod");
		fprintf(stderr,"Compilation succeeded: %d warnings\n",
			ErrorHandler->Warnings());
	}
	delete Code;
	if (run)
	{
		pmachine_t *tim = new pmachine_t;
		tim->LoadAndGo("pm.cod");
		delete tim;
	}
	// delete stuff
	if (Debugger) delete Debugger;
	delete SymTab;
	delete Parser;
	delete NameStore;
	delete Lex;
	delete ErrorHandler;
	return 0;
}
