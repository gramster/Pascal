// Quick and dirty p-code disassembler for debugging

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pcode.h"

main()
{
	pword_t *store = new pword_t[STORE_SIZE];
	assert(store);
#if __MSDOS__
	FILE *fp = fopen("pm.cod", "rb");
#else
	FILE *fp = fopen("pm.cod", "r");
#endif
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open code file %s\n", "pm.cod");
		exit(-1);
	}
	int top = fread(store, sizeof(pword_t), STORE_SIZE, fp);
	fclose(fp);
	pword_t p = 0;
	while (p < top)
		puts(DisassembleOp(store, p));
	delete store;
	return 0;
}


