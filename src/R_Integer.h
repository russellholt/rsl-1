/*
 $Header: /mongo/dest/crb/RCS/R_Integer.h,v 1.2 1996/03/18 13:32:35 holtrf Exp $
*/

/*****************************************************************************

 An integer resource.
 
 Russell Holt

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: List

RELATED RESOURCES:

DESCRIPTION: an Integer. Range is -2147483647 to +2147483647

PUBLIC MEMBER FUNCTIONS:

	=
		Assignment
	
	+, -, *, /
		Standard mathematical operations
		
	+=, -=, *=, /=
		As in C,
			a = a + 5;
		is effectively equivalent to, but less efficient than,
			a += 5;

	++
	--
		Increment/decrement the integer by one. Unary operators (no arguments)
	
	==, !=, <, >, <=, >=
		Standard comparisons
		
	%, %=
		modular arithmetic

	String text()
		return a String version of the Integer value.
		
	string(String s)
		Set the Integer value from a String conversion.

	
 <<End Resource Documentation>>
 ****************************************************************************/

 
/*****************************************************************************
 <<Begin Class Documentation>>


 CLASS NAME:  R_Integer

 DERIVED FROM: resource

 RELATED CLASSES:

 DESCRIPTION:

	 A resource interface to standard integer

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

#ifndef _R_INT_H_
#define _R_INT_H_

// INCLUDES


#include <String.h>
#include <SLList.h>
#include "resource.h"

/*****************************************************************************

		  	   	CLASSES

 ****************************************************************************/

class R_Integer : public resource {
private:	
	Init(void) { class_name = "Integer"; value = 0; }
	
protected:
	int value;	
	
public:
	R_Integer(void) { Init(); }
	R_Integer(String nm, int v) { Init(); name = nm; value = v; }
	R_Integer(char *nm, int v) { Init(); name = nm; value = v; }
	R_Integer(int v) { Init(); value = v; }

	resource *Create(String &nm, resource*& table);
	resource *execute(String& method, SLList<resource *> &args);

	inline String Value(void) { return dec(value); }
	int LogicalValue(void) { return value; }
	inline int intval(void) { return value; }

	void print(void) { cout << value; }
	
	inline R_Integer *Set(int x) { value = x; return this; }
	inline R_Integer *AddEq(int x) { value += x; return NULL; }
	inline R_Integer *SubEq(int x) { value -= x; return NULL; }
	inline R_Integer *MulEq(int x) { value *= x; return NULL; }
	inline R_Integer *DivEq(int x) { if (x!=0) value /= x; return NULL; }
	inline R_Integer *ModEq(int x) { value %= x; return NULL; }
	
	int FromResource(resource *f, int& theval);
	resource *Assign(resource *f);

	resource *Equal(resource *f);
	resource *And(resource *f);
	resource *Or(resource *f);
	resource *LessThan(resource *f);
	resource *GreaterThan(resource *f);
	resource *NotEqual(resource *f);
	resource *LessThanEqual(resource *f);
	resource *GreaterThanEqual(resource *f);
	resource *Not(resource *f);
	
	
	inline R_Integer& operator=(int i) { value = i; }
	friend inline int operator==(R_Integer& ri, int i);
	friend inline ostream& operator<<(ostream& out, R_Integer& i);
};

inline int operator==(R_Integer& ri, int i)
{
	return (ri.value == i);
}

inline ostream& operator<<(ostream& out, R_Integer& i)
{
	out << i.value;
}

#endif