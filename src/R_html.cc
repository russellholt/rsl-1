#include "R_html.h"
#include "R_String.h"
#include "restable.h"
#include <SLList.h>

resource *R_html::Create(String &nm, resource*& table)
{
	
#ifdef DEBUG
	cout << "R_html::Create " << nm << "\n";
#endif
	
	R_html *r = new R_html(nm);	// name not value
	table = NULL;
	return r;
}

resource *R_html::execute(String& method, SLList<resource *> &args)
{
String m = downcase(method);

	if (m == "title")
		return title(args.front());
	else
	if (m == "list")
		return list_front(args);
	else							// specific list alternatives
	if (m == "ul" || m == "ulist")				// bulleted
		return xlist("ul", args);
	else
	if (m == "ol" || m == "olist")				// numbered
		return xlist("ol", args);
	else
	if (m == "al" || m == "alist")
		return xlist("ol TYPE=A", args);		// lettered
	else
	if (m == "il" || m == "ilist")	// roman numerals: special caps case
	{
		if (method[0] == 'i')
			return xlist("ol TYPE=i", args);	// small roman numerals
		else
			return xlist("ol TYPE=I", args);	// capital roman numerals
	}
	else
		cout << "HTML: unknown method " << method << '\n';	
	
	return NULL;
}



resource *R_html::list_front(SLList<resource *> &args)
{
resource *r = args.front();
	if (!r)
		return NULL;

	String s= r->Value();
	args.del_front();
	return xlist(s.chars(), args);
}


// xlist
// make a String resource that is an HTML list with each
// resource as a list item. If the resource is a List (restable)
// it is made into a "sublist" (and on down, recursively).
// For Strings and other resources, see makexlist() below.
//
// The argument "tag" specifies the list open/close tag to use.
// "ul" means unordered list --> <ul> ... </ul>
// Or, for a netscape hack, "ul TYPE=square" would give square bullets,
// or "ol TYPE=A" would number the items a, b, c, ...
//
// RFH 10/10/95
resource *R_html::xlist(const char *tag, SLList<resource *> &args)
{
restable *rt = new restable;
resource *r = NULL;
Pix temp = args.first();
R_String *rs = NULL;
String s;

	s = String("\n<") + tag + String(">");
	rs = new R_String("", s.chars());
	rt->Add(rs);

	for(;temp; args.next(temp))
	{
		r = args(temp); if (!r) continue;
		if (r->ClassName() == "List")
			s = makexlist(((restable *) r)->GetList(),
				(args.length()>1), tag);
		else
			s = makeItemString(r);

		if (s.length() > 0)
		{
			rs = new R_String("", s.chars());
			rt->Add(rs);
		}
	}

	s = String("\n</") + tag + String(">\n");
	rs = new R_String("", s.chars());
	rt->Add(rs);
	
	return rt;
}

// makexlist
// Makes a String that contains the text of a restable-to-HTML-list,
// with one resource per list item. If a resource is a restable, this
// method is called recursively to build a big string.
// RFH 10/10/95
String R_html::makexlist(SLList<resource *> &args, int tagit, const char *tag)
{
String l;
resource *r;
Pix temp = args.first();

	if (tagit)
		l += String("\n<") + tag + String(">");

	for(;temp; args.next(temp))
	{
		r = args(temp);
		if (r->ClassName() == "List")
			l += makexlist(((restable *) r)->GetList(), (args.length()>1), tag);
		else
			l += makeItemString(r);
	}

	if (tagit)
		l += String("\n</") + tag + String(">\n");
	
	return l;
}

// makeItemString
// create a list item string for one non-list resource
// For String, use the value as the item text.
// for others, use the resource name as item text if it is not blank,
// or default to the class name of the resource.
// This way, all resources are listed one way or another.
// RFH 10/10/95
String R_html::makeItemString(resource *r)
{
String s;

	if (r->ClassName() == "String")
		s = String("\n<li>") + r->Value();
	else
	{	// any other type, use the resource name if it has one,
		// or use the class name.
		s = String("\n<li>") +
			( (r->Name()).length() > 0 ?
			r->Name() : r->ClassName() );
	}
	return s;
}

resource *R_html::title(resource *r)
{
	return NULL;
}


void R_html::print(void)
{

}