/*
 * (C) Copyright 2001 Diomidis Spinellis.
 *
 * For documentation read the corresponding .h file
 *
 * $Id: error.cpp,v 1.18 2006/06/24 15:45:57 dds Exp $
 */

#include <iostream>
#include <string>
#include <fstream>
#include <stack>
#include <deque>
#include <sstream>		// ostringstream
#include <map>
#include <vector>
#include <list>
#include <set>

#include "cpp.h"
#include "error.h"
#include "debug.h"
#include "attr.h"
#include "metrics.h"
#include "fileid.h"
#include "tokid.h"
#include "fchar.h"

int Error::num_errors;
int Error::num_warnings;
bool Error::parsing;


void
Error::error(enum e_error_level level, string msg, bool showloc)
{
	// Show all debugging output up to this point
	if (Debug::is_enabled())
		cout << flush;
	if (showloc)
		cerr << Fchar::get_path() << ':' << Fchar::get_line_num() << ": ";
	switch (level) {
	case E_WARN: cerr << "warning: "; break;
	case E_ERR: cerr << "error: "; break;
	case E_INTERNAL: cerr << "internal error: "; break;
	case E_FATAL: cerr << "fatal error: "; break;
	}
	cerr << msg << endl;
	switch (level) {
	case E_WARN: num_warnings++; break;
	case E_ERR:
		num_errors++;
		if (num_errors > 100)
			/*
			 * @error
			 * To avoid cascading errors only the first 100 errors
			 * are reported
			 */
			Error::error(E_FATAL, "error count exceeds 100; exiting", false);
		break;
	case E_INTERNAL: num_errors++; break;	// Should have an assertion before
	case E_FATAL: exit(1);
	}
}

void
Error::set_parsing(bool v)
{
	parsing = v;
}

void
Error::assertion_error(const char *fname, int line, const char *condition)
{
	ostringstream msg;
	msg << "assertion error: location: " << fname << ':' << line << " condition: \"" << condition << '"';
	error(E_INTERNAL, msg.str(), parsing);
}

#ifdef UNIT_TEST

#include "token.h"
#include "ptoken.h"
#include "ytab.h"
#include "pltoken.h"

main()
{
	Fchar::set_input("test/toktest.c");

	Error::error(E_WARN, "Error at beginning of file");
	for (;;) {
		Pltoken t;

		t.template getnext<Fchar>();
		if (t.get_code() == EOF)
			break;
		cout << t;
	}
	Error::error(E_WARN, "Error at EOF");

	return (0);
}
#endif /* UNIT_TEST */
