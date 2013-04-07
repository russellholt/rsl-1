#include "../server/cgi.h"
#include "rsl_control.h"
#include "../log/slog.h"
#include "R_html.h"
#include "R_Boolean.h"
#include "rescall.h"
#include "SLList_res_util.h"

slog Errorlog;
extern R_Boolean gShowStats;

extern void DestroyResList(resource *owner, SLList<resource *>& thelist);

main(int argc, char **argv)
{
int use_stdin = 0,	// source of script text data (stdin or file)
	html_output = 0;	// output is HTML formatted (obsolete ?)
restable *globals = new restable("Globals");
rsl_control scripter(globals);	// RSL
cgi abc;	// the CGI interface object (handler)
SLList<stringpair> alist;	// data interface with the CGI object
int opti;	// index var to argv
String opt;	// String of argv[opti]
int parse_only = 0, file_from_cline= 0;
String start_with = "Main";

	/* check out the command line options */
	for(opti=1; opti < argc; opti++)
	{
		opt = argv[opti];
		if (opt == "-s")	// show execution statistics
			gShowStats.Set(1);
		else
		if (opt == "-p")	// parse only
			parse_only = 1;
		else
		if (opt == "-start" && ++opti < argc)
		{
			start_with = argv[opti];
			start_with.gsub("\"", "");	// remove double quotes, if they exist
		}
		else
		if (opt == "-saveresptr")
			ResCall::SaveResptr = 1;
		else
		if (opt == "-nosaveresptr")
			ResCall::SaveResptr = 0;
		else
		if (opt == "-h")	// help/usage
		{
			cout << "Usage: " << argv[0] << " [-s] [-p] [-start block_name] "
				 << "[-setresptr] [-nosetresptr] [-h]\n"
				 << "           s: print execution statistics\n"
				 << "           p: parse but do not execute\n"
				 << "       start: name of program block to begin execution (default: Main)\n"
				 << "  saveresptr: (ResCall) save pointers to looked up resources \n"
				 << "nosaveresptr: (ResCall) don't...\n"
				 << "           h: usage information";
			exit(1);
		}
		else
		{
			int nerrors = scripter.ParseFile(opt);	// # errors returned
			if (nerrors)
			{
				cerr << opt << ": " << nerrors << " errors.\n\n";
				exit(nerrors);
			}
			file_from_cline = 1;
		}
	}

	R_html *r_ht = new R_html("HTML");
	scripter.AddReferenceObject(r_ht);	// new type

	use_stdin = 0;	// expecting CGI or some other script source
	
	restable *cgideclocals=NULL;
	resource *prerun=NULL;
	
    if (abc.LoadData())	// using CGI
    {    	
		abc.SendType("text/html");
	    abc.ParseCGIData(alist);
	    
	    // Execute the cgi variable declarations table
	    prerun = globals->lookup("CGIDeclarations");
    	cgideclocals = new restable("CGIdata");
	    if (prerun)
	    {
//	    	cout << "<p>Running CGIDeclarations script<p>";
	    	if (cgideclocals)
	    	{
				if (prerun && prerun->ClassName() == "List")
					((restable *)prerun)->execute(NULL, globals, NULL, cgideclocals);
//				cout << "<pre>CGIDeclarations table after execution:\n";
//				printordered(cgideclocals->GetList());
				// append the resulting declarations to the global table.
				globals->Add(cgideclocals->GetList());
//				cout << "<hr>Global table is now:\n";
//				printordered(globals->GetList());
//				cout << "</pre>";
			}

		}
		
	   	// make them rsl variables
//		scripter.add_var_table("CGIdata", alist);
		if (cgideclocals)
			globals->install_varlist_table(alist, cgideclocals);

		alist.clear();
		abc.GetServerVars(alist);	// get the environment variables
		scripter.add_vars(alist);	// make them rsl variables
		html_output = 1;
	}
	else
		use_stdin = 1;
	
	// parse the default file - named in the String resource
	// "script_file" in the restable rsl_control::globals

	int nerrors=0;
	if (!file_from_cline)	// not already parsed
		nerrors = scripter.ParseFile(use_stdin);
	if (nerrors > 0)
	{
		cerr << nerrors << " errors found.\n\n";
		exit(nerrors);
	}
	
	if (!parse_only)
		scripter.execute(start_with.chars());		// execute Main program
			
	/* Show execution statistics */
	if (html_output)
		cout << "<pre>\n";	// enforce text formatting in HTML


//	if (r_ht)
//		delete r_ht;
//
//	if (prerun)
//		delete prerun;
//
//	if (cgideclocals)
//		delete cgideclocals;
//

//	DestroyResList(globals, globals->GetList());
//	delete globals;
	globals = NULL;	// a memory leak test

	if (gShowStats.LogicalValue())	// check the rsl variable
		scripter.printstats();
}
