// rsl_control.cc
//
// PURPOSE: rsl controlling class
//
// PROGRAMMING NOTES:
//
// HISTORY: 
//		RFH 9/28/95
//
// $Id: rsl_control.cc,v 1.4 1996/03/18 13:27:15 holtrf Exp holtrf $
// Copyright 1995 by Destiny Software Corporation.

#include "rsl_control.h"
#include "R_Boolean.h"
#include "ifrescall.h"
#include "R_Integer.h"
#include "R_System.h"
#include "rc_command.h"

/*
#include "../log/slog.h"

#include "y.tab.h"
#include "r2.h"
#include <String.h>
#include <SLList.h>
#include <stream.h>
#include <fstream.h>

#include "resource.h"
#include "rescall.h"
#include "R_String.h"
#include "R_Output.h"
#include "R_File.h"
#include "restable.h"
#include "../server/cgi.h"
#include "../server/stringpair.h"
*/

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
int yyparse();
void rc_printlist(struct res_call *, char *);
int parse_it(const char *fn);
void unlink(char *);
int getpid(void);

/* should never call this from here:
void rc_kill(struct res_call *);
*/
void rc_kill_list(struct res_call *r);

}

/*
 * Some memory management statistics
 * Maybe make these static members of resource..? or rsl_control...
 */
int Cres_call_creates=0,	// # of struct res_call allocations in r2.y (calls to rc_create)
	Cres_call_destroys=0,	// # of calls to rc_kill (# of struct res_calls freed)
	local_creates=0,
	local_destroys=0,
	local_result_adds=0,
	local_tables_created=0,
	local_tables_destroyed=0,
	temp_adds=0,
	temp_destroys=0,
	temp_tables_created=0,
	temp_tables_destroyed=0,
	resource_lookups=0,
	resource_adds=0;
	
struct res_call *the_prog = NULL;

// extern slog Errorlog;

restable ResCall::References;

int ResCall::SaveResptr = 0;

R_Boolean *grTrue=NULL, *grFalse=NULL,
	gShowStats;	// global Boolean - show execution statistics when true

// -------- RSL initialization ------------------
void rsl_control::Setup(void)
{	

	if (is_setup)
		return;

	if (!globals)	// don't want to do this twice.
	{
		cerr << "rsl_control::Setup()- no global table.\n";
		return;
	}

	ResCall::References.SetName("References");
	InstallReferences();

	/* execution statistics global flag */
	gShowStats.Set(0);
	gShowStats.SetName("ShowStats");
	gShowStats.MakeExist(exist_perm);

	if (globals)
	{
		globals->install_resource(globals);	// hahahahaha!
		globals->install_resource(&gShowStats);
	}

	is_setup = 1;	// mark as already setup.
}

