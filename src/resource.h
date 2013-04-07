/* $Id: resource.h,v 1.5 1996/02/06 19:15:59 holtrf Exp $
 *
 * PURPOSE: Resource class definition.
 *
 * PROGRAMMING NOTES:
 *   resource is an semi-abstract class that defines data
 *   and functions common to all resource classes.
***
$Log: resource.h,v $
Revision 1.5  1996/02/06 19:15:59  holtrf
New type definitions ResList, ResListRef


***
 * Written by Russell Holt
 * Copyright 1995,1996 by Destiny Software Corporation.
 */

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <stream.h>
#include <String.h>
#include <SLList.h>
#include "../server/stringpair.h"
#include <fstream.h>

#define exist_temp 1
#define exist_local 2
#define exist_perm 4
#define exist_read_only 8
#define exist_add_to_locals 16

class resource {
protected:
	String name;
	String class_name;
	String hierarchy;	// ":parentclass:childclass1:childclass2: ... :childclassn:"
	int existence;
	int enabled;


public:
	resource(void);
//	resource *Create(SLList<resource *>& args);	// obsolete
//	void Create(resource *the_res, SLList<resource *>& args, resource *vars);	// obsolete
//	virtual resource *Create(String &nm);	// obsolete

	virtual resource *Create(String &nm, resource*& table);
//	virtual resource *Clone(void) { return new resource(*this); }

	inline int Is(String &what) { return (name == what); }

	inline String Name(void) { return name; }
	inline String ClassName(void) { return class_name; }

	inline String Heirarchy(void) { return hierarchy; }
	inline int HierarchyContains(char *s) { return hierarchy.contains(s); }
	inline int HierarchyContains(String& s) { return hierarchy.contains(s); }

	inline void SetName(String& nm) { name = nm; }
	inline void SetName(char *nm) { name = nm; }
	inline void SetClassName(String& c) { class_name = c; }	// remove these calls?
	inline void SetClassName(char *c) { class_name = c; }	// (subclasses have direct access)
	
	inline int Exists(void) { return existence; }
	inline void MakeExist(int e) { existence = e; }
	inline void Flag(int f) { existence |= f; }
	inline void DeFlag(int f) { if (existence & f) existence -= f; }
	
	virtual String Value(void) { return name; }
	
	virtual resource *execute(String& method, SLList<resource *> &args);
	virtual resource *execute(void) { }
	virtual void print(void);
	void print(SLList<resource *> things, char* indent="");
	
	virtual int LogicalValue(void) { return 1; }
	virtual resource* Assign(resource *) { return NULL; }
	
	resource *GetSet(String& themember, resource *f);
	resource *GetSet(int& themember, resource *f);

	
// - - - - - - - - - - - - - -
	// The comparison virtuals should only be here if they are actually
	// used as virtuals. Maybe this suggests a sub hierarchy of semi-primitive types
	// like String or Integer which might actually benefit from this....
		virtual resource *Equal(resource *f) { return NULL; }
		virtual resource *And(resource *f) { return NULL; }
		virtual resource *Or(resource *f) { return NULL; }
		virtual resource *LessThan(resource *f) { return NULL; }
		virtual resource *GreaterThan(resource *f) { return NULL; }
		virtual resource *NotEqual(resource *f) { return NULL; }
		virtual resource *LessThanEqual(resource *f) { return NULL; }
		virtual resource *GreaterThanEqual(resource *f) { return NULL; }
		virtual resource *Not(resource *f) { return NULL; }
// - - - - - - - - - - - - - -

	int IsComparison(String &m);
	resource *rTrue(void);
	resource *rFalse(void);
	
	inline void SetEnable(int i) { enabled = i; }
	inline int Enabled(void) { return enabled; }
	
	virtual resource *rslRead(fstream &infile) { return NULL; }
	virtual void ResError(const char *s);

//	friend inline ostream& operator<<(ostream& out, resource& r);
};

// type definitions for resource lists

typedef SLList<resource *> ResList;
typedef ResList& ResListRef;


// Some useful access functions

inline String& ArgName(stringpair &arg)	// obsolete
{	return arg.left(); }

inline String& ArgVal(stringpair &arg)	// obsolete
{	return arg.right(); }

inline void printargs(SLList<stringpair> &args)
{
Pix temp = args.first();
stringpair ar;

	while(temp) {
		ar = args(temp);
		cout << ar.left() << " = " << ar.right() << '\n';
		args.next(temp);
	}
}


#endif
