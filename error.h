#ifndef __ERROR_H
//-----------------------------------------------------------------------
// error.h - Header file for error handler class of Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#define __ERROR_H

#include <stdlib.h> // for NULL

typedef enum
{
	WARN, ERROR, FATAL
} error_class_t;

typedef enum
{
	ERR_COMMENT, ERR_NUMSIZE, ERR_BADCHAR, ERR_NOSOURCE,
	ERR_MAXIDENTS, ERR_TOKENLEN, ERR_TRAILJUNK, ERR_EXPECT,
	ERR_BADTYPEDEF, ERR_UNDEFINED, ERR_REDEFINE, ERR_CONSTANT,
	ERR_TYPE, ERR_RANGE, ERR_SYNTAX, ERR_EXPRSYNTAX, ERR_NOFIELD,
	ERR_TYPEWAR, ERR_OUTFILE
} error_t;

class error_handler_t
{
private:
	void error_handler_t::_Error(int severity, const error_t err, const char *arg);
public:
	void Warning(const error_t err, const char *arg = NULL)
	{
		_Error(WARN, err, arg);
	}
	void Error(const error_t err, const char *arg = NULL)
	{
		_Error(ERROR, err, arg);
	}
	void Fatal(const error_t err, const char *arg = NULL)
	{
		_Error(FATAL, err, arg);
	}
	int Errors();
	int Warnings();
};

extern error_handler_t *ErrorHandler;

#endif

