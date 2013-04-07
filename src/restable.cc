//	$Id: restable.cc,v 1.11 1996/04/02 19:26:57 holtrf Exp holtrf $
//	
// Purpose:
//	a resource list, used for collecting resources. When    all resources
//	in it are ResCalls, it is a program.
//	
/*

RSL interface:
	
	=
	Append, Add, <<
	Prepend, Insert
	Length
	Distribute, DistributeType
	DistributeAll
	Assoc
	Print
	[]
	DeleteByName
	DeleteByValue
	DeleteByIndex
	AllTrue
	AllFalse
	Clear
	ClearAndDestroy

*/
// History:
//	9/28/95 - began from old version (CRB class "Collection")
//	9/29/95 - working version 1
//	10/16-18 - freeing parser-created C structs
//	
//
//	Copyright 1995, 1996 by Destiny Software Corporation.
//
#include "restable.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_File.h"
#include "rc_command.h"
#include "rescall.h"
#include "restree.h"
#include "SLList_res_util.h"
#include <ctype.h>

#ifdef WEB
#include "../server/ohtml.h"
#include "../server/htmltable.h"
// give linking info for .o identification with the RCS command `ident'
const char * const webid = "$Linking: Includes HTML table formatting. $ ";
#endif

const char * const rcsid = "$Id: restable.cc,v 1.11 1996/04/02 19:26:57 holtrf Exp holtrf $ ";

extern int
	local_creates,
	local_destroys,
	local_tables_created,
	local_tables_destroyed,
	local_result_adds,
	temp_adds,
	temp_destroys,
	temp_tables_created,
	temp_tables_destroyed,
	resource_lookups,
	resource_adds;

extern "C" {
	int atoi(const char*);
}


void DestroyResList(resource *owner, SLList<resource *>& thelist);
extern unsigned int ha(const char *s0);


//
// RSL methods
//


#define _hOppEQ 42	// :=
#define _hOpLS 45	// <<
#define _hOpEQ 133	// =
#define _hPRINT 213	// print
#define _hPSORT 363	// psort
#define _hOpIndex 451	// []
#define _hSORT 503	// sort
#define _hLENGTH 589	// length
#define _hCLEAR 963	// clear
#define _hADD 1265	// add
#define _hPREPEND 1325	// prepend
#define _hOpXXX 1385	// +<<
#define _hINSERT 2389	// insert
#define _hPRINTTABLE 3185	// printtable
#define _hASSOC 5430	// assoc
#define _hAPPEND 8237	// append
#define _hDISTRIBUTE 8244	// distribute
#define _hDELETEBYVALUE 8422	// deletebyvalue
#define _hDELETEBYNAME 8560	// deletebyname
#define _hDISTRIBUTE2 12343	// distribute2
#define _hALLFALSE 16420	// allfalse
#define _hSORTASCEND 16433	// sortascend
#define _hALLTRUE 16496	// alltrue
#define _hSORTDESCEND 18785	// sortdescend
#define _hCLEARANDDESTROY 20517	// clearanddestroy
#define _hDISTRIBUTEALL 20834	// distributeall
#define _hPRINTTEXTSCRIPT 41068	// printtextscript
#define _hGETRESOURCE 59437	// getresource
#define _hDELETEBYINDEX 76369	// deletebyindex
#define _hPRINTHTMLSCRIPT 87645	// printhtmlscript
#define _hASSOCIATE 131264	// associate
#define _hDISTRIBUTETYPE 140484	// distributetype
#define _hAPPENDLISTITEMS 1387258	// appendlistitems

void restable::Init(void)
{
	class_name = "List";
	hierarchy += "List:";
	where=NULL;
	lastindex=invalid_index;
}

restable::restable(void)
{
	Init();
	hierarchy += "List:";
}
restable::restable(String &nm)
{
	Init();
	name = nm;
	hierarchy += "List:";
}
restable::restable(const char *nm)
{
	Init();
	name = nm;
	hierarchy += "List:";
}
// find a resource by name (String &)
resource *restable::lookup(String &rname)
{
	return GetResource(rname);
	// might do other things also..
}

// find a resource by name (char *)
resource *restable::lookup(const char *rname)
{
String s = rname;
	return GetResource(s);
}

