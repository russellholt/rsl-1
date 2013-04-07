// $Id: R_String.cc,v 1.3 1996/03/20 17:08:10 holtrf Exp holtrf $
//
// PURPOSE: String resource implementation
//
// PROGRAMMING NOTES:
//
// HISTORY:
//  9/14/95 - RFH created
//  1/4/96 - RFH - added the size parameter to Split
//
// Copyright 1995 by Destiny Software Corporation.

#include "R_String.h"
#include "R_Integer.h"
#include "restable.h"
#include <SLList.h>
#include <stream.h>
extern "C" {
#include <ctype.h>
}
#include "R_Boolean.h"

/* these functions should move to utility code */
unsigned int ha(const char *s0);
unsigned int collatz(unsigned int i);
String rot13(String v);


Regex RXcash("\\$?\\([0-9]?[0-9]?[0-9]\\)\\(,?[0-9][0-9][0-9]\\)*\\.[0-9][0-9]", 1, 200);

String strip_control(String s, int nl);

//
// hashed R_String resource methods
// These are not in R_String.h because
// they should only be used inside this file.
//

#define _hOpAnd 15	// &&
#define _hOpGT 33	// >
#define _hOpLT 34	// <
#define _hVAR 39	// var
#define _hOpGE 47	// >=
#define _hOpOr 93	// ||
#define _hOpNE 121	// !=
#define _hOpLE 124	// <=
#define _hOpEQ 133	// =
#define _hLEN 161	// len
#define _hBEFORE 166	// before
#define _hOpAdd 169	// +
#define _hOpEQ2 217	// ==
#define _hOpPE 227	// +=
#define _hROT13 232	// rot13
#define _hREVERSE 245	// reverse
#define _hFREQ 340	// freq
#define _hLENGTH 589	// length
#define _hPREPEND 1325	// prepend
#define _hINT 1373      // int
#define _hGSUB 1697	// gsub
#define _hCONTAINS 2754	// contains
#define _hGETREVERSE 2968	// getreverse
#define _hAFTER 4155	// after
#define _hCASHVALID 4401	// cashvalid
#define _hGSUBR 4827    // gsubr
#define _hASSIGN 8146	// assign
#define _hAPPEND 8237	// append
#define _hSPLIT 8417	// split
#define _hGETROT13 11272	// getrot13
#define _hCONTAINSOR 16278	// containsOr
#define _hCONTAINSAND 16453	// containsAnd
#define _hUPCASE 16676	// upcase
#define _hDOWNCASE 21796	// downcase
#define _hSTRIP_CONTROL 32355   // strip_control
#define _hGETUPCASE 74276	// getupcase
#define _hCAPITALIZE 111370	// capitalize
#define _hGETDOWNCASE 295204	// getdowncase
#define _hGETCAPITALIZE 372834	// getcapitalize


resource *R_String::Create(String &nm, resource*& table)
{
	
#ifdef DEBUG
	cout << "String::Create " << nm << "\n";
#endif
	
	table = NULL;
	R_String *r = new R_String(nm.chars(), "");	// name not value
	return r;
}

// Assign
resource *R_String::Assign(resource *r)
{
	if (!r)
		return NULL;

	value = r->Value();
	return this;
}

