/*
 * (C) Copyright 2002 Diomidis Spinellis.
 *
 * For documentation read the corresponding .h file
 *
 * $Id: attr.cpp,v 1.16 2004/07/23 06:55:38 dds Exp $
 */

#include <map>
#include <string>
#include <deque>
#include <vector>
#include <stack>
#include <iterator>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <cassert>

#include "cpp.h"
#include "debug.h"
#include "attr.h"
#include "metrics.h"
#include "fileid.h"


// Leave space for a single project-attribute
vector<bool>::size_type Attributes::size = attr_end;

int Project::projid = attr_end - 1;
// Maps between ids and names
map<string, int> Project::projids;
vector<string> Project::projnames(attr_end);

// Keep this in sync with the enumeration
string Attributes::attribute_names[] = {
	"__attribute(__unused__)",

	// User-visible attributes start here
	"Read-only",

	"Tag for struct/union/enum",
	"Member of struct/union",
	"Label",
	"Ordinary identifier",

	"Macro",
	"Undefined macro",
	"Macro argument",

	"File scope",
	"Project scope",

	"Typedef",
	"Enumeration constant",
	"Function",
};

// Keep this in sync with the enumeration
string Attributes::attribute_short_names[] = {
	"__attribute(__unused__)",

	// User-visible attributes start here
	"ro",

	"tag",
	"member",
	"label",
	"obj",

	"macro",
	"umacro",
	"macroarg",

	"fscope",
	"pscope",

	"typedef",
	"enumconst",
	"function",
};

void
Project::set_current_project(const string &name)
{
	map<string, int>::const_iterator p;
	if ((p = projids.find(name)) == projids.end()) {
		// Add new project
		projid++;
		projnames.push_back(name);
		projids[name] = projid;
		if (Attributes::get_num_attributes() < projnames.size())
			Attributes::add_attribute();
	} else {
		projid = (*p).second;
	}
}