void restable::install_resource(resource *r)
{
resource *rx = NULL;

	if (r)
	{
		// if it is a temporary resource, then duplicate names are fine.
		if (r->Exists() == exist_temp)
			Add(r);
		else
		{	// check for duplicate names
			rx = GetResource(r->Name());
			if (!rx)
				Add(r);
#ifdef DEBUG
			else
			{
				cerr << "restable::install_resource - "
					 << "resource name conflict on " << r->Name() << ".\n";
				cerr << "Existing resource: ";
				rx->print();
				cerr << "\nAttempted resource: ";
				r->print();
				cerr << "\nNot installed.\n";
			}
#endif
		}
	}
}


// rslAdd - private method
// Add many resources at once
// - to be called from within restable::execute only, because of
//   the specific way it deals with File resources.
void restable::rslAdd(SLList<resource *> &e)
{
	// examine the resources to add.
Pix temp = e.first();
resource *r = NULL;
	for(; temp; e.next(temp))
	{
		r = e(temp);
		if (r)
		{
			// if the resource is a File, and it is temporary, which means:
			// the rescall was File(...) which returns a temp resource
			if (r->ClassName() == "File" && r->Exists() == exist_temp)
			{
				int before = elements.length();
				((R_File *) r)->_read_(this);	// hahaha!
				resource_adds += (elements.length() - before);	// record explicit stats
			}
			else
			{
				resource_adds++;
				elements.append(r);
			}
		}
	}
}


// Add
// add a bunch of resources at once
void restable::Add(SLList<resource *> &e)
{
	resource_adds+= e.length();
	elements.join(e);
}

resource *restable::GetResource(String& resname)
{
Pix temp = elements.first();
resource *r, *rp = NULL;

#ifdef DEBUG
	cerr << "   rt::getres \"" << resname << "\": ";
#endif

	resource_lookups++;
	while(temp)
	{
		r = elements(temp);
		if (r && r->Is(resname))
		{
#ifdef DEBUG
			cerr << "found in \"" << name << "\".\n";
#endif
			return r;
		}
		elements.next(temp);
	}

#ifdef DEBUG
	cerr << "not found in \"" << name << "\".\n";
#endif

	return NULL;
}

/***************************************************************\
** execute
**    RSL message interface
** 
** RFH
** Hash branching added April 2, 1996
\***************************************************************/
resource *restable::execute(String &method, SLList<resource *>& args)
{
resource *f = NULL;

	if (args.first())
		f = args.front();

	if (method == name)	// it is a program
	{
		cerr << "restable::execute(method, args) for subprograms is no longer available:\n"
			 << "   You should not see this message.\n";
		return NULL;
	}
	
	String m = downcase(method);

	switch(ha(m))	// branch on hash value
	{
		case _hAPPEND:	// "append"
		case _hADD:	// "add"
		case _hOpLS:	// "<<"
			if (resource::existence != exist_read_only)
				rslAdd(args);
			break;

		case _hAPPENDLISTITEMS:	// "appendlistitems"
			if (f && f->HierarchyContains(":List:"))
				Add(((restable *) f)->GetList());
			break;

		case _hOpIndex:	// "[]"
		case _hGETRESOURCE:	// "getresource"
			if (f)
				return Index(f);
			break;

		case _hPREPEND:	// "prepend"
		case _hINSERT:	// "insert"
			if (resource::existence != exist_read_only)
				Insert(args);
			break;

		case _hOpEQ:	// "="
		case _hOppEQ:	// ":="
			return Assign(f);

		case _hDISTRIBUTE:	// "distribute"
		case _hDISTRIBUTETYPE:	// "distributetype"
			Distribute(args, 1);
			break;

		case _hDISTRIBUTE2:	// "distribute2"
		case _hDISTRIBUTEALL:	// "distributeall"
			Distribute(args, 0);
			break;

		case _hASSOC:	// "assoc"
		case _hASSOCIATE:	// "associate"
			return Assoc(args);

		case _hSORT:	// "sort"
		case _hSORTASCEND:	// "sortascend"
			return Sort(args, restree::ascend);

		case _hPSORT:	// "psort"
			{
				resource *rt = Sort(args, restree::ascend);
				if (rt && rt->ClassName() == "List")
				{
					elements = ((restable *) rt)->GetList();
					delete rt;
				}
			}
			return this;

		case _hSORTDESCEND:	// "sortdescend"
			return Sort(args, restree::descend);

		case _hALLTRUE:	// "alltrue"
			return AllTrueOrFalse(1);

		case _hALLFALSE:	// "allfalse"
			return AllTrueOrFalse(0);

		case _hDELETEBYNAME:	// "deletebyname"
		case _hDELETEBYVALUE:	// "deletebyvalue"
		case _hDELETEBYINDEX:	// "deletebyindex"
			if (f && f->ClassName() == "String");
				return Delete(f->Value(), String(m.after("deleteby")));
			break;

		case _hCLEAR:	// "clear"
			elements.clear();
			where =NULL;
			break;

		case _hCLEARANDDESTROY:	// "clearanddestroy"
			FreeAll();
			break;

		case _hPRINT:	// "print"
			print(f?f->Value() : "");
			break;

		case _hLENGTH:	// "length"
			return new R_Integer("", elements.length());
			break;

		case _hPRINTTABLE:	// "printtable"
			PrintTable();
			break;

		case _hPRINTHTMLSCRIPT:	// "printhtmlscript"
			PrintScript(1);
			break;

		case _hPRINTTEXTSCRIPT:	// "printtextscript"
			PrintScript(0);
			break;

		default: ;

	}

	return NULL;
}