// execute
// The bridge between C++ and rsl
resource *R_String::execute(String& method, SLList<resource *> &args)
{
	// methods are roughly in the order of expected frequency
String m = downcase(method);

	resource *f = NULL;
	if (args.first())
		f = args.front();

	unsigned int hashed = ha(m);
	switch(hashed)
	{
		case _hASSIGN:	// "assign"
		case _hOpEQ:	// "="
			return Assign(f);

		case _hAPPEND:	// "append"
		case _hOpPE:	// "+="
			Append(args);
			break;

		case _hPREPEND:	// "prepend"
			Prepend(args);
			break;

		case _hOpAdd:	// "+"
			return Plus(args);

		case _hGSUB:	// "gsub" - string pattern gsub
			_gsub_(args, 0);
			break;

		case _hGSUBR:   // "gsubr" - regex pattern gsub
			_gsub_(args, 1);
			break;

		case _hSTRIP_CONTROL:   // strip_control
			value = strip_control(value, (f?f->LogicalValue() : 0));
			break;

		case _hCONTAINS:	// "contains"
		case _hCONTAINSOR:	// "containsOr"
			return contains(args, 1);

		case _hCONTAINSAND:	// "containsAnd"
			return contains(args, 0);

		case _hSPLIT:	// "split"
			if (args.length() > 0)
			{
				Pix yack = args.first();
				resource *a=NULL, *b=NULL;
				a = args(yack);
				if (args.length() > 1)
				{
					args.next(yack);
					b = args(yack);
				}
				return Split(a, b);
			}
			else
				cout << "Split: requires an argument.\n";
			break;

		case _hINT:		// int
			return (new R_Integer(atoi(value.chars()) ));
			break;

		case _hLEN:		// "len"
		case _hLENGTH:	// "length"
			return new R_Integer(value.length());
			break;

		case _hFREQ:	// "freq"
			return (f ? Freq(f) : NULL);
		case _hBEFORE:	// "before"
			return Before(f);
		case _hAFTER:	// "after"
			return After(f);
		case _hUPCASE:	// "upcase"
			value.upcase();
			break;
		case _hGETUPCASE:	// "getupcase"
			return new R_String(upcase(value));	// StringOps(UpCase, Return);
		case _hDOWNCASE:	// "downcase"
			value.downcase();
			break;
		case _hGETDOWNCASE:	// "getdowncase"
			return new R_String(downcase(value));
		case _hREVERSE:	// "reverse"
			value.reverse();
			break;
		case _hGETREVERSE:	// "getreverse"
			return new R_String(reverse(value));
		case _hCAPITALIZE:	// "capitalize"
			value.capitalize();
			break;
		case _hGETCAPITALIZE:	// "getcapitalize"
			return new R_String(capitalize(value));

		case _hROT13:	// "rot13"
		case _hGETROT13:	// "getrot13"
			{
				String sss = rot13(value);
				if (m[0] == 'g')
					return new R_String(sss);
				else
					value = sss;
			}
			break;

		case _hVAR:	// "var"
			{
				String x = name + " = \"" + value + "\"";
				resource *nr = new R_String("", x.chars());
				return nr;
			}
			break;

		/* Comparisons */

		case _hOpEQ2:	// "=="
			return Equal(f);
		case _hOpLT:	// "<"
			return LessThan(f);
		case _hOpGT:	// ">"
			return GreaterThan(f);
		case _hOpLE:	// "<="
			return LessThanEqual(f);
		case _hOpGE:	// ">="
			return GreaterThanEqual(f);
		case _hOpNE:	// "!="
			return NotEqual(f);
		case _hOpAnd:	// "&&"
			return And(f);
		case _hOpOr:	// "||"
			return Or(f);

		case _hCASHVALID:	// "cashvalid"
			{
				return new R_Boolean(value.matches(RXcash));
			}

		default: ;
	}

	return this;	// could return this;
}

void R_String::_gsub_(SLList<resource *>&args, int reg)
{
	if (args.length() <2)
	{
		cout << "String::gsub requires 2 arguments.\n";
		return;
	}
Pix temp = args.first();
resource *s, *r;
	s = args(temp); args.next(temp);
	r = args(temp);
	if (s->ClassName() == "String" && r->ClassName() == "String")
		if (reg)
			value.gsub(Regex(s->Value()), r->Value());
		else
			value.gsub(s->Value(), r->Value());
	else
	{
		cout << "String::gsub requires arguments of type \"String\".\n";
		return;
	}
}


