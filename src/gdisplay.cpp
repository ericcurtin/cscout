/*
 * (C) Copyright 2001-2015 Diomidis Spinellis
 *
 * This file is part of CScout.
 *
 * CScout is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CScout is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CScout.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Portable graph display abstraction
 *
 */

#include <map>
#include <string>
#include <deque>
#include <vector>
#include <stack>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <utility>
#include <functional>
#include <algorithm>		// set_difference
#include <cctype>
#include <sstream>		// ostringstream
#include <cstdio>		// perror, rename
#include <cstdlib>		// atoi
#include <cerrno>		// errno

#include "swill.h"
#include "getopt.h"

#include <regex.h>

#include "cpp.h"
#include "debug.h"
#include "error.h"
#include "ytab.h"
#include "attr.h"
#include "metrics.h"
#include "fileid.h"
#include "tokid.h"
#include "token.h"
#include "ptoken.h"
#include "fchar.h"
#include "pltoken.h"
#include "macro.h"
#include "pdtoken.h"
#include "eclass.h"
#include "ctoken.h"
#include "type.h"
#include "stab.h"
#include "fdep.h"
#include "version.h"
#include "call.h"
#include "fcall.h"
#include "mcall.h"
#include "compiledre.h"
#include "html.h"
#include "option.h"

#include "gdisplay.h"

#if defined(unix) || defined(__unix__) || defined(__MACH__)
#include <sys/types.h>		// mkdir
#include <sys/stat.h>		// mkdir
#include <unistd.h>		// unlink
#elif defined(WIN32)
#include <io.h>			// mkdir
#include <fcntl.h>		// O_BINARY
#endif

void
GDDot::head(const char *fname, const char *title, bool empty_node) {
	fprintf(fo, "#!/usr/local/bin/dot\n"
		"#\n# Generated by CScout %s - %s\n#\n\n"
		"digraph %s {\n",
		Version::get_revision().c_str(),
		Version::get_date().c_str(), fname);
	if (Option::dot_graph_options->get().length() > 0)
		fprintf(fo, "\t%s;\n", Option::dot_graph_options->get().c_str());
	if (empty_node)			// Empty nodes
		fprintf(fo, "\tnode [height=.001,width=0.000001,shape=box,label=\"\",fontsize=8];\n");
	else
		fprintf(fo, "\tnode [%s];\n", Option::dot_node_options->get().c_str());
	fprintf(fo, "\tedge [%s];\n", Option::dot_edge_options->get().c_str());
}

void
GDDotImage::head(const char *fname, const char *title, bool empty_node)
{
	#if defined(unix) || defined(__unix__) || defined(__MACH__)
	strcpy(img, "/tmp");
	#elif defined(WIN32)
	char *tmp = getenv("TEMP");
	strcpy(img, tmp ? tmp : ".");
	#else
	#error "Don't know how to obtain temporary directory"
	#endif
	strcpy(dot, img);
	strcat(dot, "/CSdot-XXXXXX");
	strcat(img, "/CSimg-XXXXXX");
	/*
	 * Using mkstemp here doesn't provide more security,
	 * because an attacker can switch the files underneath
	 * dot's execution. The really secure way is to pipe our
	 * data into dot and pipe dot's results directly from it.
	 * Also, mkstemp is not available under WIN32.
	 */
	mktemp(dot);
	mktemp(img);
	fo = fopen(dot, "w");
	if (fo == NULL) {
		html_perror(fo, "Unable to open " + string(dot) + " for writing", true);
		return;
	}
	GDDot::head(fname, title, empty_node);
}

void
GDDotImage::tail()
{
	GDDot::tail();
	fclose(fo);
	snprintf(cmd, sizeof(cmd), "dot -T%s \"%s\" \"-o%s\"", format, dot, img);
	if (DP())
		cout << cmd << '\n';
	if (system(cmd) != 0) {
		html_perror(fo, "Unable to execute " + string(cmd) + ". Shell execution", true);
		return;
	}
	FILE *fimg = fopen(img, "rb");
	if (fimg == NULL) {
		html_perror(fo, "Unable to open " + string(img) + " for reading", true);
		return;
	}
	int c;
	#ifdef WIN32
	setmode(fileno(result), O_BINARY);
	#endif
	while ((c = getc(fimg)) != EOF)
		putc(c, result);
	fclose(fimg);
	(void)unlink(dot);
	(void)unlink(img);
}