// CleanUp
// delete all temporary objects and their list nodes,
// but leave everything else alone
void restable::CleanUp(void)
{
Pix temp = elements.first(), prev = temp, next=NULL;
resource *r = NULL;
int i=0, len=elements.length();

#ifdef MEMCHECK
	cout << "restable::CleanUp(): list has " << elements.length() << " nodes.\n";
	print();
	cout << "\n\n";
#endif

	while(temp)
//	for(; temp; elements.next(temp))
	{
		r = elements(temp);

		/* delete all the temporary resources pointed to from the list nodes */
		if (r && r->Exists() == exist_temp)
		{
#ifdef MEMCHECK
			cout << "restable::CleanUp(): delete ";
			r->print();
			cout << '\n';
#endif
			delete r;	// delete the object pointed to
			temp_destroys++;
			
				// save a pointer to the next node so we can advance to it
			next = Pix( ((SLNode<resource *> *) temp)->tl);	// yuck

			/* delete the list node */
			if (prev == temp)	// it is the front
				elements.del_front();
			else
				elements.del_after(prev);
			i++;

			temp = next;
		}
		else
			elements.next(temp);	// not deleted so we can use it to advance
	}

	if (i > 0)	// some were deleted
	{
		where =NULL;
		lastindex=invalid_index;
	}
#ifdef MEMCHECK
	cerr << "restable::CleanUp() " << name << " -- " << i
		 << " temporary resources deleted of " << len << " total.\n";
#endif
}

// FreeAll
// - delete all non-permanent objects pointed to by list nodes.
// 10/20/95 RFH
void restable::FreeAll(void)
{
#ifdef MEMCHECK
	cout << "   restable::FreeAll()- " << elements.length() << " nodes: ";
	print();
	cout << "\n";
#endif

	where =NULL;
	lastindex=invalid_index;

resource *r;
	while (elements.length() > 0)
	{
		r = elements.remove_front();
		if (r)
		{
#ifdef MEMCHECK
			cout << "  - free " << r->ClassName() << " " << r->Name();
			if (r->ClassName() == "String")
				cout << " = \"" << ((R_String *)r)->Value();
			cout << "\"\n";
#endif
			int ex = r->Exists();

			if (!(ex & (exist_perm | exist_read_only)))	// never delete permanents
			{
				// record deletion statistics
				if ((ex & exist_temp))
					temp_destroys++;
				else
				if ((ex & exist_local))
					local_destroys++;

				delete r;
			}
		}
	}
}


// DestroyResList
// Non restable member function
void DestroyResList(resource *owner, SLList<resource *>& thelist)
{
	resource *r = NULL;
	int ex;
	Pix temp = thelist.first();
	for(; temp; thelist.next(temp))
	{
		r = thelist(temp);
		ex = r->Exists();
		if ((ex & exist_temp))
			temp_destroys++;
		else
		if ((ex & exist_local))
			local_destroys++;

		if (r != owner)
			delete r;
	}
}



// _Flag
// Set or unset an existence flag for all resources in this list.
// Easier to invoke with Flag and DeFlag inline functions.
void restable::_Flag(int f, int on)
{
Pix temp = elements.first();
resource *r = NULL;
	for(;temp; elements.next(temp))
	{
		r = elements(temp);
		if (r)
		{
			if (on)
				r->Flag(f);
			else
				r->DeFlag(f);
		}
	}
}

resource *restable::Create(String &nm, resource*& table)
{
	table = NULL;
	return new restable(nm);
}

