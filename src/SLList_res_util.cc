/*
 *	NAME
 *		SLList_res_util.c
 *
 *	WHAT
 *		A collection of useful functions for reading/printing
 *		lists of resources.
 *
 *	Russell Holt
 *
 */

#include "SLList_res_util.h"
extern "C" {
#include <stdio.h>          // Standard I/O
#include <stddef.h>         // Standard Definitions
#include <stdlib.h>         // Standard Library
#include <stdarg.h>
#include <stdio.h>
}

// Read a bunch of Strings from an ifstream,
// make them into string resources, and add them to
// the given SLList.  They become unnamed resources...
void GetStringResources(fstream& in, SLList<resource *>& stuff)
{
	String s;
	char *inp = NULL;
	R_String *rn = NULL;
	while(in.good())
	{
//		in >> s;

		// read arbitrarily long line, up to '\n' (default delimiter)
		// - it allocates all memory needed.
		in.gets(&inp);
		rn = new R_String("", inp);	// s.chars());
		if (rn)
			stuff.append(rn);
	}
}

// printreslist
// -- print each resource in a list.
void printreslist(SLList<resource *>& stuff)
{
Pix temp = stuff.first();
resource *r =NULL;
	for(; temp; stuff.next(temp))
	{
		r = stuff(temp);
		if (r)
		{
			r->print();
			cout << '\n';
		}
	}
}

// printresnamelist
// -- print each resource in a list, in the form
// (name, value)
void printresnamelist(SLList<resource *>& stuff)
{
Pix temp = stuff.first();
resource *r =NULL;
	for(; temp; stuff.next(temp))
	{
		r = stuff(temp);
		if (r)
			cerr << '(' << r->Name() << ','
				 << r->Value() << ")\n";
	}
}


// printordered - simplified interface to the version explained below
void printordered(SLList<resource *>& stuff)
{
	printordered(stuff, NULL);
}

// printordered
// prints the resource list in the form:
//
// 0. <class0> <name0> <value0>
// 1. <class1> <name1> <value1>
// ...
// number, class, and name are left justified in a fixed printed length
// to columnize it.
//
// If `type' is non-NULL, then only the objects of class `type' are
// printed in the list.
//
// RFH 12/19/95
// `type' added March 5 1996
void printordered(SLList<resource *>& stuff, const char *type)
{
Pix temp = stuff.first();
resource *r =NULL;
int ind=0;
	for(; temp; stuff.next(temp))
	{
		r = stuff(temp);
		if (!r) continue;
		if(!type || r->ClassName() == type)
		{
			// - means left justified
			cout.form("%3d. %-8s %-20s %s\n", ind++,
				(const char *) (r->ClassName()),
				(const char *) (r->Name()),
				(const char *) (r->Value()));
		}
	}
}


/******************************************************************
 *
 *  Delete functions
 *
 ******************************************************************/

// Delete a resource by name, value, or index
// - `how' is one of {"name", "value", "index", "pointer"} (chopped from the method name in RSL)
// - only deletes the node, not the resource.
// - returns a pointer to the resource removed from the list.
// RFH March 4 1996 -- extracted from restable::Delete

// Var-arg version.
resource *ResListDelete(SLList<resource *>& elements, char how, ...)
{
resource *res=NULL;
int ind=-1;	// `ind' initial value: -1 is invalid index.
String what;
va_list ap;	// going to do var-args
	
	va_start(ap, how);
	switch(how)
	{
		case RLD_INDEX:	ind = va_arg(ap, int); break;
		case RLD_NAME:	what = va_arg(ap, char *); break;
		case RLD_POINTER:	res = va_arg(ap, resource *); break;
		case RLD_VALUE:	what = va_arg(ap, char *); break;
		Default: ;
	}
	va_end(ap);

	return RLD_realwork(elements, what, how, ind, res);
}

// RLD_realwork
// the real deletion
resource *RLD_realwork(SLList<resource *>& elements, String& what, char how,
	int ind, resource *& res)
{
Pix temp=elements.first(), prev=NULL;
int found=0, i=0;	// `ind' initial value: -1 is invalid index.
resource *r=NULL;

	// find the resource - maintain a pointer to the previous node during traversal
	for(; temp; prev=temp, elements.next(temp), i++)
	{
		r = elements(temp);
		if (r)
			switch(how)
			{
				case RLD_INDEX: found = (i == ind?1:0); break;
				case RLD_NAME: found = (r->Name() == what?1:0); break;
				case RLD_POINTER: found = (r == res?1:0); break;
				case RLD_VALUE: found = (r->Value() == what?1:0); break;
				default: ;
			}

		if (found)
		{
			elements.del_after(prev);	// if prev is 0, the first item is deleted
			return r;	// done
		}
	}
	
	return NULL;	// not found.
}

