/* 
 * (C) Copyright 2001 Diomidis Spinellis.
 *
 * Color identifiers by their equivalence classes
 *
 * $Id: webmap.cpp,v 1.7 2002/09/05 07:02:06 dds Exp $
 */

#include <map>
#include <string>
#include <deque>
#include <vector>
#include <stack>
#include <iterator>
#include <fstream>
#include <list>
#include <set>
#include <cassert>
#include <strstream>
#include <cstdio>		// perror

#ifdef unix
#include <sys/types.h>		// mkdir
#include <sys/stat.h>		// mkdir
#else
#include <io.h>			// mkdir 
#endif


#include "cpp.h"
#include "ytab.h"
#include "fileid.h"
#include "tokid.h"
#include "token.h"
#include "ptoken.h"
#include "fchar.h"
#include "error.h"
#include "pltoken.h"
#include "macro.h"
#include "pdtoken.h"
#include "eclass.h"
#include "debug.h"
#include "ctoken.h"
#include "type.h"
#include "stab.h"

// Our identifiers to store as a set
class Identifier {
	Eclass *ec;		// Equivalence class it belongs to
	string id;		// Identifier name
public:
	Identifier(Eclass *e, const string &s) : ec(e), id(s) {}
	string get_id() const { return id; }
	Eclass *get_ec() const { return ec; }
	// To create nicely ordered sets
	inline bool operator ==(const Identifier b) const {
		return (this->ec == b.ec) && (this->id == b.id);
	}
	inline bool operator <(const Identifier b) const {
		int r = this->id.compare(b.id);
		if (r == 0)
			return ((unsigned)this->ec < (unsigned)b.ec);
		else
			return (r < 0);
	}
};

static set <Identifier> ids;

// Return HTML equivalent of character c
static char *
html(char c)
{
	static char str[2];

	switch (c) {
	case '&': return "&amp;";
	case '<': return "&lt;";
	case '>': return "&gt;";
	case ' ': return "&nbsp;";
	case '\t': return "&nbsp;&nbsp;&nbsp;&nbsp;";
	case '\n': return "<br>\n";
	default:
		str[0] = c;
		return str;
	}
}

static string
html(string s)
{
	string r;

	for (string::const_iterator i = s.begin(); i != s.end(); i++)
		r += html(*i);
	return r;
}

// Display an identifier hyperlink
static void
html_id(ofstream &of, const Identifier &i)
{
	of << "<a href=\"i" << (unsigned)i.get_ec() << ".html\">" << i.get_id() << "</a>";
}

// Display the contents of a file in hypertext form
static void
file_hypertext(ofstream &of, string fname)
{
	ifstream in;
	Fileid fi;

	in.open(fname.c_str());
	if (in.fail()) {
		perror(fname.c_str());
		exit(1);
	}
	fi = Fileid(fname);
	// Go through the file character by character
	for (;;) {
		Tokid ti;
		int val, len;

		ti = Tokid(fi, in.tellg());
		if ((val = in.get()) == EOF)
			break;
		Eclass *ec;
		if ((ec = ti.check_ec()) && ec->get_size() > 1) {
			string s;
			s = (char)val;
			int len = ec->get_len();
			for (int j = 1; j < len; j++)
				s += (char)in.get();
			Identifier i(ec, s);
			ids.insert(i);
			html_id(of, i);
			continue;
		}
		of << html((char)val);
	}
	in.close();
}

// Create a new HTML file with a given filename and title
static void
html_head(ofstream &of, const string fname, const string title)
{
	of.open((string("html/") + fname + ".html").c_str(), ios::out);
	if (!of) {
		perror(fname.c_str());
		exit(1);
	}
	of <<	"<!doctype html public \"-//IETF//DTD HTML//EN\">\n"
		"<html>\n"
		"<head>\n"
		"<meta name=\"GENERATOR\" content=\"$Id: webmap.cpp,v 1.7 2002/09/05 07:02:06 dds Exp $\">\n"
		"<title>" << title << "</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>" << title << "</h1>\n";
}

// And an HTML file
static void
html_tail(ofstream &of)
{
	of <<	"<p>" 
		"<a href=\"index.html\">Main page</a>\n"
		"</body>"
		"</html>";
	of.close();
}

// Display a filename on an html file
static void
html_file(ofstream &of, Fileid fi)
{
	of << "<a href=\"f" << fi.get_id() << ".html\">" << fi.get_path() << "</a>";
}

static void
html_file(ofstream &of, string fname)
{
	Fileid fi = Fileid(fname);

	html_file(of, fi);
}