// AllTrueOrFalse
// return R_Boolean(1) if all elements of the list evaluate to 
// the condition described by "which": 1 = true, 0 = false.
// RFH 1/29/96
resource *restable::AllTrueOrFalse(int which)
{
R_Boolean *rb = new R_Boolean;
Pix temp = elements.first();
resource *r = NULL;

	for(;temp; elements.next(temp))
	{
		r = elements(temp);
		if (r->LogicalValue() != which)
		{
			rb->Set(1-which);
			return rb;
		}
	}

	rb->Set(which);
	return rb;
}

// Assoc
//	Assoc(List a, String fn)
// -For each element en of this list, apply the function fn with element
//	an of the argument list a. Return a List of the results.
//	 example:
//	suppose this list (::elements) is {e1, e2, e3}
//	and the argument list (a) is {a1, a2, a3}.
//	Then the results table is { e1.fn(a1), e2.fn(a2), e3.fn(a3) }
//
//	* If the lists are of different lengths, the mapping stops for the shorter
//	  of the two lists.
//	Soon, fn may also be a list of functions.
// RFH 1/29/96
resource *restable::Assoc(SLList<resource *>& args)
{
	if (args.length() < 2)
	{
		cerr << "List.Assoc requires two arguments: a List of arguments and a function name.\n";
		return NULL;
	}
	
restable *argtable = NULL;
String function;
resource *r = NULL;

	Pix argp = args.first();
	r = args(argp);
	if (r && r->ClassName() == "List")	// argument 1: list of arguments
		argtable = (restable *) r;
	args.next(argp);
	r = args(argp);
	if (r && r->ClassName() == "String")	// argument 2: function name
		function = r->Value();

	if (!argtable || function.length() == 0)
	{
		cerr << "List.Assoc: Error finding List of arguments or function.\n";
		return NULL;
	}

	SLList<resource *>& alist = argtable->GetList();
	restable *resultable = new restable;
	Pix etemp = elements.first();
	Pix argtemp = alist.first();
	resource *obj = NULL, *objarg = NULL, *result = NULL;
	SLList<resource *> execargs;

	for( ; etemp && argtemp; elements.next(etemp), alist.next(argtemp))
	{
		obj = elements(etemp);
		objarg = alist(argtemp);
		
		if (obj && objarg)
		{
			execargs.clear();	// only one argument
			execargs.append(objarg);
			result = obj->execute(function, execargs);
			if (result)
				resultable->Add(result);
		}
	}
	return resultable;
}


// distribute
// distribute a object.method call to all elements of the
// table that are of the same type as the type as the specified
// object-type.  Expects 2 arguments.  Eg,
//
// 		thetable.distribute(String, "upcase");
//
// would call the "upcase" method of every restable element of type
// String.
//
//		thetable.distribute(String, "append", " hey", xyz);
//
// would apply "append" with the args (" hey", xyz) to each element
// of type String.
//
// RFH 9/25/95

/*******************

resource *restable::Distribute(SLList<resource *>& args)
{
	if (args.length() < 3)
	{
		cout << "\nList::distribute-- not enough arguments.\n";
		return NULL;
	}

Pix t1 = args.first(), temp;
resource *first, *second, *third;
String type, method;
resource *table_rsrc = NULL;

	first = args(t1);	args.next(t1);
	second = args(t1);	args.next(t1);
	third = args(t1);	args.next(t1);

	if (first->ClassName() != "List")	// variable table
	{
		cout << "\nList::distribute - List expected for first argument.\n";
		return NULL;
	}

	//
	// This code is commented out
	//
	
	restable *vars = (restable *) first;

	type = second->ClassName();
	method = third->Value();
	if (third->ClassName() != "String")	// has to be a string
	{
		cout << "\nList::distribute - String expected for third argument.\n";
		return NULL;
	}
	
	ResCall *rc = NULL;
	String txt;
	SLList<resource *> newargs;
	
	// create new argument list
	for(; t1; args.next(t1))
		newargs.append(args(t1));

	//
	// This code is commented out
	//

	for(temp=elements.first(); temp; elements.next(temp))
	{
		table_rsrc = elements(temp);
//		cout << "\n working on ";
//		table_rsrc->print();
//		cout << '\n';
		
		resource *passback = NULL;
		if (table_rsrc && table_rsrc->ClassName() == type)
		{			
//			cout << "\nexecuting: " << table_rsrc->ClassName()
//				 << ' ' << table_rsrc->Name() << '.'
//				 << method << '\n';
//			cout.flush();
			passback = table_rsrc->execute(method, newargs);
		}
	}
	return NULL;	// something else?  this?
}
****************************/

