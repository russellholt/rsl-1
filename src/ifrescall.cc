#include "ifrescall.h"
#include "restable.h"

//	extern restable References;

resource *ifResCall::execute(String& method, SLList<resource *> &args)
{
	cout << "\n<p>ifResCall::execute(method, args) not implemented.<p>\n";
	return NULL;
}

// execute
// carry out the "if" operation
// In pseudo code,
//
// it = evaluate resptr
// if IsTrue(it)
//		then
//			true_res.execute(vars)
//		else
//			false_res.execute(vars)
//
//
// 11/29/95 - added 'arguments' restable - this is the table of variables passed
// into the routine that this 'if' statement lives in.
resource *ifResCall::execute(
	restable *arguments,
	restable *globals,
	restable *globals2,
	restable *locals,
	restable *result_temps)
{
resource *theres = resptr;

	if (!globals && !globals2 && !locals)
		return NULL;	// no resources
	
	if (!result_temps)
		cerr << "ifResCall::execute(): Warning: no temorary results table.\n";
	
//	if (!FindResource(vars))	// ResCall::FindResource
//		return NULL;

	// check for scope-resolved name: look for  name + "::" + method_name
	String s_name = name + "::" + method_name;
	
	if (!theres && locals)	// in local variable table
		theres = locals->GetResource(s_name);
	if (!theres && arguments)	// in argument table
		theres = arguments->GetResource(s_name);
	if (!theres && globals)	// in global variable table
		theres = globals->GetResource(s_name);
	if (!theres && globals2)	// in global variable table
		theres = globals2->GetResource(s_name);
		// scope-resolved names aren't going to be in References or temps,
		// so we can skip checking those.. (right?? right?)
	if (!theres)	// not a scope-resolved name
	{
		if (!theres && locals)	// in local variable table
			theres = locals->GetResource(name);
		if (!theres && globals)	// in global variable table
			theres = globals->GetResource(name);
		if (!theres && globals2)	// in the other globals object table
			theres = globals2->GetResource(name);
		if (!theres)	// in reference object table
			theres = References.GetResource(name);
		if (!theres && result_temps)	// in temporary object table (probably won't be there)
			theres = result_temps->GetResource(name);
		if (!theres)	// still not found!!
			return NULL;	// oh well, I tried!
	}

	if (!theres->Enabled())
		return NULL;

	if (!true_res)
	{
		cout << "'if' requires a 'true' condition.\n";
		return NULL;
	}
	
	SLList<resource *> arglist2;
	resource *theresult;
	

	if (theres->ClassName() == "ResCall")
	{
#ifdef DEBUG
		cout << "\nIF: expression is a rescall:\n";
		String x;
		((ResCall *)theres)->TextEquiv(x);
		cout << x << "\n";
#endif
		theresult = ((ResCall *)theres)->execute(arguments, globals, globals2,
			locals, result_temps);
		// ResCall does result storage management internally

		// Pass on rc_commands
		// FYI -- restable consumes 'return',
		// looprescall consumes 'break' and 'continue'
		// However if we see these rc_commands we must stop and return them up
		// the call chain.
		if (theresult && theresult->ClassName() == "rc_command")
			return theresult;
	}
	else
	{
#ifdef DEBUG
		cout << "\nIF: expression is not a rescall\n";
#endif
		if (args.length() > 0)	// resource arguments need to be evaluated
			evaluate_args(args, arglist2, arguments, globals, globals2, locals, result_temps);	// ResCall::evaluate_args
	
		theresult = theres->execute(method_name, arglist2);
		if (theresult && theresult->ClassName() == "rc_command")
			return theresult;	// bye for now

		// store a temporary or local result
		StoreResult(theresult, locals, result_temps);
	}


	resource *which = NULL;

	if (theresult && theresult->LogicalValue() > 0)
	{
#ifdef DEBUG
		cout << "IF: evaluate true condition\n";
#endif
		which = true_res;
	}
	else
	{
#ifdef DEBUG
		cout << "IF: evaluate false condition\n";
#endif
		which = false_res;
	}
	
	if (which && which->Enabled())
	{
		if (which->ClassName() == "ResCall")
		{
#ifdef DEBUG
			cout << "IF: executing rescall\n";
#endif
			// execute it
			// must make the 'arguments' available to the statement!
			resource *retvalx =
			((ResCall *) which)->execute(arguments, globals, globals2, locals, result_temps);
			if (retvalx && retvalx->ClassName() == "rc_command")
				return retvalx;	// bye for now...

			// ResCall does result storage management internally
		}
		else
			if (which->ClassName() == "List")
			{
#ifdef DEBUG
				cout << "IF: executing rescall\n";
#endif
				// execute the restable - and give it temporary storage
				resource *retval= ((restable *) which)->execute(arguments, globals, globals2, locals, result_temps);
				if (retval && retval->ClassName() == "rc_command")
					return retval;	// bounce command back up.
			}
	}
	else
	{
#ifdef DEBUG
		cout << " which is NULL?\n";	
#endif
	}
	
	if (SaveResptr)
		resptr = theres;
	return NULL;
}

void ifResCall::print(void)
{
	String s;
	TextEquiv(s, 1);
	cout << s << '\n';
}

void ifResCall::TextEquiv(String &text, int showname)
{
String o,t,f;
	ResCall::TextEquiv(o, showname);
	
	text = String("if (") + o + String(")\n");
	if (true_res && true_res->ClassName() == "ResCall")
		((ResCall *)true_res)->TextEquiv(t, showname);
	text += String("\t") + t + String(";\n");
	if (false_res && false_res->ClassName() == "ResCall")
	{
		((ResCall *)false_res)->TextEquiv(f, showname);
		text += "else\n\t";
		text += f + String(";\n");
	}
}

