/*
 $Header: /mongo/dest/crb/RCS/restable.h,v 1.9 1996/03/18 13:26:43 holtrf Exp holtrf $
*/

/*****************************************************************************

 restable.h
 
 A singly-linked list of resources, which is itself a resource.
 featuring complex LISP-like operations including sorting, multiple list
 association & function mapping. A restable is an executable RSL program
 when the resources it contains are ResCalls.

 Russell Holt - began life on July 24 1995 in the old `CRB' system.

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: List

RELATED RESOURCES:

DESCRIPTION: a list of resources

PUBLIC MEMBER FUNCTIONS:
	
	
	Assign(List L)
	=
		Assign this list to L. Clears the previous contents. If argument is
		not a List, then that element becomes the sole resource in
		the List.

	Append(...)
	Add(...)
		Append argument(s) to the end of the list.

	Prepend(...)
	Insert(...)
		Insert argument(s) to the front of the list. Note that
		mylist.prepend(a, b, c, d); will result in the arguments
		appearing in the list in the reverse order that they are in
		the argument list (because it first prepends a, then b, ...)
	
	Integer Length
		return Integer length of the list.

	Distribute, DistributeType (<TYPE>, String method, arg1, arg2, ..., argn)
		apply method and remaining arguments to each element of the list.
		Only if the element matches the given type.
		
		Example:
			List x;
			x.append("one", "two", "three", "eleven");	// it goes to eleven.

			// Capitalize each String element.
			x.distribute(String, "Capitalize");
			
	DistributeAll (String method, arg1, arg2, ..., argn)
		Same as Distribute, DistributeType, except the method is applied
		to all resources in the List.

		Example:
		
			// append " and..." to each element of x.
			x.distributeAll("append", " and...");

	Assoc(List L, String method)
		For each element `en' of the List, apply method `method' with element
		Ln of the argument List L. Return a List of the results.

		Example:
			suppose this List is {e1, e2, e3}
			and L is {L1, L2, L3}.
			Then the results table is { e1.m(L1), e2.m(L2), e3.m(L3) }

		Real example:
			List x, a, r;
			x.append("string 1", "string 2", "string 3");
			a.append("1", "2", "3");
			r = x.Assoc(a, "contains");	// match a with x via "contains"
			if (r.AllTrue())
				out.print("Matched successfully.");
			else
				out.print("Nope.");

		If the lists are of different lengths, the mapping stops for the
		shorter of the two lists.
	
	Print
		Print the list. Optional String argument serves as a delimiter
		to be printed between items such as "\n" for a vertical column,
		", " for a `list', etc.

	[]
	Index
		List element access. Two argument types:
			String:  return named element.
			Integer: return element at that index # (zero based)
	Example:
		List l;
		l.append(1, a:2, 3, 4);
		out.print("zero element: ", l[0], endl);	// gives 1
		out.print("element 'a': ", l["a"], endl);	// gives 2
		out.print("element # 2: ", l[2], endl);	// gives 3

	DeleteByName(String name)
		Delete named element from the list.

	DeleteByValue(String value)
		Delete element from the list whose String value is the same as
		the String value of the argument.

	DeleteByIndex(Integer index)
		Delete element from the list at position `index'.

	AllTrue
		return true if all list elements evaluate to true.

	AllFalse
		return true if all list elements evaluate to false.

	Clear
		removes elements from the list (does not destroy them)
		
	ClearAndDestroy
		delete all elements from the list and destroy them.


 <<End Resource Documentation>>
 ****************************************************************************/

 
/*****************************************************************************
 <<Begin Class Documentation>>


 CLASS NAME:  restable

 DERIVED FROM: resource

 RELATED CLASSES: SLList

 DESCRIPTION:

	 A singly-linked list of resources, which is itself a resource.
	 featuring complex LISP-like operations including sorting, multiple list
	 association & function mapping. A restable is an executable RSL program
	 when the resources it contains are ResCalls.

 PUBLIC DATA MEMBERS:

	<name>
		documentation including data definition, valid values


 PROTECTED DATA MEMBERS:

	<name>
		documentation


 PRIVATE DATA MEMBERS:

	<name>
		documentation


 CONSTRUCTORS AND DESTRUCTORS

	<function prototype>

		documentation

 OPERATORS:
	
	<operator function prototype>

		documentation

 PUBLIC MEMBER FUNCTIONS:

	<function prototype>
		documentation including input/output explanation

 PROTECTED MEMBER FUNCTIONS:

	<function prototype>
		documentation including input/output explanation

 SEE ALSO: <SLList.h>, resource.h
 

 <<End Class Documentation>>
 ****************************************************************************/


#ifndef _RES_TABLE_H_
#define _RES_TABLE_H_

// INCLUDES

#include "resource.h"
#include <SLList.h>
#include <String.h>

/*****************************************************************************

		  	   	CLASSES

 ****************************************************************************/

class restable : public resource {
	enum { invalid_index=-1 };
	SLList<resource *> elements;
	Pix where;
	int lastindex;

	void Init(void);
	void rslAdd(SLList<resource *> &e);

public:
	restable(void);
	restable(String &nm);
	restable(const char *nm);
	
	
	
	resource *Create(String &nm, resource*& table);

	resource *GetResource(String& resname);
	resource *lookup(String& resname);
	resource *lookup(const char *resname);
	
	String Value(void);

	resource *execute(String& method, SLList<resource *> &args);

	resource *execute(restable *arguments, restable *globals, restable *globals2,
		restable *givenlocals=NULL, restable *restemps=NULL);
	
	void Clear(void) { elements.clear(); where=NULL; } // deletes list nodes only
	void CleanUp(void);
	void FreeAll(void);

	void Add(SLList<resource *> &e);
	void Add(resource *r);
	void Add(String &r);
	void Add(const char *r);
	void install_varlist(SLList<stringpair>& vars);
	void install_varlist_table(String nm, SLList<stringpair>& vars);
	void install_varlist_table(SLList<stringpair>& vars, restable *rt);
	void install_resource(resource *r);
	void AddOrReplaceStrings(resource *r);
	int AddOrReplace(resource *r);

	resource *Delete(String what, String how, resource *res=NULL);

	resource *Assign(resource *r);
	
//	resource *Distribute(SLList<resource *>& args);
	void Distribute(SLList<resource *>& args, int typecheck=0);
	resource *Assoc(SLList<resource *>& args);
	resource *AllTrueOrFalse(int which);

	void Insert(SLList<resource *>& args);
	void Insert(resource *r);
	
	resource *Sort(SLList<resource *>& args, int order);
	resource *Sort(String method, int order);
	
	inline void Flag(int f) { _Flag(f, 1); }
	inline void DeFlag(int f) { _Flag(f, 0); }
	void _Flag(int f, int on);

	void print(void);
	void print(String delim);
	void PrintTable(int html_out=1);
	void PrintScript(int html_out=1);

	SLList<resource *> &GetList(void) { return elements; }
	
	resource *operator[](int i);
	resource *Index(resource *r);
	resource *remove_front(void);
	resource *front(void);
	inline int length(void) {	return elements.length();	}
	
	
	
// iterators
	inline resource *BeginIteration(void)
		{	where = elements.first();
			lastindex = 0;
			if (where)
				return elements(where);
			return NULL;
		}
	inline resource *GetNextResource(void)
		{	if (where && lastindex < elements.length() )
			{	lastindex++;
				elements.next(where);
				if (where)
					return elements(where);
			}
			lastindex = invalid_index;
			return NULL;
		}
	inline Pix GetLocation(void) { return where; }
	inline void RestoreLocation(Pix w) { where = w; }
	
};

#endif
