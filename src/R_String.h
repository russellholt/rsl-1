/*
 $Header: /mongo/dest/crb/RCS/R_String.h,v 1.3 1996/04/25 21:44:18 holtrf Exp holtrf $
*/

/*****************************************************************************

 R_String.h
 
 A resource interface to the GNU String class, with additional operations.

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: String

RELATED RESOURCES:

DESCRIPTION: a string

PUBLIC MEMBER FUNCTIONS:
	
	assign(R)
	=
		Set the value of this string to the value of R.

	append(...)
	+=
		Append the value of the given resource(s) to this String.

	prepend(...)
		Prepend this String with the value of a resource.

	+
		return a new String that is the concatenation of
		this resource and the value of R

	gsub(String pattern, String replacement)
		substitute every occurrence of `pattern' with 'replacement'
		(`gsub' stands for `global substitute').

	contains(...)
	containsOr(...)
		Determine if this String contains one or more of the
		arguments as substring patterns. Return true if so, false if not.

	containsAnd(...)
		Determine if this String contains ALL of the arguments
		as substring patterns. Return true if so, false if not.

	List split(String delimiter)
		extract substrings, separated by the given
		delimiter (String representation), and return them in a List.
		Example:
			String s;
			List l;
			s = "one/two/three";
			l = s.split("/");
			// l now has three elements: "one", "two", "three".

	Integer freq(String pattern)
		return an Integer giving the number of non-overlapping
		occurrences of `pattern'.

	before (String pattern)
		Return the String preceding the first occurence of `pattern'

	after
		Return the String following the first occurence of `pattern'

	upcase
		Make all letters of this String uppercase
		
	getupcase
		Return a copy of this String with all uppercase letters.
		
	downcase
		Make all letters of this String lowercase
		
	getdowncase
		Return a copy of this String with all lowercase letters.
		
	reverse
		Reverse this String.
		
	getreverse
		Return the reverse of this String
		
	capitalize
		Upcase The First Letter Of Each Word
		
	getcapitalize
		Return a copy of this String with all first letters capitalized.
		
	rot13
		rot13 this String, that is, for each letter i where 'a' = 1,
		'b' = 2, etc, inew = (i+13) mod 26
		
	getrot13
		return a copy of the rot13 version of this String.
		
	var
		return a String of the form:
		<variable name> = "<string value>"

	Comparison: ==, <, >, <=, >=, !=, &&, ||
		Normal string comparison, case sensitive.
			Eg, "A" < "B", "abc" > "a", "1000" < "2"
		Boolean operators (&&, ||) use the length of the
		String. Eg, "abc" && "" is false while "abc" || "" is true.
		This is useful to determine if a value has been set.

 <<End Resource Documentation>>
 ****************************************************************************/

 
/*****************************************************************************
 <<Begin Class Documentation>>


 CLASS NAME:  R_String

 DERIVED FROM: resource

 RELATED CLASSES: String

 DESCRIPTION:

	A resource interface to the GNU String class, with additional operations.
	Regex is not supported via RSL at this time
 

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

 SEE ALSO: <String.h>, resource.h
 

 <<End Class Documentation>>
 ****************************************************************************/




//	C++ Interface:
//
//	RFH	9/14/95

#ifndef _R_STRING_H_
#define _R_STRING_H_


// INCLUDES

#include <String.h>
#include "resource.h"


/*****************************************************************************

		  	   	CLASSES

 ****************************************************************************/
 
class R_String: public resource {
	String value;
	void Init(void) { class_name = "String"; }

public:	
	R_String(void) { Init(); }
	~R_String(void) { }
	R_String(String &nm, String& val)
		{ Init(); resource::name = nm; value = val; }
	R_String(const char *nm, const char *val)
		{ Init(); resource::name = nm; value = val; }
	R_String(String &val)
		{ Init(); resource::name = ""; value = val; }
	R_String(const char *val)
		{ Init(); resource::name = ""; value = val; }
		
	resource *Create(String &nm, resource*& table);

	resource *execute(String& method, SLList<resource *> &args);

	inline String Value(void) { return value; }
	inline const char *chars(void) { return value.chars(); }
	inline void Assign(String &v) { value = v; }
	inline void Assign(char *v) { value = v; }
	resource *Assign(resource *r);

	void Append(SLList<stringpair> &args);
	void Append(SLList<resource *> &args);
	inline void Append(String &s) { value += s; }

	void Prepend(SLList<stringpair> &args);
	void Prepend(SLList<resource *> &args);
	inline void Prepend(String &s) { value.prepend(s); }

	resource *Split(resource *str, resource *nelems=NULL);
	resource *Before(resource *r);
	resource *After(resource *r);
	resource *Freq(resource *r);
	resource *contains(SLList<resource *>& args, int oneMatch);
	inline int length(void) { return value.length(); }
	
	void _gsub_(SLList<resource *> &args, int reg=0);
	resource *Plus(SLList<resource *> &args);
	
	void print(void);

	int LogicalValue(void) { return value.length(); }
	resource *Equal(resource *f);
	resource *And(resource *f);
	resource *Or(resource *f);
	resource *LessThan(resource *f);
	resource *GreaterThan(resource *f);
	resource *NotEqual(resource *f);
	resource *LessThanEqual(resource *f);
	resource *GreaterThanEqual(resource *f);
	resource *Not(resource *f);	
	
	
	inline void operator=(String& s) { value = s; }
	inline void operator=(char *s) { value = s; }
	inline void operator+=(String& s) { value += s; }
	inline void operator+=(const char *s) { value += s; }
	inline void operator+=(char s) { value += s; }

	friend inline int operator==(R_String &v1, R_String &v2);
	friend inline int operator==(R_String &v1, String &v2);
	friend inline ostream& operator<<(ostream& out, R_String& s);
};

inline int operator==(R_String &v1, R_String &v2)
{
	return (v1.value == v2.value);
}

inline int operator==(R_String &v1, String &s)
{
	return (v1.value == s);
}

inline ostream& operator<<(ostream& out, R_String& s)
{
	out << s.value;
}


#endif