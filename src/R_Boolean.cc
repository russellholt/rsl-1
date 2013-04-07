#include "R_Boolean.h"
#include "../log/slog.h"

//	extern slog Errorlog;

resource *R_Boolean::Create(String &nm, resource*& table)
{
	resource *nr = new R_Boolean();	//	Create();
	nr->SetName(nm);
	table = NULL;
	return nr;
}

// FromString
// returns a 1 or a 0 based on the string s:
// 1 = t, T, true, TRUE, y, Y, Yes, yes, YES
// 0 = anything else
int R_Boolean::FromString(String s)
{
	s.downcase();
	if (s == "t" || s == "true" || s == "y" || s == "yes")
		return 1;

	return 0;
}

resource *R_Boolean::Assign(resource *f)
{
	if (f)
	{
		if (f->ClassName() == "Boolean")
			Set(f->LogicalValue());
		else if (f->ClassName() == "String")
			Set(FromString(f->Value()));
	}
	else
		Set(0);
	return this;
}

resource *R_Boolean::execute(String& method, SLList<resource *> &args)
{
resource *f = NULL;
int len = args.length();

	if (len > 0)
		f = args.front();

String m = downcase(method);

	if (m == "boolean")
			return (new R_Boolean)->Assign(f);

	// if (f && f->ClassName() != "Boolean")
	//	return this;	// should return error object
	
	if (m == "=" || m == ":=" || m == "+=" || m == "assign")
		return Assign(f);
	if (m == "!")
		return new R_Boolean(value?0:1);
	if (m == "==")
		return R_Boolean::Equal(f);
	if (m == "&&" || m == "and")
	{
		if (len > 1)
			return And(args);
		return R_Boolean::And(f);
	}
	if (m == "||" || m == "or")
	{
		if (len > 1)
			return Or(args);
		return R_Boolean::Or(f);
	}
	if (m == "<")
		return R_Boolean::LessThan(f);
	if (m == ">")
		return R_Boolean::GreaterThan(f);
	if (m == "!=")
		return R_Boolean::NotEqual(f);
	if (m == "<=")
		return R_Boolean::LessThanEqual(f);
	if (m == ">=")
		return R_Boolean::GreaterThanEqual(f);
	
//	Errorlog.Log(1, "Boolean: '%s' not defined (ignored).\n", method);
	return this;	// evaluate so program can continue
}

int R_Boolean::LogicalValue(void)
{	return value; }

resource *R_Boolean::Equal(resource *f)
{
	R_Boolean *x = new R_Boolean( (value == f->LogicalValue()) );
	return x;
}
resource *R_Boolean::LessThan(resource *f)
{	R_Boolean *x = new R_Boolean( (value < f->LogicalValue()) );
	return x;
}
resource *R_Boolean::GreaterThan(resource *f)
{	R_Boolean *x = new R_Boolean( (value > f->LogicalValue()) );
	return x;
}
resource *R_Boolean::NotEqual(resource *f)
{	R_Boolean *x = new R_Boolean( (value != f->LogicalValue()) );
	return x;
}
resource *R_Boolean::LessThanEqual(resource *f)
{	R_Boolean *x = new R_Boolean( (value <= f->LogicalValue()) );
	return x;
}
resource *R_Boolean::GreaterThanEqual(resource *f)
{	R_Boolean *x = new R_Boolean( (value >= f->LogicalValue()) );
	return x;
}
resource *R_Boolean::Not(resource *f)
{	R_Boolean *x = new R_Boolean( (value == 0)?1:0 );
	return x;
}

resource *R_Boolean::And(resource *f)
{	R_Boolean *x = new R_Boolean( (value && f->LogicalValue()) );
	return x;
}

resource *R_Boolean::Or(resource *f)
{	R_Boolean *x = new R_Boolean( (value || f->LogicalValue()) );
	return x;
}

// Multiple arguments
resource *R_Boolean::And(SLList<resource *>& args)
{
int ok=0;
Pix temp=args.first();
resource *r = NULL;
	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) break;
		ok += (value && r->LogicalValue()) ? 1: 0;
	}
	R_Boolean *x = new R_Boolean( (ok == args.length()) );
	return x;
}

// Multiple arguments
resource *R_Boolean::Or(SLList<resource *>& args)
{
int ok=0;
Pix temp=args.first();
resource *r = NULL;
	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) break;
		ok += (value || r->LogicalValue()) ? 1: 0;
	}
	R_Boolean *x = new R_Boolean(ok > 0);
	return x;
}