void rsl_control::InstallReferences(void)
{	
	restable *r_t = new restable("List");	// a template variable
	r_t->MakeExist(exist_read_only);
	r_t->SetName("List");
	AddReferenceObject(r_t);

	R_File *r_file = new R_File;
	r_file->MakeExist(exist_read_only);
	r_file->SetName("File");	// give it a name
	AddReferenceObject(r_file);

	R_Output *r_out = new R_Output;
	r_out->MakeExist(exist_read_only);
	r_out->SetName("out");
	AddReferenceObject(r_out);

	R_String *r_string = new R_String;
	r_string->MakeExist(exist_read_only);
	r_string->SetName("String");
	AddReferenceObject(r_string);

	R_String *r_endl = new R_String("endl", "\n");
	r_endl->MakeExist(exist_read_only);
	AddReferenceObject(r_endl);
	
	R_String *r_endp = new R_String("endp", "\n\n");
	r_endp->MakeExist(exist_read_only);
	AddReferenceObject(r_endp);
	
	R_Boolean *r_bool = new R_Boolean;
	r_bool->SetName("Boolean");
	r_bool->MakeExist(exist_read_only);
	AddReferenceObject(r_bool);
	
	R_Integer *r_int = new R_Integer("Integer", 0);
	r_int->MakeExist(exist_read_only);
	AddReferenceObject(r_int);

	// Process ID variable
	R_Integer *r_pid = new R_Integer("PID", getpid());
	r_pid->MakeExist(exist_read_only);
	AddReferenceObject(r_pid);
	
	// Time variable
	time_t now = time(0);
	struct tm *ltime = localtime(&now);
	R_String *r_now = new R_String("Time", asctime(ltime));
	r_now->MakeExist(exist_read_only);
	AddReferenceObject(r_now);
	
	R_System *r_sys = new R_System("System");
	r_sys->MakeExist(exist_read_only);
	AddReferenceObject(r_sys);
	
	// an experiment:
	grTrue = new R_Boolean;
	grTrue->SetName("bTrue");
	grTrue->Set(1);
	grTrue->MakeExist(exist_read_only);
	AddReferenceObject(grTrue);

	grFalse = new R_Boolean;
	grFalse->SetName("bFalse");
	grFalse->Set(0);
	grFalse->MakeExist(exist_read_only);
	globals->install_resource(grFalse);	
	AddReferenceObject(grFalse);	
}

void rsl_control::AddReferenceObject(resource *r)
{
	//	globals->install_resource(r);	
	ResCall::References.install_resource(r);
}

void rsl_control::AddGlobalObject(resource *r)
{
	if (globals)
		globals->install_resource(r);
	else
		cerr << "rsl_control: AddGlobalObject: no global table!\n";
}


void rsl_control::add_vars(SLList<stringpair>& vars)
{
	if (globals)
		globals->install_varlist(vars);
	else
		cerr << "rsl_control: add_vars: no global table!\n";
}

// add_var_table
// Creates String resources for each stringpair.
// These are added to the global variables, and also to a separate
// restable which identifies the collection as a whole.
//
void rsl_control::add_var_table(String nm, SLList<stringpair> &vars)
{
	if (globals)
		globals->install_varlist_table(nm, vars);
	else
		cerr << "rsl_control: add_var_table: no global table!\n";

}


// -------- RSL Parsing -------------------------

// ParseFile
//  **** this is a weirdo obsolete routine. (as of 3/20/96) ****
//
// use the filename given in the String resource "script_file"
// as the source of the script data.
// "script_file" would be a resource created by add_vars, from
// CGI data or some other source.
//
// RFH 9/29/95
int rsl_control::ParseFile(int	&use_stdin)
{
int nerrors=0;
	if (!use_stdin)
	{
		resource *script_file =	globals->lookup("script_file");
		if (script_file)
		{
			ScriptFileName = script_file->Value();
			nerrors = ParseFile(ScriptFileName.chars());
		}

		script_file	= globals->lookup("script_file2");	// maybe others also
		if (script_file)
		{
			String x = script_file->Value();
			the_prog = NULL;
			nerrors = ParseFile(x.chars());
		}
	}
	else
		nerrors = ParseFile("");	// blank string means read from standard in

	return nerrors;
}

// ParseFile
// parse the named file and add all programs contained in
// it to the restable "globals."
//
// RFH 9/27-29/95
int rsl_control::ParseFile(const char *fn)
{
#ifdef MEMCHECK
	cout << "Parsing file \"" << fn << "\"...\n";
#endif

	int nerrors = parse_it(fn);
	if (nerrors)
		return nerrors;

#ifdef MEMCHECK
	cout << "Creating resources from the parse tree...\n";
#endif
	// resourcify parse tree
	make_program(the_prog, globals);

	if (!the_prog)
		return 1;	// error.

#ifdef MEMCHECK
	cout << "\n--- The program: --------------------------\n";
	rc_printlist(the_prog, "\n");
	cout << "\n-------------------------------------------\n";
	cout << "Freeing the parse tree...\n";
#endif

	// free parse tree
	rc_kill_list(the_prog);
	the_prog = NULL;

	return 0;	// no apparent errors (successful parse)
}

