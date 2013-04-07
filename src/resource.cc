/* $Id: resource.cc,v 1.7 1996/03/18 13:26:23 holtrf Exp $
 *
 *  A `component', even though I am beginning to despise that word.
***
$Log: resource.cc,v $
Revision 1.7  1996/03/18 13:26:23  holtrf
release 1

Revision 1.6  1996/02/06 19:16:30  holtrf
GetSet(int& ..  ) correctly works with R_Boolean*


***
 * Russell Holt, July 1995.
 */
#include "resource.h"
#include <stream.h>
#include "restable.h"
#include "R_Boolean.h"
#include "R_Integer.h"
#include "R_String.h"

const char * const rcsid = "$Id: resource.cc,v 1.7 1996/03/18 13:26:23 holtrf Exp $";

extern R_Boolean grTrue, grFalse;	// defined in rsl_control.cc

extern int local_creates;

resource::resource(void)
{
	existence = exist_temp;
	enabled = 1;
	hierarchy = ":";
}


resource *resource::execute(String& method, SLList<resource *> &args)
{
	cout << "RSL strangeness: resource::execute() called for `" << name << "'.\n";
}
/*
// _execute
// pre-process the argument lists by replacing any resources of
// existence 'exist_lookup' with the resources they refer to
// in the resource table.
//
// invoked from rescall::execute
resource *resource::_execute(String& method, SLList<resource *> &args,
	restable *rtab)
{
Pix temp=args.first(), ptemp=temp;
resource *r, *realr, *tr;
	for(;temp; args.next(temp)
	{
		resource *r = args(temp);
		if (r && r->existence == exist_lookup)
		{
			realr = rtab->GetResource(r->name);
			if (realr)
			{
				// delete the item after the previous temp pointer
				args.del_after(ptemp);
				delete r;	// not sure if del_after deletes item
				// insert the new resource * where the old was removed
				args.ins_after(ptemp, realr);
			}
		}
	}
	return execute(method, args);	// virtual overload
}
*/

//  Print the name and/or other interesting data.
void resource::print(void)
{
	cout << class_name << "  " << name << '\n';
}

void resource::print(SLList<resource *> things, char *indent)
{
Pix temp=things.first();
resource *r;
	for(; temp; things.next(temp))
	{
		r = things(temp);
		cout << indent;
		if (r)
			r->print();
			//	cout << "Resource: " << r->Name() << "<p>\n";
	}
}

// Create
//	Make a new resource object and install it in the variable table.
//	- The first resource in args should be a restable (rsl type "List")
//	  which is the restable to install the new restable resource into!
//	- The rest of the resources in args should be Strings which
//	  specify the names of each new resource to create.
//	- calls the copy constructor of each resource to make a new
//    object of the right resource subclass.
//	  
//	RFH 9/20/95
// template <class T> resource *restable::Create(SLList<resource *>& args)
/****** obsolete ******
resource *resource::Create(SLList<resource *>& args)
{
Pix temp=args.first();
resource *x = args.front();
restable *vartable = NULL;

	cerr << "\nresource::Create(args) should no longer be called.\n";


	if (!x || x->ClassName() != "List")
	{
		cout << "<b>Error</b> restable::Create - no variable table "
			 << "to install new resource into.<p>\n";
		return NULL;
	}
	vartable = (restable *) x;
	
	for(args.next(temp); temp; args.next(temp))
	{
		x = args(temp);
		if (x)
		{
			// call virtual Create to make correct subclass object
			if (x->ClassName() == "String")
			{
				resource *nr = Create(x->Value());	//	new T(x->Value());
				if (nr)
					vartable->install_resource(nr);
			}
			else
				cout << "<b>resource::Create</b> with non-String argument "
					 << "for the name: " << x->ClassName() << ' '
					 << x->Name() << "<br>\n";
		}
	}
	return NULL;
}
*/
/*
// Create
// to be called from within ResCall, not from individual resources.
void resource::Create(resource *the_res, SLList<resource *>& args, resource *v)
{
Pix temp = args.first();
resource *new_name = NULL, *nr=NULL;

	if (!(v && v->ClassName() == "List"))
		return;

restable *vars = (restable *) v;

	if (!the_res || !vars)
		return;

	for(; temp; args.next(temp))
	{
		new_name = args(temp);
		if (new_name && new_name->ClassName()=="String")
		{
			nr = the_res->Create(new_name->Value());
			if (nr)
			{
				nr->MakeExist(exist_local);	// these are local variables
				vars->install_resource(nr);
				local_creates++;
			}
		}
		else
			cout << "resource::Create() with non-String argument "
				 << "for the name: " << new_name->ClassName() << ' '
				 << new_name->Name() << '\n';
	}
}
*/

// Create
// Overload to return a pointer to a resource subclass
// --- May go away, replaced by copying the "template" object of the
// specified type
resource *resource::Create(String &nm, resource*& table)
{
resource *r = new resource;
	table = NULL;
	r->SetName(nm);
	return r;
}

int resource::IsComparison(String &m)
{
	return (m == "==" || m == "<" || m == "<=" || m == ">"
			|| m == ">=" || m == "!" || m == "!="
			|| m == "&&" || m == "||"
			|| m == "and" || m == "not" || m == "or");
}


resource *resource::rTrue(void)
{
	return &grTrue;	
}

resource *resource::rFalse(void)
{
	return &grFalse;	
}


// ResError
// print an error string indicating the class name
// and the resource name
// Override to do something else
// -- maybe this will go away with the introduction of "hierarchy",
//    that is, maybe if/when all resources use it.
// -- or maybe this will make an slog call
void resource::ResError(const char *s)
{
	cerr << class_name << ' ' << name << ": " << s << '\n';
}


resource *resource::GetSet(String& themember, resource *f)
{
	if (f)	// set
		themember = f->Value();
	return new R_String(themember);	// get
}

resource *resource::GetSet(int& themember, resource *f)
{
	if (f && f->ClassName() == "Integer")	// set
		themember = ((R_Integer *)f)->intval();
	else
		if (f && f->ClassName() == "Boolean")
			themember = f->LogicalValue();

	return new R_Integer(themember);	// get
}
