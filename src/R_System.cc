// R_System.cc
//
// PURPOSE:
//
// PROGRAMMING NOTES:
//
// HISTORY: 
//		1/8/96 Russell Holt
//
// $Id: R_System.cc,v 1.1 1996/03/18 13:20:42 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.
#include <String.h>
#include "R_System.h"
#include "R_String.h"
#include "R_Integer.h"
#include "destiny.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


#ifdef SYSTEM_EMAIL
extern int SendMail(String mailserver, String to, String from, String subj,
	String data);
#endif

void R_System::Init(void)
{
	class_name = "System";
	hierarchy += "System:";
	
	// and maybe initialize format to a default.
}



// Create
// Create an object of this type. Optionally return a restable
// ("List" type in RSL) to be used as the table of local variables
// in an rsl Create method (confused? then ignore the table,
// and just do what is done here)
resource *R_System::Create(String &nm, resource*& table)
{
	table = NULL;
	return new R_System(nm);
}

// execute
// This is the rsl interface to this class. Eg, in rsl, the statement
//			obj.meth("a", b, c);
// means object named "obj" with method "meth", and the args are { "a", b, c }.
// Quoted strings, like "a", are made into R_String resources by the
// parser. During execution, rsl will look up b and c and substitute
// resource * to the actual objects those names refer to, if they exist.
// So in this case, the args will have 3 resource, the first one is
// an R_String *, followed by two more of unknown type (from this information
// alone - the type can be determined by calling ClassName())
//
// Basically this function "switches" on the method name, "does its thing",
// and returns a resource *, if applicable (often, 'this').
resource *R_System::execute(String& method, SLList<resource *> &args)
{	
	// be case-insensitive
	String m = downcase(method);	// downcase() from <String.h>
	
	resource *f = NULL;
	int len = args.length();
	if (len > 0)
		f = args.front();

	// RSL data member access
	if (m == "date" || m == "today")
	{
		time_t now = time(0);
		struct tm *tmtime = localtime(&now);
		char buf[80];
		int r = strftime(buf, 80, "%m/%d/%y", tmtime);
		return new R_String(buf);
		return NULL;
	}
	else
	if (m == "time" || m == "now")
	{
		time_t now = time(0);
	    struct tm *ltime = localtime(&now);
	    return new R_String(asctime(ltime));
	}
		
#ifdef SYSTEM_EMAIL
	else
	if (m == "email")
		email(args);
#endif
	else
	if (m == "abort")
		abort();

	else
	if (m == "usage")
		cout << name << ": <usage message here in the future, maybe>\n";
	else
	{
		cout << name	// resource::name
			 << ": unknown method \"" << method << "\"\n";
	}

	return NULL;	// no return value to RSL
}


// Value
// return a String version of the "value" of the data this class represents,
// if it applies. The default version (resource::Value()) prints the
// class name and the instance name.
String R_System::Value(void)
{
	return String("a test resource");
}

// print
// similar to Value, only printed to stdout
void R_System::print(void)
{
	cout << class_name << ": System resources.";
}

// LogicalValue
// the "trueness" or "falseness" of your resource. Only applicable to
// "primitive" resource types, and used for comparison.
// Eg, R_Integer::LogicalValue returns its integer value, and R_String
// returns a 0 if its String == "" and a 1 otherwise.
int R_System::LogicalValue(void)
{
	// be true by default.
	return 1;
}

#ifdef SYSTEM_EMAIL

// email
// expects: mailserver, to, from, subj, data
void R_System::email(SLList<resource *> &args)
{
resource *mailserver, *to, *from, *subj, *data;

	mailserver = to = from = subj = data = NULL;

	if (args.length() != 5)
	{
		cout << "System: email requires a mailserver, from, to, subject,
			and data.\n";
		return;
	}
	
Pix temp = args.first();

	mailserver = args.remove_front();	//	args(temp); args.next(temp);
	to = args.remove_front();	//	args(temp); args.next(temp);
	from = args.remove_front();	//	args(temp);
	subj = args.remove_front();
	data = args.remove_front();
	
	if (mailserver && to && from && subj && data)
	{
		int ok = SendMail(mailserver->Value(),
			to->Value(),
			from->Value(),
			subj->Value(),
			data->Value());

		if (ok == FAIL)
			cout << "System: email - unable to send email.\n";
	}
}

#endif