main(int argc, char *argv[])
{
	Pdtoken t;

	Debug::db_read();
	// Pass 1: process master file loop
	Fchar::set_input(argv[1]);
	do
		t.getnext();
	while (t.get_code() != EOF);

	// Pass 2: Create web pages
	vector <Fileid> files = Fileid::sorted_files();
	ofstream fo;

	#ifdef unix
	(void)mkdir("html", 0777);
	#else
	(void)mkdir("html");
	#endif

	// Index
	html_head(fo, "index", "Table of Contents");
	fo <<	"<ul>\n"
		"<li> <a href=\"afiles.html\">All files</a>\n"
		"<li> <a href=\"rofiles.html\">Read-only files</a>\n"
		"<li> <a href=\"wfiles.html\">Writable files</a>\n"
		"<li> <a href=\"aids.html\">All identifiers</a>\n"
		"<li> <a href=\"roids.html\">Read-only identifiers</a>\n"
		"<li> <a href=\"wids.html\">Writable identifiers</a>\n"
		"<li> <a href=\"uids.html\">Unused identifiers</a>\n"
		"</ul>";
	html_tail(fo);

	// Read-only files
	html_head(fo, "rofiles", "Read-only Files");
	fo << "<ul>";
	for (vector <Fileid>::const_iterator i = files.begin(); i != files.end(); i++) {
		if ((*i).get_readonly() == true) {
			fo << "\n<li>";
			html_file(fo, *i);
		}
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Writable files
	html_head(fo, "wfiles", "Writable Files");
	fo << "<ul>";
	for (vector <Fileid>::const_iterator i = files.begin(); i != files.end(); i++) {
		if ((*i).get_readonly() == false) {
			fo << "\n<li>";
			html_file(fo, *i);
		}
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// All files
	html_head(fo, "afiles", "All Files");
	fo << "<ul>";
	for (vector <Fileid>::const_iterator i = files.begin(); i != files.end(); i++) {
		fo << "\n<li>";
		html_file(fo, (*i).get_path());
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Details for each file 
	// As a side effect populite the EC identifier member
	for (vector <Fileid>::const_iterator i = files.begin(); i != files.end(); i++) {
		strstream fname;
		const string &pathname = (*i).get_path();
		fname << (*i).get_id();
		string sfname(fname.str(), fname.pcount());
		html_head(fo, (string("f") + sfname).c_str(), html(pathname));
		fo << "<ul>\n";
		fo << "<li> Read-only: " << ((*i).get_readonly() ? "Yes" : "No") << "\n";
		fo << "<li> <a href=\"s" << sfname << ".html\">Source code</a>\n";
		fo << "</ul>\n";

		html_tail(fo);
		// File source listing
		html_head(fo, (string("s") + sfname).c_str(), html(pathname));
		file_hypertext(fo, pathname);
		html_tail(fo);
	}

	// All identifiers
	html_head(fo, "aids", "All Identifiers");
	fo << "<ul>";
	for (set <Identifier>::const_iterator i = ids.begin(); i != ids.end(); i++) {
		fo << "\n<li>";
		html_id(fo, *i);
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Read-only identifiers
	html_head(fo, "roids", "Read-only Identifiers");
	fo << "<ul>";
	for (set <Identifier>::const_iterator i = ids.begin(); i != ids.end(); i++) {
		if ((*i).get_ec()->get_readonly() == true) {
			fo << "\n<li>";
			html_id(fo, *i);
		}
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Writable identifiers
	html_head(fo, "wids", "Writable Identifiers");
	fo << "<ul>";
	for (set <Identifier>::const_iterator i = ids.begin(); i != ids.end(); i++) {
		if ((*i).get_ec()->get_readonly() == false) {
			fo << "\n<li>";
			html_id(fo, *i);
		}
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Unused identifiers
	html_head(fo, "uids", "Unused Identifiers");
	fo << "<ul>";
	for (set <Identifier>::const_iterator i = ids.begin(); i != ids.end(); i++) {
		if ((*i).get_ec()->get_size() == 1) {
			fo << "\n<li>";
			html_id(fo, *i);
		}
	}
	fo << "\n</ul>\n";
	html_tail(fo);

	// Details for each identifier
	for (set <Identifier>::const_iterator i = ids.begin(); i != ids.end(); i++) {
		strstream fname;
		fname << (unsigned)(*i).get_ec();
		string sfname(fname.str(), fname.pcount());
		html_head(fo, (string("i") + sfname).c_str(), html((*i).get_id()));
		fo << "<ul>\n";
		fo << "<li> Read-only: " << ((*i).get_ec()->get_readonly() ? "Yes" : "No") << "\n";
		fo << "<li> Unused: " << ((*i).get_ec()->get_size() == 1 ? "Yes" : "No") << "\n";
		fo << "</ul>\n";
		files = (*i).get_ec()->sorted_files();
		fo << "<h2>Dependent Files (Writable)</h2>\n";
		fo << "<ul>\n";
		for (vector <Fileid>::const_iterator j = files.begin(); j != files.end(); j++) {
			if ((*i).get_ec()->get_readonly() == false) {
				fo << "\n<li>";
				html_file(fo, (*j).get_path());
			}
		}
		fo << "</ul>\n";
		fo << "<h2>Dependent Files (All)</h2>\n";
		fo << "<ul>\n";
		for (vector <Fileid>::const_iterator j = files.begin(); j != files.end(); j++) {
			fo << "\n<li>";
			html_file(fo, (*j).get_path());
		}
		fo << "</ul>\n";

		html_tail(fo);
	}

	return (0);
}