// execute
// find the named compound statment and execute it with the supplied
// restable to use as its local variable storage (defaults to NULL)
void rsl_control::execute(const char *prog, restable *locals)
{
	if (!globals)
	{
		cerr << "rsl_control: execute(): no global table.\n";
		return;
	}
resource *start = globals->lookup(prog);

	if (start && start->ClassName() == "List")
	{
		restable *the_rtab = (restable *) start;
		// execute it
		// - the first argument is a table of arguments (just resources)
		//   to pass into the routine (ie, maybe from the command line)
		// - the third argument is just another table to look for resources.

		resource *retval = 
		the_rtab->execute(NULL, globals, NULL, locals);
	}
	else
		cout << "\n*** Unable to find program " << prog << '\n';
}

/* make_program
 * A program is a list of res_calls, where each is TYPE_RESTABLE
 * each of these represent a named compound statement, ie subprograms,
 * to be installed as restable resources in the global variable restable.
 * Execution begins with the one named "main".
 *
 * The parser creates each compound statement as a list of res_calls,
 * and the program is a list of header res_calls where the compound statemnts
 * are stored in the res_call.args pointer.
 *
 * RFH 9/28/95
 */
void rsl_control::make_program(struct res_call *rc, restable *globals)
{
	if (!rc || !globals) return;

struct res_call *rct = rc, *rctnext=NULL;
restable *rtab;

	while(rct)
	{
		rctnext = rct->next;	// save a pointer to the next in the list
		if (rct->args)	// args begins the list of res_calls
		{
#ifdef MEMCHECK
			cerr << "make restable " << rct->res_name << '\n';
#endif
			rtab = make_restable(rct->args);
			if (rtab)
			{
				String newname = rct->res_name;
				if (rct->method_name)	// scope resolved: object_name::method_name
				{
					newname += "::" + String(rct->method_name);
#ifdef DEBUG
					cout << "\nScope resolved restable -- " << newname << '\n';
#endif
				}
				rtab->SetName(newname);
				globals->install_resource(rtab);
			}
		}
#ifdef MEMCHECK
		cerr << "-- free restable: --\n";
#endif

		rct = rctnext;
	}
	
}

restable *rsl_control::make_restable(struct res_call *rc)
{
	if(!rc) return NULL;	// bogus
	
	restable *rtab = new restable();
	struct res_call *rct = rc, *rctnext=NULL;

	while(rct)	// walk the res_call list
	{
		rctnext = rct->next;	// save a pointer to the next in the list
		resource *xr = make_resource(rct);
		rtab->Insert(xr);
#ifdef MEMCHECK
		cerr << "-- free rescall: --\n";
#endif
		rct = rctnext;
	}
	if (rtab)
		rtab->MakeExist(exist_perm);
	return rtab;
}

/* make_resource
 * construct the appropriate resource subclass from
 * a struct res_call (r2.h), created by the parser.
 * Recursively descent any argument lists (depth
 * first).
 * RFH 9/23/95
 */