// contains
// if oneMatch == 1 ("OR") -- from RSL "Contains" or "ContainsOr"
//    return true if this string contains one or more of the
//      argument strings.
// 
// if oneMatch == 0 ("AND") -- from RSL "ContainsAnd"
//    return true if this string contains all of the argument
//      strings.
// RFH 1/28/96
resource *R_String::contains(SLList<resource *>& args, int oneMatch)
{
R_Boolean *rb = new R_Boolean;

Pix temp = args.first();
resource *r = NULL;
int len = args.length(), count = 0;

	for (; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;

		if (value.contains(r->Value()) || r->Value() == "")
		{
			count++;	
			if (oneMatch)
				break;
			// cout << "contains for " << r->Value() << "\n";
		}
		
	}

	if (oneMatch)	
		rb->Set(count);
	else
		rb->Set(count >= len ? 1 : 0);

	return rb;
}


// Plus
// return a new String resource that is the sum of all the
// args and this->value
resource *R_String::Plus(SLList<resource *>& args)
{
String nval = value;
Pix temp=args.first();

	for(; temp; args.next(temp))
		nval += (args(temp))->Value();	// even for non-Strings
	R_String *nr = new R_String("", nval.chars());
	return nr;
}

// Before --> String::before()
// return a new String resource which is a substring of this->value
// before the given String.
resource *R_String::Before(resource *r)
{
	if (!r) {
		cout << "String.After(): Expects an argument"; return NULL; }
	if (r->ClassName() != "String") {
		cout << "String.Before(): Expected String\n"; return NULL; }
	
	R_String *nr = new R_String();
	String x = value.before(r->Value());
	nr->Assign(x);
	return nr;
}

// After --> String::after()
// return a new String resource which is a substring of this->value
// after the given String.
resource *R_String::After(resource *r)
{
	if (!r) {
		cout << "String.After(): Expects an argument"; return NULL; }
	if (r->ClassName() != "String") {
		cout << "String.After(): Expected String\n"; return NULL; }
	
	R_String *nr = new R_String();
	String x = value.after(r->Value());
	nr->Assign(x);
	return nr;
}

// Freq (uency)
// return an Integer count of the frequence of the given R_String.
resource *R_String::Freq(resource *r)
{
int i = 0;

	// will return zero if r was NULL or not R_String
	if (r && r->ClassName() == "String")
		i = value.freq(r->Value());		// String::freq()
	
	R_Integer *ri = new R_Integer("",i);
	return ri;
}

//
// Access via String - - -
//

void R_String::Append(SLList<stringpair> &args)
{
Pix temp;
	for(temp=args.first(); temp; args.next(temp))
		Append(ArgVal(args(temp)));
}

void R_String::Prepend(SLList<stringpair> &args)
{
Pix temp;
	for(temp=args.first(); temp; args.next(temp))
		Prepend(ArgVal(args(temp)));
}

//
// Access via resources - - -
//

void R_String::Append(SLList<resource *> &args)
{
Pix temp;
resource *r;
String cname;

	if (args.length() > 0)
		for(temp=args.first(); temp; args.next(temp))
		{
			r = args(temp);
			if (!r) continue;

			cname = r->ClassName();

				// For a list, recursively append the contents
			if (cname == "List")
			{
				SLList<resource *> xl = ((restable *) r)->GetList();
				Append(xl);
			}
			else	// String, Integer, etc. -- append the `value'
				Append(r->Value());	// `String' equivalent.
		}
}

void R_String::Prepend(SLList<resource *> &args)
{
Pix temp;
resource *r;

	if (args.length() > 0)
		for(temp=args.first(); temp; args.next(temp))
		{
			r = args(temp);
			if (r && r->ClassName() == "String")
				Prepend(r->Value());
			else
				{ /* error: non-string can't be appended */ }
		}
}

