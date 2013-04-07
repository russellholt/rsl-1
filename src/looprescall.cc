#include "looprescall.h"
#include "restable.h"
#include "rc_command.h"

//	extern restable References;

resource *loopResCall::execute(String& method, SLList<resource *> &args)
{
	cout << "\n<p>loopResCall::execute(method, args) not implemented.<p>\n";
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
resource *loopResCall::execute(
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
		cerr << "loopResCall::execute(): Warning: no temorary results table.\n";
	
//	if (!FindResource(vars))	// ResCall::FindResource
//		return NULL;


	// check for scope-resolved name: look for  name + "::" + method_name
	// -- but only if (12/6/95 RFH) the method name is an identifier
	if (method_name.matches(RXidentifier))
	{
		String s_name = name + "::" + method_name;
		
//		if (!theres && locals)	// in local variable table
//			theres = locals->GetResource(s_name);
//		if (!theres && arguments)	// in argument table
//			theres = arguments->GetResource(s_name);
		if (!theres && globals)	// in global variable table
			theres = globals->GetResource(s_name);
		if (!theres && globals2)	// in global variable table
			theres = globals2->GetResource(s_name);
	}
		// scope-resolved names aren't going to be in References or temps,
		// so we can skip checking those.. (right?? right?)
	if (!theres)	// not a scope-resolved name
	{
		if (!theres && locals)	// in local variable table
			theres = locals->GetResource(name);
		if (!theres && arguments)	// in arguments table
			theres = arguments->GetResource(name);
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

	if (!loop_res)
	{
		cout << "'while' requires a looping conditional expression.\n";
		return NULL;
	}
	
	SLList<resource *> arglist2;
	resource *theresult;
	int doneLooping = 0, isRC = (theres->ClassName() == "ResCall")?1:0,
		loopRC = (loop_res->ClassName() == "ResCall")?1:0,
		loopList = (loop_res->ClassName() == "List")?1:0,
		count = 0;

#ifdef DEBUG
	int testSafetyMax = 300;
#endif

	resource *retval = NULL;

	while(!doneLooping)
	{
		if (isRC)
		{
	#ifdef DEBUG
			cout << "\nWHILE: expression is a rescall:\n";
			String x;
			((ResCall *)theres)->TextEquiv(x);
			cout << x << "\n";
	#endif
			theresult = ((ResCall *)theres)->execute(arguments, globals, globals2,
				locals, result_temps);
			// ResCall does result storage management internally
		}
		else	// this doesn't quite make sense. Does it happen?
		{
	#ifdef DEBUG
			cout << "\nWHILE: expression is not a rescall\n";
	#endif
			// resource arguments need to be evaluated
			// only on the first pass (count > 0) --- I guess!
			if (!count && args.length() > 0)
				evaluate_args(args, arglist2, arguments, globals, globals2, locals, result_temps);	// ResCall::evaluate_args
		
			theresult = theres->execute(method_name, arglist2);
	
			// store a temporary or local result
			StoreResult(theresult, locals, result_temps);
			
			// NOTE:
			// Should the temp results be destroyed after each iteration?
			// If not, then there is a practical limit on the number of
			// iterations when temp results are returned.. the table could
			// become large. Yeah.
		}
	
	
		resource *which = NULL;
	
		if (theresult && theresult->LogicalValue() == 0)
		{
	#ifdef DEBUG
			cout << "WHILE: loop condition is false.\n";
	#endif
			doneLooping = 1;
		}
	#ifdef DEBUG
		else
			cout << "WHILE: loop condition is true.\n";
	#endif
		
		if (!doneLooping)	//	which && which->Enabled())
		{
			if (loopRC)	// same as:	loop_res->ClassName() == "ResCall")
			{
	#ifdef DEBUG
				cout << "WHILE: executing rescall\n";
	#endif
				// execute it
				// must make the 'arguments' available to the statement!
				retval = ((ResCall *) loop_res)->execute(arguments, globals,
					globals2, locals, result_temps);
				if (retval && retval->ClassName() == "rc_command")
					doneLooping = 1;
				// ResCall does result storage management internally
			}
			else
				if (loopList)	//	loop_res->ClassName() == "List")
				{
	#ifdef DEBUG
					cout << "WHILE: executing rescall\n";
	#endif
					// execute the restable - and give it temporary storage
					retval = ((restable *) loop_res)->execute(arguments, globals,
						globals2, locals, result_temps);

					// Check for rc_commands
					// We are a consumer of 'Break' and 'Continue' rc_commands.
					// All others, like Exit and Return
					if (retval && retval->ClassName() == "rc_command")
					{
						if ( ((rc_command *) retval)->Type() == rc_command::Continue)
						{
							retval = NULL;	// DO NOT DELETE!!
							continue;
						}
						// other rc_commands - eg exit or return -- get out.
						// return retval;	// bounce back up & stuff.
						doneLooping = 1;
					}
				}

		}	// not done looping

		count++;

#ifdef DEBUG
		if (count > testSafetyMax)
			break;
#endif

	}	// ---- The Loop -------

//	else
//	{
//#ifdef DEBUG
//		cout << " which is NULL?\n";	
//#endif
//	}
	
//	if (SaveResptr)
//		resptr = theres;
	if (retval && retval->ClassName() == "rc_command")
	{
		int t = ((rc_command *) retval)->Type();

		// consume Break command
		//		(all rc_commands are an implicit break in a way)
		if (t == rc_command::Break)
		{
			retval=NULL;	// DO NOT DELETE!!!!! IT IS PART OF THE PROGRAM!
		}
	}
	// return others...
	return retval;
}

void loopResCall::print(void)
{
	String s;
	TextEquiv(s, 1);
	cout << s << '\n';
}

void loopResCall::TextEquiv(String &text, int showname)
{
String o,t,f;
	ResCall::TextEquiv(o, showname);
	
	text = String("while (") + o + String(")\n");

	if (loop_res && loop_res->ClassName() == "ResCall")
		((ResCall *)loop_res)->TextEquiv(t, showname);

	text += String("\t") + t + String(";\n");
}

