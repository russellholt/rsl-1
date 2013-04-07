// $Id: rescall.cc,v 1.8 1996/03/18 13:26:08 holtrf Exp holtrf $
//
// PURPOSE: a resource call, the basic element
//
// PROGRAMMING NOTES:
//
// HISTORY: 
//
// Copyright 1995 by Destiny Software Corporation.

#include "rescall.h"
#include "R_String.h"	// for AddArgument
#include "restable.h"
#include "rc_command.h"

const char * const rcsid = "$Id: rescall.cc,v 1.8 1996/03/18 13:26:08 holtrf Exp holtrf $";

extern int
	local_creates,
	local_destroys,
	local_tables_created,
	local_tables_destroyed,
	local_result_adds,
	temp_adds,
	temp_destroys,
	temp_tables_created,
	temp_tables_destroyed;


void ResCall::print(void)
{
String s;
	TextEquiv(s, 1);
	cout << s << '\n';
}

String ResCall::Value(void)
{
String s;
	TextEquiv(s, 1);
	return s;
}

// TextEquiv
// Make a text version of a resource call that is equivalent to,
// if not exactly the same as, the script text that was compiled.
// 
// RFH 9/15/95
void ResCall::TextEquiv(String &text, int showname)
{
Pix temp=args.first();
resource *r;
int op = 0;
String argtext;

	if (name == "" && resptr && resptr->ClassName()=="ResCall")	// eval
	{
		// instead of being a resource lookup by name, this
		// contains a pointer to another resource (ie an expression
		// evaluation) with a method and args, like
		// (a + b).reverse();
		((ResCall *)resptr)->TextEquiv(argtext, 1);
		text = argtext;
	}
	else
		text = name;

	if (method_name == "")	// variable reference - no method or args.
		return;

	if (method_name.matches(RXidentifier) || method_name[0] == '@')
		text += '.' + method_name + '(';	// object.method syntax
	else
	{
		op = 1;
		text += ' ' + method_name + ' ';	// operator syntax
	}

	while(temp) {
		r = args(temp);
		if (r)
		{
				// recursively print ResCall arguments
			if (r->ClassName() == "ResCall")
			{
				((ResCall *)r)->TextEquiv(argtext, 1);
				text += argtext;
			}
			else
			{
				//	text += r->ClassName() + " ";	// add type
				if (showname || op)
					text += r->Name();

					// only show value for unamed Strings (literals)
				if (r->ClassName() == "String" && r->Name() == "")
				{
					text += '\"';				// equals open-quote
					String xs = r->Value();

						// make control chars printable:
					xs.gsub("\n", "\\n");	// newline
					xs.gsub("\t", "\\t");	// tab
					xs.gsub("\"", "\\\"");	// double quote
					
					text += xs + '\"';		// arg-value close-quote
				}
			}
		}
//		else
//			text += "<null>";	// null resource pointer in the list!
			
		args.next(temp);
		if (temp)
		{
			if (op)
				text += ' ' + method_name + ' ';	// operator list!
			else
				text += ", ";			// add comma-space if not the last arg
		}
	}
	if (!op)
		text += ") ";
}

/* insert argument - called from parser */
void ResCall::InsertArg(resource *arg)
{
	if (arg)
		args.prepend(arg);
#ifdef DEBUG
	else
		cout << "ResCall::AddArgument() "
			 << "- null resource * (ignored)\n";
#endif
}

void ResCall::AddArgument(resource *arg)
{
	if (arg)
		args.append(arg);
#ifdef DEBUG
	else
		cout << "ResCall::AddArgument() "
			 << "- null resource * (ignored)\n";
#endif
}

void ResCall::AddArgument(stringpair &arg)
{
	resource *r = new R_String(arg.left(), arg.right());
	args.append(r);
}

void ResCall::AddArgument(const char *nm, const char *arg)
{
	resource *r = new R_String(nm, arg);
	//	stringpair s(nm, arg);
	args.append(r);
}
void ResCall::AddArgument(String &val)
{
String x = "no-name";
	resource *r = new R_String(x, val);	// don't care about the name
	args.append(r);
}
void ResCall::AddArgument(const char *val)
{
	resource *r = new R_String("no-name", val);	// don't care about the name
	args.append(r);
}