// Split
// makes a list of strings by extracting substrings from (this)
// using str as the separator
resource *R_String::Split(resource *str, resource *nelems)
{
restable *nrt = NULL;
R_String *nrs = NULL;
	if (!str) return NULL;
	
	if (str->ClassName() == "String")
	 // or later- if (str && str->HierarchyContains(":primitive:"))
	 {
		nrt = new restable;
		if (nrt)
		{
			String a = str->Value();
			int i, x = value.freq(a);
			String thelist[x+1];
			
			// if R_String were a subclass of String, then thelist
			// could be an array of R_Strings.
			split(value, thelist, x+1, a);
			
			// Make each String in thelist into an R_String,
			// and add it to the new restable
			// -- function of restable???
			for(i=0; i<=x; i++)
			{
				nrs = new R_String(thelist[i]);
				nrt->install_resource(nrs);
			}
			
			// record the size of the list in the "nelems" parameter if it exists
			if (nelems && nelems->ClassName() == "Integer")
			{
				((R_Integer *) nelems)->Set(x+1);
			}
		}
	}
	else
	// split into substrings of the given size
	if (str->ClassName() == "Integer")
	{
		nrt = new restable;
		int size = ((R_Integer *) str)->intval();
		if (value.length() < size)	// only one
		{
			nrs = new R_String(*this);	// copy
			nrt->install_resource(nrs);
		}
		else
		{
			String s =value, sleft;
			while (s.length() > size)
			{
				sleft = s.before(size);
				s = s.after(size-1);
				nrs = new R_String(sleft);
				nrt->install_resource(nrs);
			}
			if (s.length())
			{
				nrs = new R_String(s);
				nrt->install_resource(nrs);
			}
		}
		
	}
	
	return nrt;
}

void R_String::print(void)
{
	if (!value.contains("\\"))
	{
		cout << value;
		return;
	}
	else
	{
		const char *p;
	
		for(p = value.chars(); *p; p++)
			if (*p == (char) 0x5c)		// 0x5c
			{
				if (*(++p) && *p == 'n')
						cout << '\n';
			}
			else
				cout << *p;
	}
}



resource *R_String::Equal(resource *f)
{
	return new R_Boolean(f && (value == f->Value()));
}
resource *R_String::And(resource *f)
{
	return new R_Boolean(f && (LogicalValue() && f->LogicalValue()));
}
resource *R_String::Or(resource *f)
{
	return new R_Boolean(f && (LogicalValue() || f->LogicalValue()));
}
resource *R_String::LessThan(resource *f)
{
	return new R_Boolean(f && (value < f->Value()));
}
resource *R_String::GreaterThan(resource *f)
{
	return new R_Boolean(f && (value > f->Value()));
}
resource *R_String::NotEqual(resource *f)
{
	return new R_Boolean(f && (value != f->Value()));
}
resource *R_String::LessThanEqual(resource *f)
{
	return new R_Boolean(f && (value <= f->Value()));
}
resource *R_String::GreaterThanEqual(resource *f)
{
	return new R_Boolean(f && (value >= f->Value()));
}
resource *R_String::Not(resource *f)	
{
	return new R_Boolean(LogicalValue());	// nope
}



/******************************
 * Non-R_String functions
 * --- should move to utility files...
 *****************************************/

String rot13(String v)
{
	String newv = v;
	int i=0, x=newv.length();
	char c;
	for(i=0; i<x; i++) {
		c=newv[i];
		if (isalpha(c))
			c = isupper(c) ? ((c-'A'+13)%26)+'A' : ((c-'a'+13)%26)+'a';
		newv[i] = c;		
	}
	return newv;
}

	
// ha
// the hash
unsigned int  ha(const char *s0)
{
String s = s0;
int len = s.length(), i;
unsigned int what=s[0], pwhat=0;

	s.downcase();

	for(i=0; i<len; i++,pwhat=what)
		what ^= collatz(pwhat ^ s[i]);
	return (unsigned int) what;
}

// collatz
// compute "hailstone" sequence
unsigned int collatz(unsigned int i)
{
	if (i&1)	// if odd
		return 3*i + 1;
	return i>>1;	// divide by 2
}


// strip_controls
// strips control chars from s.
// `nl' is a boolean: 1 = keep newline (0x0a), 0 = strip newline.
String strip_control(String s, int nl)
{
int i, j=0, len = s.length();
char c, nw[len];

	for(i=0; i<len; i++)
	{
		c = s[i];
		if ((c > 0x1f) || (nl && c == 0x0a))
			nw[j++] = c;
	}
	nw[j] = '\0';
	return String(nw);
}