// Distribute - new 10/19/95 (RFH)
// - note the void return type
// Jan 24/96: added type check option 
void restable::Distribute(SLList<resource *>& args, int typecheck)
{
	if (args.length() < 2 && typecheck)
	{
		cout << "\nList::distribute-- not enough arguments.\n";
		return;
	}
	if (args.length() < 1 && !typecheck)
	{
		cout << "\nList::distribute-- not enough arguments.\n";
		return;
	}
	if (elements.length() < 1)	// this restable is empty
		return;	// no point in continuing

Pix t1 = args.first(), temp;
resource *first, *second;
String type, method;
resource *table_rsrc = NULL;

	first = args(t1);
	
	if (typecheck)
	{
		args.next(t1);
		second = args(t1);	args.next(t1);

		/* Find the type of element to operate on */
		if (first->ClassName() != first->Name())
			type = first->Name();	// eg, x.distribute("List", ... )
		else
			type = first->ClassName();	// eg, x.distribute(List, ... )

		/* find the method to apply */
		method = second->Value();
		if (second->ClassName() != "String")	// has to be a string
		{
			cout << "\nList::distribute - String expected for second argument.\n";
			return;	// **log
		}
	}	
	else
	{
		method = first->Value();
		args.next(t1);
	}

//	ResCall *rc = NULL;
	String txt;
	SLList<resource *> newargs;
	
	// create new argument list
	for(; t1; args.next(t1))
		newargs.append(args(t1));

	restable *dist_temp_results = new restable;
	temp_tables_created++;
	int correctType = 0;

	for(temp=elements.first(); temp; elements.next(temp))
	{
		table_rsrc = elements(temp);
		resource *theresult = NULL;

		if (!table_rsrc)
			continue;
	
		correctType = (table_rsrc->ClassName() == type) ? 1:0;

		if (!typecheck || (typecheck && correctType))
		{
			theresult = table_rsrc->execute(method, newargs);
			// add temporary results to the temp table,
			// even though they cannot be used elsewhere
			// and could be destroyed right here.
			if (theresult && theresult->Exists() == exist_temp)
			{
				if (dist_temp_results)
				{
#ifdef MEMCHECK
					cout << "distribute: add temp result- class: " << theresult->ClassName()
						 << " name: " << theresult->Name();
					if (theresult->ClassName() == "String")
						cout << " value: " << ((R_String *)theresult)->Value();
					cout << '\n';
#endif
					// Add not install_resource - allow duplicate names
					dist_temp_results->Add(theresult);
					temp_adds++;
				}
			}
		}
	}
	if (dist_temp_results)
	{
#ifdef MEMCHECK
		cout << "  distribute:  calling FreeAll():\n";
#endif
//		dist_temp_results->CleanUp();	// delete all temporary results
//		dist_temp_results->FreeAll();	// only temporaries were added to the table
	}
	delete dist_temp_results;
	temp_tables_destroyed++;
}

// Delete
// Delete a resource by name, value, or index
// - `how' is one of {"name", "value", "index", "pointer"} (chopped from the method name in RSL)
// - only deletes the node, not the resource.
// - returns a pointer to the resource removed from the list.
// RFH 12/19/95 -written to support clone self-pointers in the App system
resource *restable::Delete(String what, String how, resource *res)
{
int ind=-1;	// initially invalid.

	lastindex = invalid_index;
	where = NULL;
	
	char chow = tolower(how[0]);
	if (chow == RLD_INDEX)	// RLD_ stuff is from SLList_res_util.h
		ind = atoi((const char *) what);

	return RLD_realwork(elements, what, chow, ind, res);
}

// remove_front
// removes and returns the first element in the list. If no items in the list, return NULL.
resource* restable::remove_front(void)
{
	lastindex = invalid_index;
	where = NULL;
	
	if (elements.length() > 0)
		return elements.remove_front();
	
	return NULL;
}

// front
// return the resource at the front of the list
resource* restable::front(void)
{
//	if (elements.length() > 0)
	if (elements.first() != NULL)
		return elements.front();
	
	return NULL;
}