resource *ResCall::execute(String& method, SLList<resource *>& args)
{
	cout << "Do you really think you can \"" << method << "\" with a ResCall?\n";
	
	String m = downcase(method);
	resource *f = NULL;
	if (args.first() != NULL)
		f = args.front();

	if (m == "method")
		return GetSet(method_name, f);
	else
	if (m == "addargs" || m == "appendargs" || m == "insertargs")
	{
		Pix temp = args.first();
		int w = (m[0]=='i'?1:0);
		for(; temp; args.next(temp))
			if (w)
				InsertArg(args(temp));
			else
				AddArgument(args(temp));
	}
	else
	if (m == "execute")
		return _execute(args);
	
	return NULL;
}

///* FindResource
// * -- obsolete -- */
//int ResCall::FindResource(restable *vars)
//{			
//	// find the resource
//	
//	if (!resptr)
//		resptr = vars->GetResource(name);
//	if (!resptr)
//	{
//#ifdef DEBUG
//		cout << "ResCall: resource \"" << name
//			 << "\" not found for " << name << " " << method_name << " <br>\n";
//#endif
//		return 0;
//	}
//	return 1;
//}

//
// FindResource
// - find a resource, search by name with ::name. Search all relevant
//   restables. Search global tables by name for "name::method" in
//   addition.
// RFH Jan 15/96-- Extracted from ::execute
resource *ResCall::FindResource(
	restable*& arguments,
	restable*& globals,
	restable*& globals2,
	restable*& locals,
	restable*& result_temps,
	int& scope_resolved)
{
resource *theres = resptr, *cresptr = NULL;
int res_already_there = (resptr != NULL);

#ifdef DEBUG
	cout << "Looking for " << name << '\n';
#endif

	// First, check for the resource by name.
	
	if (!theres && locals)	// in local table
		theres = locals->GetResource(name);
	if (!theres && arguments)	// an argument to the routine where this rescall is located
		theres = arguments->GetResource(name);
	if (!theres && globals)	// in global variable table
		theres = globals->GetResource(name);
	if (!theres && globals2)	// in the other global object table
		theres = globals2->GetResource(name);
	if (!theres)	// in reference object table
		theres = References.GetResource(name);
	if (!theres && result_temps)	// in temporary object table.. (??? necessary?)
		theres = result_temps->GetResource(name);

	// Second, check for scope-resolved name by method, regardless of
	// whether it was found before,
	// -- but only if (12/6/95 RFH) the method name is an identifier
	// - and only if ResCall::type is 0 or 16
	if (method_name.matches(RXidentifier))	// &&
		//	( /* ResCall::type == 0 || */ ResCall::type == rescall_obj_var_ref)
		
	{
		String rs_name = name + "::" + method_name;
	
#ifdef DEBUG
		cout << "\nResCall: looking for \"" << rs_name << "\"\n";
#endif
	
		// if the resource had to be looked up, then we'll look up
		// a variation on its name as well - using the name::method notation,
		// to see if there were any compound statements defined by the user
		// which would take precedence. <whew>
		//
		// -- It looks in 'globals', not 'globals2', so 'globals' is assumed to
		//    be the "system globals", where restables were originally stored after
		//    the parse. This is because unless a program switches "global" restables
		//    in the rsl_control between the parsing of files, the only place that
		//    a resource whose name is of the form "name::method" is in the main global
		//    table (and those resources can only be restables)
		if (!res_already_there)
		{
			cresptr = globals->GetResource(rs_name);
			
			// if the first was found, second wasn't, check for scope-resolved
			// name by resource class name and method
			if (theres && !cresptr)	// Check for class_name::method
			{
				String cs_name = theres->ClassName() + "::" + method_name;
#ifdef DEBUG
			cout << "\nResCall: looking for \"" << cs_name << "\"\n";
#endif
				// look for things like "String::myfunc"
				cresptr = globals->GetResource(cs_name);
			}
		
			// if some scope resolved resource was found, let it override name.method
			if (cresptr)
			{
				theres = cresptr;
				scope_resolved = 1;
			}
		}
	}	// method name is an identifier
	
	return theres;
}