resource *rsl_control::make_resource(struct res_call *r)
{
	if (!r)
		return NULL;

#ifdef DEBUG
	cout << "make_resource " << (r->res_name?r->res_name:"") << '\n';
#endif
		
	switch(r->type)
	{
		case TYPE_RESCALL:
			if (r->res_name)
			{
//				cout << r->res_name << "." << r->method_name;
				ResCall *nr = new ResCall(r->res_name, r->method_name);
//				cout << "( ";
				if (r->args)
				{
					struct res_call *tr = r->args;
					while(tr)
					{
						nr->InsertArg(make_resource(tr));
						tr=tr->next;
//						if (tr)
//							cout << ", ";
					}
				}
#ifdef DEBUG
				else
					cout << "\nNo arguments for " << r->res_name << '.'
						 << r->method_name << '\n';
#endif
//				cout << ") ";

				if (r->eval)
					cout << "make rescall: eval exists! "
						<< (r->eval->res_name?r->eval->res_name: "" ) << '\n';

				if (nr)
					nr->MakeExist(exist_perm + exist_read_only);
				return nr;
			}
			break;
		case TYPE_STRING:	// a literal string
			if (r->method_name)
			{					
				R_String *nr = NULL;
				String s = r->method_name;

				if (s[0] == '\"')	// remove " marks
				{
					s = s.after(0);
					if (s.length() > 0)
						s = s.before((int) (s.length()-1));
				}
				// quick check for blank string
				if (s.length() == 0)
					nr = new R_String("", "");
				else
				{	
					int n = s.gsub("\\n", "\n");	// newline
						n = s.gsub("\\t", "\t");	// tab
						n = s.gsub("\\r", "\r");	// carriage return
						n = s.gsub("\\\"", "\"");	// double quote
					
					if (s.length() > 0)
						nr = new R_String(r->res_name, s.chars());
					else
						nr = new R_String("", "");
				}

				if (nr)
					nr->MakeExist(exist_perm);

				return nr;
			}
			break;
		case TYPE_VAR_REF:
			if (r->res_name)
			{
				ResCall *nr = new ResCall(r->res_name, "");	// variable lookup has blank method name
//				cout << " Var " << r->res_name;
				if (nr)
				{
					nr->MakeExist(exist_perm);
					nr->Type(rescall_var_ref);
				}
				return nr;
			}
			break;
		case TYPE_DECL:
			if (r->res_name)
			{			// give declaration strings the same name and value
						// so they can be printed in script reconstruction
						// (only value matters though)
				R_String *nr = new R_String(r->res_name, r->res_name);
//				cout << ' ' << r->res_name << ' ';
				if (nr)
					nr->MakeExist(exist_perm + exist_read_only);
				return nr;
			}
			break;
		case TYPE_FILE:
//			if (r->res_name)	// the file name
//			{
				if (r->args)
				{
					ResCall *nr = new ResCall("", "setname");
					R_File *rf = new R_File;
					nr->SetResPtr(rf);
					nr->InsertArg(make_resource(r->args));
					if (nr)
						nr->MakeExist(exist_perm);
					return nr;
				}
//			}
			break;
		case TYPE_EVAL:
			if (r->eval)
			{
				ResCall *nr = new ResCall("", r->method_name);
				if (r->eval)
				{
					resource *xr = make_resource(r->eval);
					nr->SetResPtr(xr);
					if (r->args)
					{
						struct res_call *tr = r->args;
						resource *argres;
						while(tr)
						{
							argres = make_resource(tr);
							if (argres)
								nr->InsertArg(argres);
							tr=tr->next;
						}
						r->args = NULL;	// ** important!!
					}
//					else
//						cout << "\nmake_resource: TYPE_EVAL: no arguments for "
//							 << r->method_name << "\n";
				}
				if (nr)
					nr->MakeExist(exist_perm);
				return nr;
			}
			break;
			
		case TYPE_INT:
			if (r->method_name)
			{
				R_Integer *nr = new R_Integer("", atoi(r->method_name));
				if (nr)
					nr->MakeExist(exist_perm);
				return nr;
			}
			break;
			
		case TYPE_RESTABLE:
			{
				/* for compound statements */
				if (r->args)	// list to make a restable is in the args pointer
				{
					restable *nr = make_restable(r->args);
					// make_restable frees its argument, so pointers to that
					// memory must be set to NULL, ie, saying make_restable(r->args)
					// tells make_restable about the block of memory but not
					// about the pointer to it named "r->args".
					r->args = NULL;
					
					if (nr && r->res_name)
						nr->SetName(r->res_name);	// name is actually optional
					if (nr)
						nr->MakeExist(exist_perm);
					return nr;
				}
				else
					cout << "\nmake_resource: no restable data for TYPE_RESTABLE!\n";
			}
			break;
			
		case TYPE_SELECT:	/* if then else */
			{
#ifdef DEBUG
				cout << "\nFound selection\n";
#endif
				if (r->eval && r->args)
				{
					ifResCall *nr = new ifResCall;
					resource *res = make_resource(r->eval);

					nr->SetResPtr(res);

#ifdef DEBUG
					if (r->args)
						cout << "Select: true condition exists\n";
#endif
					res = make_resource(r->args);
					nr->SetTrueRes(res);

					if (r->extra)	// false (else) is optional
					{
#ifdef DEBUG
						cout << "Select: false condition exists\n";
#endif
						res = make_resource(r->extra);

						nr->SetFalseRes(res);
					}
					
					if (nr)
						nr->MakeExist(exist_perm);
					return nr;
				}
			}
			break;
			
		case TYPE_BOOLEAN:
			{
				R_Boolean *nr = new R_Boolean;
				nr->Set(r->method_name[0] == 't'?1:0);
				if (nr)
					nr->MakeExist(exist_perm);
				return nr;
			}
			break;
			
		case TYPE_FLOAT:	// value stored as string (char *) in r->method_name
			cout << "type float not implemented.\n";
			break;
			
		case TYPE_HEXVAL:	// value stored as string (char *) in r->method_name
			cout << "type hexidecimal not implemented.\n";
			break;			
		
		case TYPE_OBJ_VAR_REF:	// object.variable
			if (r->res_name)
			{
				ResCall *nr = new ResCall(r->res_name, r->method_name);	// variable lookup has blank method name
//				cout << " Var " << r->res_name;
				if (nr)
				{
					nr->MakeExist(exist_perm);
					nr->Type(rescall_obj_var_ref);
				}
				return nr;
			}
			break;

		case TYPE_RETURN:	
			{
				rc_command *nr = new rc_command(rc_command::Return);
				return nr;
			}
			break;
		case TYPE_BREAK:	
			{
				rc_command *nr = new rc_command(rc_command::Break);
				return nr;
			}
			break;
		case TYPE_CONTINUE:	
			{
				rc_command *nr = new rc_command(rc_command::Continue);
				return nr;
			}
			break;
		case TYPE_EXIT:	
			{
				rc_command *nr = new rc_command(rc_command::Exit);
				return nr;
			}
			break;
		
		default: 			cout << " unknown type " << r->type << " ? \n";
	}

#ifdef DEBUG
	cout << "\nmake_resource: returning NULL\n";
#endif

	return NULL;	// fall through -- error report?
}