// Assign
// set the contents of the restable
// - if the resource is temporary (ie, as in the result of a call)
//   then it is destroyed as its contents are added
resource *restable::Assign(resource* r)
{
	if (!r)
		return NULL;
	
	// if (r->ClassName() == "List")	// later: HierarchyContains(":container:")
	if (r->HierarchyContains(":List:"))	// it is a restable
	{
		SLList<resource *> &x = ((restable *) r)->GetList();
		elements = x;	// element-by-element copy of x;
	}
	else
	// wasn't given a List, so we'll just make the given resource be
	// the only resource in this list. Is this OK or error-prone??
	{
	//	CleanUp();	// delete temporaries
		elements.clear();

		resource_adds++;
		elements.append(r);
	}

	lastindex=invalid_index;
	where = NULL;

	return this;
}

void restable::Insert(resource *r)
{
	resource_adds++;
	if (r)
		elements.prepend(r);
	
	if (lastindex > invalid_index)	// bump fast index shortcut
		lastindex++;
}

void restable::Insert(SLList<resource *>& args)
{
Pix temp=args.first();
	if (args.length() > 0)
		for(; temp; args.next(temp))
		{
			resource_adds++;
			elements.prepend(args(temp));

			if (lastindex > invalid_index)	// bump fast index shortcut
				lastindex++;
		}
}

// Add
// add a resource
// If it is an R_File, then we want to add the contents of that file to
// this list, which creates one R_String resource
void restable::Add(resource *r)
{
	if (!r)
		return;
	elements.append(r);
	resource_adds++;
}

void restable::Add(String &r)
{
	resource_adds++;
	resource *rsrc = new R_String("", r.chars());
	elements.append(rsrc);
}

void restable::Add(const char *r)
{
	resource_adds++;
	resource *rsrc = new R_String("", r);
	elements.append(rsrc);
}

void restable::install_varlist(SLList<stringpair>& vars)
{
Pix temp = vars.first();
R_String *i;
String name, val;

	while(temp)
	{
		name = vars(temp).left();
		val = vars(temp).right();
		i = new R_String(name, val);
		Add(i);
		vars.next(temp);
	}
}

void restable::install_varlist_table(String nm, SLList<stringpair>& vars)
{
Pix temp = vars.first();
R_String *i;
String name, val;
restable *rt = new restable(nm);

	while(temp)
	{
		name = vars(temp).left();
		val = vars(temp).right();
		i = new R_String(name, val);
		AddOrReplaceStrings(i);	// replace existing values in this restable
		
//		rt->install_resource(i);
		rt->Add(i);	// Add to new restable
		vars.next(temp);
	}
	Add(rt);
}

void restable::install_varlist_table(SLList<stringpair>& vars, restable *rt)
{
Pix temp = vars.first();
R_String *i;
String name, val;

	if (!rt)
	{
		cerr << "restable: install_varlist_table got NULL table!\n";
		return;
	}

	while(temp)
	{
		name = vars(temp).left();
		val = vars(temp).right();
		i = new R_String(name, val);
		AddOrReplace /*Strings*/ (i);	// replace existing values in this restable
		
		rt->Add(i);	// Add to new restable
		vars.next(temp);
	}
	Add(rt);
}

// AddOrReplaceStrings
// does just what it sez
// This is called chiefly from install_varlist_table (above)
// -- for doing things like installing CGI variables when it is good
// practice to declare them in the script ahead of time.
// RFH 1/12/96
// **********************************************************************
// *** This function will go away soon! Superceded by AddOrReplace(). ***
// **********************************************************************
void restable::AddOrReplaceStrings(resource *r)
{
//	cout << "AddOrReplace ";
	if (!r)
		return;
	resource *ther = GetResource(r->Name());
	
	if (ther)
	{
//		cout << "Found Existing: \"" << ther->Name() << "\"<br>";
		if (ther->ClassName() == "String")
			((R_String *)ther)->Assign(r->Value());	// Replace existing value
//			ther->Assign(r);	// Replace existing value
	}
	else
		Add(r);
}

// AddOrReplace
// Add r to this restable if a resource of the same name doesn't already exist,
// or assign the existing resource to r.
//
// Return 1 if it was added, 0 if an existing resource was assigned
// its value ("replace").
//
// RFH 1/23/96
// Return value added March 9 1996
int restable::AddOrReplace(resource *r)
{
	if (!r)
		return -1;
	
	resource *ther = GetResource(r->Name());
	if (ther)
	{
		ther->Assign(r);
		return 0;
	}

	Add(r);
	return 1;
}

