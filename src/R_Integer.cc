#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_String.h"
#include "restable.h"

extern "C" {
#include <stdlib.h>
}

// hash function
extern unsigned int ha(const char *);

//
// Integer rsl methods
//

#define _hOpGT 33	// >
#define _hOpLT 34	// <
#define _hOpGE 47	// >=
#define _hOpTE 62	// *=
#define _hOpMult 63	// *
#define _hOpMod 85	// %
#define _hOpModE 97	// %=
#define _hOpNE 121	// !=
#define _hOpLE 124	// <=
#define _hOpEQ 133	// =
#define _hOpDiv 161	// /
#define _hOpSubt 165	// -
#define _hOpAdd 169	// +
#define _hPRINT 213	// print
#define _hOpEQ2 217	// ==
#define _hOpMM 225	// --
#define _hOpPE 227	// +=
#define _hTEXT 231	// text
#define _hOpPP 232	// ++
#define _hOpME 233	// -=
#define _hOpDE 239	// /=
#define _hMAKE 507	// make
#define _hSTRING 4334	// string


resource *R_Integer::Create(String &nm, resource*& table)
{
	R_Integer *r = new R_Integer(nm, 0);
	table = NULL;
	return r;
}

// FromResource
// - extract an int from a variety of resources.
// - returns a 1 if successful, 0 if not.
// - extracted integer is returned through 'theval'
int R_Integer::FromResource(resource *f, int& theval)
{
int ok = 0;
	if (!f)
		return 0;

	if (f->ClassName() == "Integer")
	{
		theval = ((R_Integer *) f)->intval();
		ok=1;
	}
	else
	if (f->ClassName() == "String")
	{
		ok=1;
		theval = atoi((f->Value()).chars());	// try to convert from String
	}	
	else
	if (f->ClassName() == "List")
	{
		ok=1;
		theval = (((restable *)f)->GetList()).length();	// use length of list
	}

	return ok;
}

resource *R_Integer::Assign(resource *f)
{
	if (!f)
		return NULL;
	
	int theval;
	int ok = FromResource(f, theval);

	if (ok)
	{
		value = theval;
		return this;
	}
	return NULL;
}

//
// execute
//
// the RSL interface
//
// -- branching on hash value added April 2, 1996
resource *R_Integer::execute(String& method, SLList<resource *> &args)
{
String m = downcase(method);
unsigned int hashv = ha(m);	// get hash value for downcase'd method
int usearg=0;
resource *f = NULL;

	if (args.first())
		f = args.front();

	switch(hashv)	// branch on hash value
	{
		case _hTEXT:	// "text"
			{
				R_String *nr = new R_String;
				nr->Assign(dec(value));
				return nr;
			}

		case _hPRINT:	// "print"
			cout << value;
			return this;

		case _hOpPP:	// "++"
			value++;
			return this;

		case _hOpMM:	// "--"
			value--;
			return this;

		case _hMAKE:	// "make"
		case _hSTRING:	// "string"
			if (f && f->ClassName() == "String")
			{
				value = atoi((f->Value()).chars());
				return this;
			}
			break;
		default: usearg = 1;
	}

	/***************************
	 * Single argument methods.
	 ***************************/
	int argint;
	
	if (usearg && FromResource(f, argint))
	{
		switch(hashv)
		{
			case _hOpEQ:	// =
				value = argint;
				return this;

			case _hOpAdd:	// "+"
				return new R_Integer(value + argint);

			case _hOpSubt:	// "-"
				return new R_Integer(value - argint);

			case _hOpMult:	// "*"
				return new R_Integer(value * argint);

			case _hOpDiv:	// "/"
				return new R_Integer(value / argint);

			case _hOpPE:	// "+="
				value += argint;
				return this;

			case _hOpME:	// "-="
				value -= argint;
				return this;

			case _hOpTE:	// "*="
				return MulEq(argint);

			case _hOpDE:	// "/="
				return DivEq(argint);

			case _hOpMod:	// "%"
				return new R_Integer(value % argint);

			case _hOpModE:	// "%="
				return ModEq(argint);

			case _hOpEQ2:	// "=="
				return new R_Boolean(value == argint);
			case _hOpLT:	// "<"
				return new R_Boolean(value < argint);
			case _hOpGT:	// ">"
				return new R_Boolean(value > argint);
			case _hOpLE:	// "<="
				return new R_Boolean(value <= argint);
			case _hOpGE:	// ">="
				return new R_Boolean(value >= argint);
			case _hOpNE:	// "!="
				return new R_Boolean(value != argint);

			default: ;

		}	// case

	}	// usearg
	// else ... invalid argument or unable to translate argument..
}


resource *R_Integer::Equal(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value == i));
}
resource *R_Integer::And(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value && i));
}
resource *R_Integer::Or(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value || i));
}
resource *R_Integer::LessThan(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value < i));
}
resource *R_Integer::GreaterThan(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value > i));
}
resource *R_Integer::NotEqual(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value != i));
}
resource *R_Integer::LessThanEqual(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value <= i));
}
resource *R_Integer::GreaterThanEqual(resource *f)
{
	int i, ok = FromResource(f, i);
	return new R_Boolean(ok && (value >= i));
}
resource *R_Integer::Not(resource *f)
{
	return new R_Boolean(0);	// unsupported
}