resource *rsl_control::GetResource(char *nm)
{
	return (globals? globals->lookup(nm) : NULL);
}

// printstats
// print pre- and post-execution statistics, concerning the number
// of parser structures created/destroyed, the number of ResCalls,
// the number of variable lookups, etc.
void rsl_control::printstats(void)
{
	cout << "\nrsl execution statistics\n========================\n" << '\n';
	cout << "struct res_calls: created " << Cres_call_creates
		 << ", destroyed " << Cres_call_destroys << '\n';
	cout << "local resources: created " << local_creates
		 << ", destroyed " << local_destroys << '\n';
	cout << "local variable restables: created " << local_tables_created
		 << ", destroyed " << local_tables_destroyed << '\n';
	cout << "return values added to local restables: " << local_result_adds << '\n';
	cout << "temp resources: created " << temp_adds
		 << ", destroyed " << temp_destroys << '\n';
	cout << "temp restables: created " << temp_tables_created
		 << ", destroyed " << temp_tables_destroyed << '\n';
	cout << "resource lookups: " << resource_lookups << '\n';
	cout << "resources added to restables: " << resource_adds << "\n\n";
}


// SetGlobalTable
// Set the table to be used as the global variables, available to
// all program blocks.
void rsl_control::SetGlobalTable(restable *g)
{
	if (g)
	{
		globals = g;
		if (!is_setup)
			Setup();	// install primitive resources
	}
}