void restable::print(void)
{
	print("");
}
void restable::print(String delim)
{
//	if (elements.length() == 0)
	if (elements.first() == NULL)
		return;

	resource *r = elements.front();
	// special formatting for scripts
	int do_script = (r->ClassName() == "ResCall");

	if (do_script)
	{
		cout << name << "\n{\n";
		resource::print(elements, do_script?"    ":"");	// indent script text
		cout << "\n}\n";
	}
	else
	{
		Pix temp = elements.first();
		resource *r = NULL;
		for(; temp; elements.next(temp))
		{
			r = elements(temp);
			if (!r)
				continue;
			if (!(r == this))	// check for self reference!! (only applicable to containers)
				r->print();	// print the resource
			// print a newline for String if there wasn't one
			if (r->ClassName() == "String")
			{
				String v= r->Value();
				if (v.length() == 0 || v[v.length()-1] != '\n')
					cout << '\n';
			}
			cout << delim;
		}
	}
}

/********************************************************************
*																	*
*	restable::PrintTable											*
*	- print	a table	of the installed resources,	in three columns:	*
*		Object class, Instance name, Remarks.						*
*	- the Remarks field	is intended	to be a	free-form place	for		*
*	  general information supplied by the resource itself.	It is	*
*	  likely to	be a sub-table or a	list.							*
*	RFH	9/12-13/95													*
*																	*
********************************************************************/
void restable::PrintTable(int html_out)
{

#ifdef WEB

Pix temp = elements.first();
resource *r;
String cname, iname, val;
htmltable tb;

	if (html_out)
	{
		tb.SetBorder(1);
		tb.SetCellPadding(3);
		tb.AddCharRow(3, "<b>Object class</b>", "<b>Instance name</b>",
			"<b>Value</b>");
	}
	while(temp)
	{
		r = elements(temp);
		if (r)  {
			//	r->print();
			cname = r->ClassName();
			iname = r->Name();
			val = r->Value();
			if (html_out)
				tb.AddRow(3, &cname, &iname, &val);
			else
				cout << cname << ' ' << iname << " = " << val << '\n';
		}
		elements.next(temp);
	}
	if (html_out)
	{
		SLList<String> *newtable = tb.GetTable();
		cout << *newtable;
	}
	else
		cout << '\n';

#endif	// WEB

}

String restable::Value(void)
{
	String x = dec(elements.length()) + String(" resources.");
	return x;
}

/*
 * print the script text, optionally formatted in an HTML table
 *
 */
void restable::PrintScript(int html_out)
{

#ifdef WEB

Pix temp = elements.first();
String s;
htmltable tb;

	if (html_out)
	{
		tb.SetBorder(1);
		tb.SetCellPadding(3);
		s = "<b>Script " + name + "</b><br>";
		tb.AddRow(1, &s);
	}
	while(temp)
	{
		resource *r = elements(temp);
		if (r && r->ClassName() == "ResCall")
		{
			((ResCall *) r)->TextEquiv(s, 1);
			if (html_out)
			{
				s.prepend("<tt>");
				s += ";</tt><br>";
				tb.AddRow(1, &s);
			}
			else
				cout << s << ";\n";	// end of statement
		}
		elements.next(temp);
	}
	if (html_out)
		cout << *(tb.GetTable());
	else
		cout << '\n';

#endif	// WEB

}

/* Execute a "script"
 *
 * Steps through all resources in the table and calls their
 * execute(restable *) method.  Note: this is not the "real"
 * execute!  Real execution happens within a ResCall.
 *
 * Originally in the "script" class, which went away
 * RFH 9/13/95 (written) 9/23/95 (moved)
 * 10/19/95 - modified for 4 variable restables
 */
