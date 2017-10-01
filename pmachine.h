// Pascal- p-machine

#ifndef __PMACHINE_H

#define __PMACHINE_H

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "pcode.h"

typedef enum	// run-time errors
{
	ERR_INPUT, ERR_ZERODIV, ERR_STACK, ERR_CUSTOM
}
RT_error_t;

class pmachine_t
{
private:
	pword_t*	store;			// memory store
	pword_t		s, b, p;		// p-machine registers
	pword_t		code_top;		// length of code in store & offset of stack

	pword_t		Drop(pword_t n = 1)
	{
		assert(s>=n);
		s -= n;
		return s;
	}
	pword_t		Pop(pword_t *v = NULL)
	{
		assert(s>0);
		return v ? (*v = store[s--]) : (store[s--]);
	}
	pword_t		Push(const pword_t v)
	{
		assert((s+1) < STORE_SIZE);
		store[++s] = v;
		return s;
	}
	pword_t	Rel(pword_t off)			{ return store[s+off]; 		}
	pword_t	Rel(pword_t off, pword_t v)	{ return store[s+off] = v; 	}
	pword_t	Abs(pword_t off)			{ return store[off]; 		}
	pword_t	Abs(pword_t off, pword_t v)	{ return store[off] = v; 	}
	pword_t TOS()						{ return store[s];			}
	pword_t TOS(pword_t v)				{ return store[s] = v;		}
	pword_t TOS1()						{ return store[s+1];		}
	pword_t TOS1(pword_t v)				{ return store[s+1] = v;	}

	pword_t Chain(pword_t level); // follow activation chain
	void	RunTimeError(RT_error_t err, char *arg = NULL);
	int		ExecOp();
public:
	pmachine_t()
	{
		store = new pword_t[STORE_SIZE];
		assert(store);
		s = 0;
	}
	~pmachine_t()
	{
		delete store;
	}
	void LoadAndGo(char *codefile);
};

#endif