// _execute
// RSL interface to execute. Accepts up to 5 List arguments.
resource *ResCall::_execute(SLList<resource *>& args)
{
restable *arguments=NULL, *globals=NULL,
	*globals2=NULL, *locals=NULL, *result_temps=NULL;

Pix temp =args.first();
resource *r=NULL;
int i;
	for(i=0; temp; args.next(temp), i++)
	{
		r = args(temp);
		if (!r || (r && r->ClassName() != "List"))
		{
			cout << "ResCall::execute() requires List arguments only.\n";
			return NULL;
		}
		switch(i)
		{
			case 0:	arguments = (restable *)r;	break;
			case 1: globals = (restable *)r; break;
			case 2: globals2 = (restable *)r; break;
			case 3: locals = (restable *)r; break;
			case 4: result_temps = (restable *)r; break;
			default:
				cout << "ResCall::execute() uses only the first 5 arguments.\n";
		}
	}
	if (i > 0)
		return execute(arguments, globals, globals2, locals, result_temps);
	
	return NULL;
}


// execute
// - Step through the argument list and look for ResCalls
//   (resource invocations embedded in expressions) and make those
//   calls.
// - Add their return values to a new argument list for this
//   resource call.
// - perform variable lookups and add their values to the new argument list
// - make this resource call with the new argument list
//
// RFH 9/18 - 9/20/95
//
// 11/29/95 -
// - added 'arguments' restable: this is a table of arguments to the routine
//   that this rescall lives in.
resource *ResCall::execute(restable *arguments, restable *globals,
	restable *globals2, restable *locals, restable *result_temps)	// temp_results is default NULL
{
resource *theresult = NULL, *theres = resptr, *cresptr = NULL;
int scope_resolved=0, res_already_there = (resptr != NULL);

#ifdef DEBUG
	print();
#endif

	if (!globals && !globals2 && !locals)
		return NULL;	// no resources
		
	// look up the resource (if necessary)
	// old way:	if (!FindResource(vars)) return NULL;

#ifdef DEBUG
	cout << "Looking for " << name << '\n';
#endif

	// First, check for the resource by name.
	
	if (!theres && locals)	// in local table
		theres = locals->GetResource(name);
	if (!theres && arguments)	// an argument to the routine where this rescall is located
		theres = arguments->GetResource(name);
	if (!theres && globals)	// in global variable table
		theres = globals->GetResource(name);
	if (!theres && globals2)	// in the other global object table
		theres = globals2->GetResource(name);
	if (!theres)	// in reference object table
		theres = References.GetResource(name);
	if (!theres && result_temps)	// in temporary object table.. (??? necessary?)
		theres = result_temps->GetResource(name);

	// Second, check for scope-resolved name by method, regardless of
	// whether it was found before,
	// -- but only if (12/6/95 RFH) the method name is an identifier
	// - and only if ResCall::type is 0 or 16
	if (method_name.matches(RXidentifier))	// &&
		//	( /* ResCall::type == 0 || */ ResCall::type == rescall_obj_var_ref)
		
	{
		String rs_name = name + "::" + method_name;
	
#ifdef DEBUG
		cout << "\nResCall: looking for \"" << rs_name << "\"\n";
#endif
	
		// if the resource had to be looked up, then we'll look up
		// a variation on its name as well - using the name::method notation,
		// to see if there were any compound statements defined by the user
		// which would take precedence. <whew>
		//
		// -- It looks in 'globals', not 'globals2', so 'globals' is assumed to
		//    be the "system globals", where restables were originally stored after
		//    the parse. This is because unless a program switches "global" restables
		//    in the rsl_control between the parsing of files, the only place that
		//    a resource whose name is of the form "name::method" is in the main global
		//    table (and those resources can only be restables)
		if (!res_already_there)
		{
			cresptr = globals->GetResource(rs_name);
			
			// if the first was found, second wasn't, check for scope-resolved
			// name by resource class name and method
			if (theres && !cresptr)	// Check for class_name::method
			{
				String cs_name = theres->ClassName() + "::" + method_name;
#ifdef DEBUG
			cout << "\nResCall: looking for \"" << cs_name << "\"\n";
#endif
				// look for things like "String::myfunc"
				cresptr = globals->GetResource(cs_name);
			}
		
			// if some scope resolved resource was found, let it override name.method
			if (cresptr)
			{
				theres = cresptr;
				scope_resolved = 1;
			}
		}
	}	// method name is an identifier

	/* error messages (12/6/95)
	 * they go to stdout so that they appear when accessed via the web */
	if (!theres)
	{
		cout << "resource \"" << name << "\" not found.\n";
		return NULL;
	}
	if (!theres->Enabled())
	{
		cout << "resource \"" << name << "\" is disabled.\n";
		return NULL;
	}

	/************
	 * Special Cases:
	 * 1. variable lookup - no arguments or method- just return the resource * found
	 * 2. Allocation - look for method "@ Create" (set from within yacc parser, and
	 *    named this way so that users cannot call it directly from an rsl script)
	 *    - call resource::Create directly
	 * 3. Sub program execution - resource name == method name and the resource is of
	 *    type "List". This used to be handled within restable, but had to be moved
	 *    here to give subprograms access to global, reference, and local variables.
	 ************/
	
	// 1. variable lookup - no arguments, no method
//	if (method_name == "" && args.length() == 0)
	if (type == rescall_var_ref || type == rescall_obj_var_ref)
		return theres;
	
	// 2. Allocation
	if (method_name == "@ Create")	// declaration - allocation
			// must insert a pointer to variable storage
			// but only if it's not already there!
	{
		// old way:
//		if (args.front() && vars->Name() != (args.front())->Name())
//			args.prepend(vars);

		// new way:
		// I don't have to evaluate the args because I know the grammar,
		// and I can just directly call resource::Create from here
		// instead of making the individual resources call it.
		
			// new resource is to be installed into "locals".
//		resource::Create(resptr, args, locals);
//==== was resource::Create() =========================================		
		Pix temp = args.first();
		resource *new_name = NULL, *nr=NULL;

		if (!locals)
			return NULL;
	
		for(; temp; args.next(temp))
		{
			new_name = args(temp);
			if (new_name && new_name->ClassName()=="String")
			{
				restable *create_local = NULL;
				
				// Call the resource's Create method
				// -- this is the ONLY place it happens in the RSL system.
				nr = theres->Create(new_name->Value(), create_local);
				if (nr)
				{
					nr->MakeExist(exist_local);	// these are local variables
					locals->install_resource(nr);
					local_creates++;
					//======== On a roll!
					// See if there is a user-defined Create method for
					// this variable, like var_name::Create
					String new_cs_name = new_name->Value() + "::" + String("Create");
					resource *new_cs_res = globals->GetResource(new_cs_name);
					if (new_cs_res && new_cs_res->ClassName() == "List")
					{
						resource *retval =
						((restable *) new_cs_res)->execute(arguments, globals, globals2,
								(create_local? create_local : locals), result_temps);

						if (retval && retval->ClassName() == "rc_command")
							// If it is the 'Exit' command, we need to leave.
							if ( ((rc_command *) retval)->Type() == rc_command::Exit)
								return retval;
					}
					//========
				}
				else
					cout << "ResCall::execute (create): resource Create method failed.\n";
			}
			else
				cout << "ResCall::execute (create): Declaration with non-String argument (?) "
					 << "for the name: " << new_name->ClassName() << ' '
					 << new_name->Name() << '\n';
		}	
//====================================================================		
		
		
		return NULL;	// declarations have no return value.
	}

	
	//
	// Evaluate the arguments, putting the results into a new arglist.
	//
	// The new arglist is important, to keep the original untouched so that
	// the script can be executed again and again. The resources such as rescalls,
	// however, may be modifidied by having their resource pointers (rescall::resptr)
	// set (in the case of variable lookups, so that they aren't repeatedly looked up) 
	
	SLList<resource *> arglist2;

	evaluate_args(args, arglist2, arguments, globals, globals2, locals, result_temps);

	
	
	// 3. sub-program execution ("function" call)
	if (theres->ClassName() == "List" && (method_name == name || scope_resolved))
	{
#ifdef DEBUG
		cout << "ResCall- executing " << theres->Name() << '\n';
#endif
		restable *rtv; 	//	= locals;

		rtv = new restable;
		local_tables_created++;
		
		// table for the arguments - this is where (in the future) I'd make the
		// 'pass by value' or 'pass by reference' distinction, and where I'd
		// do the parameter substitution thing as well...
		restable *passing_arguments = new restable("Args");
		local_tables_created++;
		if (passing_arguments)
		{
			rtv->Insert(passing_arguments);

			passing_arguments->Add(arglist2);
		}

		resource *retval =
		((restable *) theres)->execute(passing_arguments, globals, globals2, rtv);
			

		// Clean up -----
		
		if (passing_arguments)
		{
			if (rtv->front() == passing_arguments)	// restable::front() is safe
				rtv->remove_front();	// should be passing_arguments
			else
			{
				cerr << "RESCALL: argument table is not element zero of local table.\n";
				//	rtv->Delete(String("Args"), String("name"));
				rtv->Delete("", "pointer", passing_arguments);
			}
			//	passing_arguments->remove_front(); // remove passing_arguments self-pointer
			delete passing_arguments;	// want to keep the resources though
			passing_arguments = NULL;
			local_tables_destroyed++;
		}

		// if locals didn't exist, we must free all local stuff we created.
		if (rtv)		//	!locals && rtv)
		{
			rtv->FreeAll();
			delete rtv;
			local_tables_destroyed++;
		}

		return retval;
	}
	
//	if (method_name == "distribute" && theres->ClassName()=="List")
// old way:		args.prepend(vars);


//	if (method_name == "distribute" && theres->ClassName()=="List")
//		theresult = (restable *)theres))->Distribute(arglist2, globals, globals2, locals)
//	else

	if (theres->ClassName() == "ResCall")
	{
//		cout << "RSL NOTICE: ResCall executing a ResCall (see Russell):\n";
		theresult = ((ResCall *) theres)->execute(arguments, globals,
			globals2, locals, result_temps);
	}
	else
		theresult = theres->execute(method_name, arglist2);

	StoreResult(theresult, locals, result_temps);

	if (SaveResptr)
		resptr = theres;
	
	return theresult;
}