resource * restable::execute(
	restable *arguments,
	restable *globals,
	restable *globals2,
	restable *givenlocals /* == NULL by default */,
	restable *restemps /* == NULL by default */)
{
Pix temp = elements.first();
String s;

	/* Create a local variable table if not given one */
	restable *locals = NULL;
	if (givenlocals)
		locals = givenlocals;
	else
	{
		locals = new restable;
		local_tables_created++;
	}

	/* Create a temporary variable table if not given one */
	restable *result_temps = NULL;
	if (restemps)
		result_temps = restemps;	// use the given table for temporary results
	else
	{
		result_temps = new restable;	// make a new table for temporary results
		temp_tables_created++;
	}

	
	/* Execute each ResCall */
	int Done = 0;
	
	resource *rc = NULL;
	resource *retval = NULL;
	while(!Done && temp)
	{
		rc = elements(temp);
		if (!rc) continue;

		if (rc->ClassName() == "ResCall")
			retval = ((ResCall *) rc)->execute(arguments, globals, globals2,
							locals, result_temps);
		else
			if (rc->ClassName() == "rc_command")
				retval = rc;	// examine it later on...
		else
			if (rc->ClassName() == "List")	// does this situation _ever_ occur???
			{
				cerr << "Argument Eval: ResCall pointed to List to execute\n";
				retval = ((restable *)rc)->execute(arguments, globals, globals2, locals);
			}

		// An executing restable is a consumer of the rc_command 'return'.
		// Pass up break and continue though.
		if (retval && retval->ClassName() == "rc_command")
			//	if ( ((rc_command *) retval)->Type() > rc_command::NoOp)
			{
#ifdef DEBUG
				cout << "Restable: Got an rc command, breaking from execution..\n";
#endif
				Done = 1;
				break;
			}

		retval = NULL;	// reset ...
		elements.next(temp);
	}
	
	/* Free allocated memory */
	
	if (!givenlocals && locals)	// if a local table was created here
	{	// free local variables
#ifdef MEMCHECK
		cout << "  restable::execute()- FreeAll() for locals table\n";
#endif
		locals->FreeAll();
		delete locals;
		local_tables_destroyed++;
	}
	
	if (!restemps && result_temps)	// if a temp restable was created here
	{	// free temporaries
#ifdef MEMCHECK
		cout << "  restable::execute()- FreeAll() for temp table\n";
#endif
		result_temps->FreeAll();
		delete result_temps;
		temp_tables_destroyed++;
	}
	
	// Bounce up rc_command...

	if (retval && retval->ClassName() == "rc_command")
		if ( ((rc_command *) retval)->Type() == rc_command::Return)
		{
			// would be- return retval->RetVal();
			// Only consume the Return if we are a named restable!!
			// That means we are a subroutine- a return is valid only for the
			// enclosing subroutine.
			if (name.length() > 0)
				retval=NULL;
		}

	return retval;	// may be NULL--- may be an rc_comand::Break or Continue. or NULL.
}

// C++ operator []
// return element whose index is given.
// -1 returns the last element.
// RFH - extracted from restable::Index() below, 2/2/96
resource *restable::operator[](int i)
{
	if (i >= elements.length() || i < -1)	// -1 signifies last element
	{
//		cout << "List \"" << name << "\": Index out of bounds.\n";
		return NULL;
	}

	if (i == 0)	// fast first element access
	{
		lastindex=0;
		where = elements.first();
		return (where? elements.front() : NULL);
	}
		
	/* easy access to the last element */
	if (i == -1)	// maybe -n should mean (n-1) elements from the last one
	{
		lastindex = invalid_index;
		where = NULL;
		return elements.rear();
	}

	// other elements: search through the list
	int a;
	Pix temp;
	
	if (where != NULL && i == lastindex+1 && lastindex > invalid_index
		&& (lastindex < elements.length()-1)) 
	{
		a = lastindex;
		temp = where;
	}
	else	// start from the beginning.
	{
		a = 0;
		temp = elements.first();
	}

	for(a; a<i; a++)	// count through the list nodes
		elements.next(temp);

	lastindex = a;	// set fast access shortcut
	where = temp;

	return elements(temp);
}


// Index : the RSL [] operator
// Return a resource based on an argument index value.
// If the argument is a String, return the resource of that name
// from this table.
// If the argument is an Integer, return the resource with that index
// in the list.
resource *restable::Index(resource *r)
{
resource *it = NULL;
	if (!r) return NULL;

			/* index by string */
	if (r->ClassName() == "String")
		it = GetResource(r->Value());
	else
			/* index by integer */
		if (r->ClassName() == "Integer")
		{
			return operator[](((R_Integer *)r)->intval());
		}

	return it;
}

// Sort
// list interface to Sort(String meth, int order) below.
// - extracts the method name from the arg list.
resource *restable::Sort(SLList<resource *>& args, int order)
{
String meth;
//	if (args.length() > 0)
	if (args.first() != NULL)
	{
		resource *f = args.front();
		if (f && f->ClassName() == "String")
			meth = f->Value();
	}
	return Sort(meth, order);
}

// Sort
// inserts each list element into a binary tree (type restree)
// and returns a new list from an inorder traversal.
resource *restable::Sort(String meth, int order)
{
restree tree;

Pix temp = elements.first();
resource *r = NULL;

	for(; temp; elements.next(temp))
	{
		r = elements(temp);
		if (!r) continue;
		tree.Insert(r, (restree::order) order, meth);
	}
	// tree.print();
	restable *ret = new restable;
	tree.GetList(ret->GetList());
	return ret;
}

