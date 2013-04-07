#include "R_Output.h"
#include "R_File.h"


/* execute
 *  route named method calls with 'resource' arguments.
 */
resource *R_Output::execute(String& method, SLList<resource *> &args)
{
	method.downcase();
	if (args.length() == 0)
	{
		cout << "out: no arguments passed for \"" <<  method << "\"\n";
		return NULL;
	}

	if (method == "<<" || method == "putto" || method == "print")
		PutTo(args);
	else
		if (method == "sendfile")
			SendFile(args);
	else
		cout << "Error: Output: Unknown method " << method << '\n';

	return NULL;
}

// PutTo - the crb "<<" operator
// Calls the inline PutTo for each argument value
void R_Output::PutTo(SLList<stringpair> &args)
{
Pix temp = args.first();
	
	for(temp = args.first(); temp; args.next(temp))
		cout << ArgVal(args(temp));
}

void R_Output::PutTo(SLList<resource *> &args)
{
Pix temp = args.first();
resource *r;

	for(temp = args.first(); temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;
		if (r->ClassName() == "File")
			((R_File *)r)->PrintFile();
		else
			r->print();
	}
}

void R_Output::SendFile(SLList<stringpair> &args)
{
	cout << "Output::SendFile(SLList<stringpair> &args) unimplemented\n";
}

void R_Output::SendFile(SLList<resource *> &args)
{
Pix temp = args.first();
resource *r;
R_File f;

	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (r && r->ClassName()=="String")
			f.PrintFile(r->Value());
		else
			cout << "SendFile: expected argument type String.\n";
	}
}