// StoreResult
// for recording execution results
// - puts a resource into the table of temporaries or into the table of
//   local variables
// 10/20/95 RFH
void ResCall::StoreResult(resource *theresult, restable *locals, restable *result_temps)
{
	if (!theresult)
		return;

	// For rc_commands, the consumer will destroy it.
	if (theresult->ClassName() == "rc_command")
		return;

	// add any temporary result to a temp table		
	if (theresult->Exists() == exist_temp)
	{
		if (result_temps)	// try table of temporaries
			result_temps->install_resource(theresult);
		else
		if (locals)	// try local table
			locals->install_resource(theresult);
		else
		{
			cerr << "ResCall: Warning: no table to collect temporaries.\n";
			return;
		}
		temp_adds++;
	}
	else	/* add to local table */
		if ((theresult->Exists() & exist_add_to_locals) && locals)
		{
			int e = theresult->Exists();
			theresult->MakeExist(e - exist_add_to_locals + exist_local);
			locals->install_resource(theresult);
			local_result_adds++;
		}
}

// evaluate_args
// go through the argument list and evaluate (execute) each rescall
// put the results into the resultlist
// if an arg is not a rescall, just append it to the resultlist.
void ResCall::evaluate_args(
	SLList<resource *>& args,
	SLList<resource *>& resultlist,
	restable *arguments,
	restable *globals,
	restable *globals2,
	restable *locals,
	restable *result_temps)
{

Pix temp;
resource *rsrc = NULL, *result = NULL;

	if (args.length() > 0)
		for(temp = args.first(); temp; args.next(temp))
		{
			rsrc = args(temp);
			if (!rsrc)
				continue;
	
			if (rsrc->ClassName() == "ResCall")
			{
				if (rsrc == this)	// woah - check for self-reference
				{
#ifdef DEBUG
					cout << "ResCall::execute() -- Recursive loop!  "
						 << "found a ResCall resource that pointed to itself!<p>\n";
#endif
					continue;	// skip this one!
				}
	
				result = ((ResCall *) rsrc)->execute(arguments, globals, globals2,
						locals, result_temps);
				if (result)
				{
					resultlist.append(result);
					// don't want to:
					//		StoreResult(result, locals, result_temps);
					// because ResCall already does that internally!!!
				}
			}
			else
					resultlist.append(rsrc);
		}
	

}

