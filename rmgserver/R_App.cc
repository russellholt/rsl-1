// $Id: R_App.cc,v 1.12 1996/10/12 17:29:55 holtrf Exp $
//
// PURPOSE: online application class implementation
//
// PROGRAMMING NOTES:
//
// - incoming events are routed based on address2.  If there is an address2, 
//   (length non-zeroit is assumed to be the name of an existing subordinate 
//   object (like a Form), *not* data to be used by the event.  All incoming 
//   data should be stored in the datapairs list.
//
// - events without an addr2 *and* RSL method invocations through execute are 
//   both routed through EventCentral.
//
// HISTORY: long and painful
//
// Copyright 1995, 1996 by Destiny Software Corporation.

#include "resource.h"
#include "R_App.h"
#include "R_csap_obj.h"
#include "restable.h"
#include "slog.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "dResHandler.h"
#include "SLList_res_util.h"
#include "destiny.h"


extern unsigned int ha(const char *s0);
extern unsigned int collatz(unsigned int i);


// should not be here
#include "R_Form.h"

#ifdef PURIFY
#include "purify.h"
#endif


extern "C" {
#include <stdio.h>
}

const char * const rcsID = "$Id: R_App.cc,v 1.12 1996/10/12 17:29:55 holtrf Exp $";

extern restable References, *gSysGlobals;
extern int resource_lookups, Testing;
extern slog mylog;

extern timeout_manager to_mgr;
extern void DestroyResList(resource *owner, SLList<resource *>& thelist);


//
// App methods
//

#define _hADDAUTOEVENT 67889	// addautoevent
#define _hADDEVENT 21589	// addevent
#define _hALERT 4829	// alert
#define _hCLOSE 4812	// close
#define _hCLOSEALL 8302	// closeall
#define _hERROR 4711	// error
#define _hEXIT 2141	// exit
#define _hFORGETTIMEOUT 373	// forgettimeout
#define _hGETUSERDATA 126787	// getuserdata
#define _hGETUSERDATA2 270615	// getuserdata2
#define _hGETUSERDATA3 71931	// getuserdata3
#define _hGETUSERDATA4 270629   // getuserdata4
#define _hINIT 586	// init
#define _hINITOBJECT 8069	// initobject
#define _hINSTANCESEED 34117	// instanceseed
#define _hISUNIQUE 9568	// isunique
#define _hNOTUNIQUE 1264	// notunique
#define _hOPEN 2087	// open
#define _hOPENWINDOWS 84726	// openwindows
#define _hREGISTERTIMEOUT 99256	// registertimeout
#define _hREMOVE 1124	// remove
#define _hSETDATA 15817	// setdata
#define _hSETSYSTEMID 16316	// setsystemid
#define _hTIMEOUT 16312	// timeout
#define _hUSERDATA 27779	// userdata
#define _hUSERDATA2 76439	// userdata2
#define _hUSERDATA3 23259	// userdata3
#define _hUSERDATA4 76453	// userdata4


void R_App::Init(void)
{
//	username.SetName("Username");
	class_name = "App";
	hierarchy += "App:";
	next_object_id = 500;
	to_node = NULL;
	open_windows = 0;
}

resource *R_App::Create(String &nm, resource*& table)
{
	R_csap_obj *r = new R_App(nm);
	table = NULL;

	if (r)
	{
		// find the "Setup" script
		r->SetSetup();	// R_csap_obj::SetSetup()

		// set storage for variables declared in the rsl Create
		table = r->Locals();	// R_csap_obj::Locals()
		((restable *)table)->install_resource(r);	// add pointer to itself
	}
	return r;
}

// Clone
// return a copy of me
R_csap_obj *R_App::Clone(void)
{
	return new R_App(*this);
}

// Destructor
R_App::~R_App(void)
{
//	1. Kill everything in "locals"
	logf->Log(_DEBUG, LOGAPPENV, "~R_App() for %S.", &(instance_id.Value()));

	ResListDelete(locals.GetList(), RLD_NAME, "SystemID");	// owned by R_csap_obj
	ResListDelete(locals.GetList(), RLD_NAME, name.chars());	// pointer to myself
//	DestroyResList(this, locals.GetList());
	locals.FreeAll();
	CleanUp();

#ifdef DEBUG
	cout << "~R_App- removing from timeout list..\n";
#endif

	ForgetTimeout();
}

// ForgetTimeout
// Tell the timeout manager to forget about us.
void R_App::ForgetTimeout(void)
{
	if (to_node)
	{
		to_mgr.BeGone(to_node);
		to_node = NULL;
	}
}

// RegisterTimeout
// Inform the timeout manager that we want to be notified
// for inactivity (timeout event).
void R_App::RegisterTimeout(resource *arg)
{
	if (!to_node)
		to_node = to_mgr.AddObject((resource *) this);

	if (arg && arg->ClassName() == "Integer")
		to_node->max_idle = ((R_Integer *) arg)->intval();
}

// OutEvent
// Send outgoing events
void R_App::OutEvent(Event *e)
{
	if (!e)
		return;

	if (R_App::rh_owner)
	{
		if (e->type == EVENT_Open)
		{
			open_windows++;
			//	cerr << "\nApp: " << open_windows << " open windows.\n";
		}
		else
		if (e->type == EVENT_CloseWindow)
		{
			open_windows--;
			//	cerr << "App: " << open_windows << " open windows.\n";
		}

		e->address1 = instance_id.Value();	// set the "session id"
		R_App::rh_owner->OutEvent(e);
	}
}


// Setup
// - called for a session application
// - argument `e' is NULL by default.
void R_App::Setup(Event *e)
{
	// make instance ID available to all csap objects
	// owned by this App (their scripts)

	locals.install_resource(&instance_id);

	// test: change the name of the app already installed in locals
	// that is the same name as this (it won't be the same object -we're
	// the clone and it's the "original")
	
	// delete (by name) the pointer to the previous (original) object
	locals.Delete(name, String("name"));
//	resource *r = locals.GetResource(name);
//	if (r)
//		r->SetName(name + "_");	// see which one gets changed

	// add pointer to itself. See note in R_Form::Setup for info
	// about this (why it's done in R_App::Create() as well)
	locals.Insert(this);

	current_event = e;
	logf->Log(_DEBUG, LOGAPPENV, "R_App::Setup.. current ev \"%S\"",
		&(current_event->type));
	ExecuteSetup(&locals);
}

// CleanUp
// Destroy any temporary memory etc that was accumulated
// during event processing.
void R_App::CleanUp(void)
{
	// items marked for delayed deletion
//	DestroyResList(this, GoingAway);
	Pix temp=GoingAway.first();
		resource *r =NULL;
	for(; temp; GoingAway.next(temp))
	{
		r = GoingAway(temp);
		if (!r)
			continue;

#ifdef PURIFY
		if (purify_is_running())
			purify_printf("R_App::CleanUp() - delete \"%s\"", (r->Name()).chars());

		if (r->HierarchyContains(":csap_obj:"))
			delete (R_csap_obj *) r;
		else
			if (purify_is_running())
				purify_printf("not deleted: %s %s", (r->Name()).chars(), (r->ClassName()).chars());
#endif
	}
	GoingAway.clear();
}


// HandleEvent
// Front end to _HandleEvent
// -exists to do cleanup work.
int R_App::HandleEvent(Event *e, restable *obj_globals)
{
	if (!e)
		return 1;	// event was handled (it was just null!)

	if (to_node)
		to_mgr.Touch(to_node);

	// Process events
	Event *ev = e, *ev_next = NULL;

	int handled = 0;

	// Send individual events by breaking up the event chain.
	//		for(; ev; evlast=ev, ev=ev->next)
	while (ev)
	{
		ev_next = ev->next;	// save next pointer
		ev->next = NULL;

		handled += Handle1Event(ev, obj_globals);

		// destroy the event
		// this is not ev->kill() because that kills a chain
		// (but not the head!)
		delete ev;
		ev = ev_next;	// advance
	}
		
	CleanUp();	// delayed deletion
	
#ifdef SHOW_FORMS
	cout << "Active objects for App #" << instance_id.chars() << ":\n";
	printordered(locals.GetList(), "Form");
	cout << "-------(end form list)----\n";
#endif

	cout.flush();
	cerr.flush();
	
	if (handled && to_node)
		to_mgr.Touch(to_node);

	return handled;
}

// Handle1Event - where the real work is done
// here, obj_globals will be NULL because the res handler would
// otherwise pass the restable of active applications, which is
// not desirable. (is it?)
//
// Here, the Application grabs events relevant to
// an "application", and passes the rest on to the
// destination object (if there is one)
int R_App::Handle1Event(Event *ev, restable *obj_globals)
{
int handled=0;
resource *destination = NULL;
String &addr2 = ev->address2;
		
		// Try to route based on addr2
	if (addr2.length() > 0)
	{
		addr2.after(rmatch) = "";	// remove trailing junk
#ifdef DEBUG
		cerr << "App: " << instance_id
			 << "Route event to object " << addr2 << "\n";
#endif

		//	cerr << "Looking for:" << addr2 << '\n';

		//	destination = FindCSAPObject(addr2, &locals);

		// Find the destination CSAP object, but ignore 'this';
		// because there may be a pointer to 'this' in the locals
		// restable, and when the InstanceID (in addr2) of the
		// destination object matches the InstanceID of the R_App ('this')
		// we would result in infinite recursion ==> core dump.
		destination = FindCSAPObject(addr2, &locals, this);

/*
		if (destination)
			cerr << "\nFound " << destination->Name() << '\n';
		else
		{
			cerr << "\nNot found.. locals are:\n";
			printordered(locals.GetList());
			cerr << "\n-------\n";
		}
*/
		R_csap_obj *obj = (R_csap_obj *) destination;


		if (ev->type == EVENT_CloseWindow)
		{
			//	cerr << "\nCLOSEWINDOW!!!!!!\n";
			if (obj)
			{
				if (obj->IsOpen())
				{
					//	cerr << "Object is open..\n";
					open_windows--;
					if (open_windows <= 0) // send quit event
					{
						/************************************************************
						 * Run App event handler for "last window closed" condition * 
						 ************************************************************/
						Event *e = new Event("LastWindowClosed");
						if (RunEventHandler(e, obj->Name(), &locals) == 0)
						{
							// if no event handler, then revert to old behavior:
							// simply send quit event.
							logf->Log(_NOTICE, LOGAPPENV,
								"R_App: No event handler for LastWindowClosed.");

							// Want to quit
							Event *e2 = new Event(EVENT_Quit);
							OutEvent(e2);
						}
						delete e;	// but not e2! it gets deleted by dResHandler
					}

					/*----------------------------------------------*
					 *   Remove the "close window" target object.   *
					 *----------------------------------------------*/
					if (destination)
					{
						RemoveByPointer(destination);	// "delayed deletion"
						logf->Log(_DEBUG, LOGAPPENV, "R_App: Incoming Closewindow for %S.", &addr2);
					}
				}
				/*
				else
					cerr << "\n       OBJECT IS ALREADY CLOSED...\n";
				*/
			}
			/*
			else
				cerr << "\n        NO OBJECT FOUND\n";
			*/

			handled = 1;	// handled -- for both cases.
			return handled;
		}
		
		if (obj)
		{
			if (obj == this)
				logf->Log(_DEBUG, LOGAPPENV, "App: Skip recursive call to handleEvent...");
			else
				handled = obj->HandleEvent(ev, &locals);
		}

		if (!handled)
		{
			/*** need to disable annoying log message 
			logf->Log(_ERROR, LOGAPPENV, "App: %s[%S] event \"%S\" for %S %S %s",
				(obj? (obj->Name().chars()) : "<no object>"),
				&addr2, &(ev->type), &(ev->item), &(ev->field),
				"not found in event map.");
			*/

			// Alert the user/developer.
			if (Testing)
			{
				String errtext = addr2	// where it was headed
					+ String(": event \"")
					+ ev->type	// event type, eg "Action"
					+ String("\" for ")
					+ ev->item
					+ String("  ")
					+ ev->field
					+ String(" not found in event map.");
				Event *outevt = new Event("Alert");
				outevt->AddDataPair("MessageText", errtext);
				outevt->Source() = "RSL";
				OutEvent(outevt);
			}
		}
	}
	else
	{
		resource *result=NULL;
		// let user event handlers override built-ins
		handled = RunEventHandler(ev, name, &locals);
		if (!handled)
			handled = EventCentral(ev->type, ev->DataPairs, ev->source, result);
		// clean up whatever result.. (actually, probably won't occur)
		if (result && result->Exists() == exist_temp)
			delete result;
	}
	
#ifdef DEBUG
	if (!handled)
	{
		cerr << "R_App: Event '" << ev->type << "' from '" << ev->source
			 << "' to '" << addr2 << "' -- not handled.\n";
	}
#endif

	return handled;
}	// handle1event


// execute
// RSL interface
resource *R_App::execute(String& method, SLList<resource *> &args)
{
	resource *result = NULL;
	
	/* maybe filter events that aren't supposed to be accessible
	 * from RSL (should be switch selectable though)... */
	
	// Allow eventhandler script override of methods.
	Event *ne = new Event(method);
	int handled = 0;
	
	int h = RunEventHandler(ne, name, &locals);
	delete ne;
	if (h)
	{
		if (to_node)
			to_mgr.Touch(to_node);
		return NULL;	// it was handled -- get out
	}

	// call the central system
	// - would be nice if "source" could be the name of the script
	// that made this call (anything else, like the name of the form
	// that owns the script would be ridiculously difficult to obtain)
	if (EventCentral(method, args, String("RSL"), result))
	{
		if (to_node)
			to_mgr.Touch(to_node);

		//	CleanUp();
		return result;	// did
	}
	else
	{
		logf->Log(_ERROR, LOGAPPENV,
			"App: no method or event handler for \"%S\".", &method);
//		// event not handled yet..
//		// check local table for named resource!
//		resource *rft = locals.GetResource(method);
//		if (rft)
//			return rft;
	}
	
	return NULL;
}


// EventCentral
// the real "event handler", for which HandleEvent and execute act
// as interfaces.
int R_App::EventCentral(String &evname, SLList<resource *>& args,
	String source, resource*& result)
{
	result = NULL;	// for starters
	String m = downcase(evname);
	resource *fa = NULL;

	Pix temp = args.first();
	if (temp)
		fa = args.front();

	unsigned int hashv = ha(m);
	switch(hashv)
	{
		case _hTIMEOUT:	// "timeout"
			logf->Log(_NOTICE, LOGAPPENV, "App: Timeout event not handled by RSL.");
			return 1;

		case _hREGISTERTIMEOUT:	// "registertimeout"
			RegisterTimeout(fa);
			return 1;

		case _hFORGETTIMEOUT:	// "forgettimeout"
			ForgetTimeout();
			return 1;

		case _hOPEN:	// "open"
			result = Open(args);
			return 1;

		case _hINIT:	// "init"
		case _hINITOBJECT:	// "initobject"
			if (fa && fa->ClassName() == "String")
				result = InitObject(fa->Value());	// return the new object.
			return 1;

		case _hEXIT:	// "exit"
			return QuitSession(fa?0:1, source);

		case _hOPENWINDOWS:     // "openwindows"
			result = new R_Integer(open_windows);
			return 1;

		case _hSETDATA:	// "setdata"
			if (fa && fa->ClassName() == "List")
				SetDataPairVars(((restable *) fa)->GetList());
			return 1;

		case _hUSERDATA:	// "userdata"
		case _hUSERDATA2:	// "userdata2"
		case _hUSERDATA3:	// "userdata3"
		case _hUSERDATA4:	// "userdata3"
			SetDataPairVars(args);
			return 1;

		case _hGETUSERDATA:	// "getuserdata"
		{
			Event *e = new Event("GetUserData", "", //	(instance_id.Value()).chars(),
				"", "", "RSL");
			OutEvent(e);
			return 1;
		}
		case _hGETUSERDATA2:	// "getuserdata2"
		{
			Event *e = new Event("GetUserData2", "", //	(instance_id.Value()).chars(),
				"", "", "RSL");
			OutEvent(e);
			return 1;
		}
		case _hGETUSERDATA3:	// "getuserdata3"
		{
			Event *e = new Event("GetUserData3", "", //	(instance_id.Value()).chars(),
				"", "", "RSL");
			OutEvent(e);
			return 1;
		}
		case _hGETUSERDATA4:	// "getuserdata3"
		{
			Event *e = new Event("GetUserData4", "", //	(instance_id.Value()).chars(),
				"", "", "RSL");
			OutEvent(e);
			return 1;
		}

		case _hNOTUNIQUE:       // "notunique"
			result = new R_Boolean(1-(rh_owner==NULL?0:rh_owner->IsUnique(args)));
			return 1;
			break;

		case _hISUNIQUE:        // "isunique"
			result = new R_Boolean(rh_owner==NULL?0:rh_owner->IsUnique(args));
			return 1;
			break;

		case _hCLOSE:	// "close"
			Close(fa, source);
			return 1;

		case _hREMOVE:	// "remove"
			RemoveByName(fa);
			return 1;

		case _hCLOSEALL:	// "closeall"
			CloseAll(fa);
			return 1;

		case _hADDEVENT:	// "addevent"
			Add(args);	// R_csap_obj::Add
			return 1;

		case _hADDAUTOEVENT:	// "addautoevent"
			AddAuto(args);
			return 1;

		case _hSETSYSTEMID:	// "setsystemid"
			if (fa && fa->ClassName() == "String")
				SetSystemID(fa->Value());
			/*
			else
				cerr << "App::SetSystemID() requires a String argument.\n";
			*/
			return 1;

		case _hALERT:	// "alert"
			SendSimpleEvent("Alert", fa? (fa->Value()):String("---"), "RSL");
			return 1;

		case _hERROR:	// "error"
			SendSimpleEvent("Error", fa? (fa->Value()):String(""), "RSL");	// type, text, source
			return 1;

		case _hINSTANCESEED:	// "instanceseed"
			if (fa && fa->ClassName() == "Integer")
				next_object_id = (unsigned long) ((R_Integer *) fa)->intval();
			return 1;

		default: ;
	}

	// Event not handled
	return 0;
}

// CloseAll
// --- closes all objects except argument (pointer to object, not object name)
void R_App::CloseAll(resource *& fa)
{
	logf->Log(_DEBUG, LOGAPPENV, "%S: CloseAll except %S",
		&(InstanceID()), &(fa? fa->Value() : String("<none>")));

#ifdef DEBUG
	cerr << "CloseAll ";
	if (fa)
		cerr << " except " << fa->Name();
	cerr << '\n';
	// cerr << "App: " << instance_id << "CloseAll..";
#endif

	SLList<resource *>& l = locals.GetList();
	resource *r = NULL;
	Pix temp = l.first();

	while(temp)
	{
		r = l(temp);
		l.next(temp);
		if (r && r->HierarchyContains(":csap_obj:"))
			if (r != this && (!fa || fa != r))	// argument is exception to closeall
			{
#ifdef PURIFY
				if (purify_is_running())
					purify_printf("(CloseAll) Close for \"%s\"", (r->Name()).chars());
#endif
				((R_csap_obj *)r)->Close();
				RemoveByPointer(r);	// delayed deletion
			}
	}

/*
	locals.BeginIteration();
	while(r = locals.GetNextResource())
		if (r && r->HierarchyContains(":csap_obj:"))
			if (!fa || fa != r)	// argument is exception to closeall
			{
				((R_csap_obj *)r)->Close();
				RemoveByPointer(r);
			}
*/
	//	Close(args, source);
}

// QuitSession
// End and delete a session.
// - Only if called from within RSL
// - If any arguments, then don't send the event.
// - Calls reshandler's QuitSession (a delayed delete of this object)
int R_App::QuitSession(int sendevent, String source)
{
#ifdef DEBUG
	cout << "Got the quit event from source " << source << "\n";
#endif
	logf->Log(_INFO, LOGAPPENV, "App: Exit received from source \"%S\"..", &source);
	if (source == "RSL")
	{
		if (sendevent)
		{
			// send quit event
			Event *e = new Event("Quit");
			//	if (fa && fa->ClassName() == "String")
			//		e->AddDataPair("MessageText", fa->Value());
			e->Source() = source;
			OutEvent(e);
			return 1;
		}
		rh_owner->QuitSession(instance_id.Value());
		return 1;
	}
#ifdef DEBUG
	cout << "        (( not handled ))\n";
#endif
	return 0;
}	// QuitSession

// InitObject
// - allocate a subordinate object named by "object_name" from its template
// - returns the object created.
// RFH 1/24/96 (extracted from Open)
resource *R_App::InitObject(String object_name)
{
#ifdef PURIFY
	if (purify_is_running())
		purify_printf("R_App::InitObject() for %s...", object_name.chars());
#endif

R_csap_obj *inst = NULL;

	if (object_name.length() == 0)
	{
		mylog.Log(_ERROR, "App: %S InitObject - no object name specified.\n",
			&(instance_id.Value()));
		return NULL;
	}

#ifdef DEBUG
	cerr << "R_App::InitObject \"" << object_name << "\" in instance \""
		 << instance_id << "\"\n";
#endif

/** Find instance **/
	
	resource *r = NULL;

	// First, check to see if the named object already exists in
	// the locals (for this application)
	r = locals.GetResource(object_name);

	/* will -> */			// if not found by name,
	/* this -> */			// look for an object instance by ID
	/* ever -> */			//	if (!r)
	/* happen? */			//		r = FindCSAPObject(object_name, &locals);
	/* --> it would mean that an object (template?) exists in this App instance's */
	/* local table... worthwhile or not..? */


	if (r)	// object instantiation exists in local table
	{
		/* This case should be taken care of since Open was split into
		 * rInit. The sending of an Open event should be moved up into R_csap_obj for
		 * each subclass to call when necessary (through their Show method or what-have-you)
		 */

#ifdef DEBUG
		cerr << "App: object \"" << object_name << "\" exists in App locals. "
			 << "It is already initialized.\n";
#endif
		
		return r;
	}
	else	/** Find "template" **/
	// if still not found, then there is no instantiation of the
	// requested object for this application - get it from the "database"
	// (system global restable)
		r = gSysGlobals->GetResource(object_name);

	// make sure it's a csap object.. (GetResource probably could take
	// a string to pass to HierarchyContains to narrow the search, in
	// case there are duplicate names.)
	if (r && r->HierarchyContains(":csap_obj:"))
	{
#ifdef DEBUG
		cerr << "App: found csap obj \"" << r->Name() << "\"\n";
#endif
		inst = ((R_csap_obj *) r)->Clone();
		if (inst)
		{
			// add the new resource to the collection of objects
				// owned by this application (no duplicates...?)
			locals.install_resource(inst);	// add() would allow duplicates

				// Set the instance ID of the new object
			inst->SetInstanceID(NextObjectID());

				// this app owns the object
			inst->SetOwner(this);

#ifdef DEBUG
			cerr << "\nAbout to call C++ Setup for \"" << inst->Name() << "\"\n";
#endif

				// install local var instances & stuff
			inst->Setup();
#ifdef DEBUG
			cout << "R_App::InitObject - adding pointer to locals..\n";
			cout.flush();
#endif
			(inst->Locals())->Insert(inst);	// add pointer to head (faster access)

#ifdef DEBUG
			cerr << "App local table:\n";
			printordered(locals.GetList());

			cerr << "\nAbout to execute script \"Setup\" for \"" << inst->Name() << "\":...\n";
#endif
				// Run its setup script
			inst->ExecuteSetup(&locals);	// will pass arguments later
			/* Note: notice that the previous call, ExecuteSetup, differs
			 * from the way it was done for reshandler/app. Look at
			 * R_App::Setup() - it calls ExecuteSetup() itself. This is
			 * because an App Setup script needs to have access to the
			 * application local table (global to forms) --- and so does
			 * the form setup scripts. Calling ExecuteSetup from R_Form::Setup
			 * would not give it this access
			 */

#ifdef DEBUG				
			cerr << "* * * * * App locals table * * * * *\n";
			printordered(locals.GetList());
			cerr << "* * * *\n";
#endif
		}
		else
			mylog.Log(_ERROR, "App: Init- unable to copy object \"%S\"!\n", &(r->Name()));
		
	}
	else	// not found, or not a CSAP object
		{
			logf->Log(_ERROR, LOGAPPENV, "%s \"%S\" %s\n",
					"R_App: Unable to send Open event.", &object_name,
					"not found, or is not a csap object.");
		}
	return inst;
}

// Quit event
// From rsl: App.Quit();
// optional text string to send...
void R_App::Quit(String text)
{
	SendSimpleEvent("Quit", text, "RSL");
}

// Open
// - Create the requested object
// - Send the open event.
// -----
// This will be changed to call InitObject() followed by calling Open() for the
// newly allocated object.
//
//void R_App::Open(String object_name)
resource *R_App::Open(SLList<resource *>& args)
{
R_csap_obj *inst = NULL;
	if (args.length() <= 0)
	{
		mylog.Log(_ERROR, "App: %S Open - no object name specified.\n",
			&(instance_id.Value()));
		return NULL;
	}

	resource *r = args.front();
	String object_name = r->Value();
	
	Pix temp = args.first();
	args.next(temp);	// ready to get argument #2
	
	if (object_name.length() == 0)
	{
		mylog.Log(_ERROR, "App: %S Open - no object name specified.\n",
			&(instance_id.Value()));
		return NULL;
	}

#ifdef DEBUG
	cerr << "R_App::Open \"" << object_name << "\" in instance \""
		 << instance_id << "\"\n";
#endif

	//***************************
	//*   Create the new object
	//***************************

	r = InitObject(object_name);
	if (r && r->HierarchyContains(":csap_obj:"))
		inst = (R_csap_obj *) r;
	else
		return NULL;

	/* Set Variables given in argument list:
	 * - Walk through the rest of the argument list (after first arg, the name of
	 *   the object template to open).
	 * - add each object to the new object's local table as variables.
	 * - At the current time, only string variables are supported. This is because
	 *   they are ** copied ** and there is no resource::Clone(). */
	restable *instlocals = inst->Locals();
	for (; temp; args.next(temp))
	{
		resource *newr = NULL, *r = args(temp);
		if (r && r->ClassName() == "String")
		{
			newr = new R_String( * ((R_String *) r));
			instlocals->AddOrReplace(newr);
		}
	}

	//*
	//* Send the open event (or "reopen"... whichever)
	//*
	inst->Open();
	
	/*
	 * Run its setup script
	 * -- but only if it hasn't been run before
	 * ---> should have been run by InitObject() above, but here for good measure...
	 */
	if (!(inst->IsSetup()))
	{
#ifdef DEBUG
		cerr << "\nAbout to execute script \"Setup\" for \"" << inst->Name() << "\":...\n";
#endif
		inst->ExecuteSetup(&locals);
	 }		

	return inst;
}

// Remove
// - removes an object from the Application local table,
//   and deletes the object.
/* - takes a resource* (eg, an R_String *) whose Value()
	is used to find, by Name(), the resource to remove. That is,
	saying in RSL `` app.remove("f210") '' sends an R_String* whose
	Value() is "f210", which is matched against resources to find
	one whose Name() is "f210".
*/
void R_App::RemoveByName(resource *resname)
{
	if (!resname)
	{
		logf->Log(_DEBUG, LOGAPPENV, "R_App::Remove requires an object name.\n");
		return;
	}
	
	Remove(resname->Value());
}

// RemoveResource
// `theresource' is a pointer to the actual resource to remove!
void R_App::RemoveByPointer(resource *respointer)
{
	if (!respointer)
		return;

#ifdef PURIFY
	if (purify_is_running())
		purify_printf("R_App::RemoveByPointer object \"%s\"", (respointer->Name()).chars());
#endif
	resource *theobj = ResListDelete(locals.GetList(), RLD_POINTER, respointer);
	innerRemove(theobj);
}

// Remove
// - NO CLOSE EVENT IS SENT; a close event should be sent prior to calling
//   this function.
// RFH Jan 26/96 -- extracted from ::Close
void R_App::Remove(String what)
{
	/* delete (by name) from the Application local table */
#ifdef DEBUG
	cout << "R_App::Remove: " << what << "\n";
#endif

//	resource *theobj = locals.Delete(what, String("name"));
	resource *theobj = ResListDelete(locals.GetList(), RLD_NAME, what.chars());

	if (theobj)
	{
#ifdef PURIFY
		if (purify_is_running())
			purify_printf("R_App::Remove object \"%s\"", (theobj->Name()).chars());
#endif

		innerRemove(theobj);
#ifdef SHOW_FORMS
		cout << "R_APp::Remove: local table after being removed:\n";
		printordered(locals.GetList());
#endif
	}
}

// innerRemove
// Log the action and mark the object for delayed deletion.
void R_App::innerRemove(resource *theobj)
{
	if (theobj)
	{
		logf->Log(_INFO, LOGAPPENV,
			"R_App::Remove- deleting %S from App id %S",
			&(theobj->Name()),
			&(InstanceID()) );

//		if (!Testing)	// safety check: (server -d option not given)
//			delete theobj;

		// Delayed deletion for safety. An rsl procedure may remove the form
		// it belongs to and it would kill context not to mention bashing some bits.

		GoingAway.append(theobj);
	}
#ifdef PURIFY
	else
		if (purify_is_running())
			purify_printf("R_App::innerRemove-- null object pointer!");
#endif
}

// Close
// - close an object
// - args should contain one R_String* which is the name of the object
//   (not an instance id- the name should be unique in R_App::locals)
void R_App::Close(resource *r, String source)
{
//	cerr << "\nR_App::Close --- instance " << InstanceID() << '\n';
	
	if (!r)
	{
		logf->Log(_ERROR, LOGAPPENV, "App::Close() requires an object name.\n");
		return;
	}
	
	String val = r->Value(), thewid;

#ifdef DEBUG
	cout << "R_App::Close -- removed " << val << ". table is now:\n";
	printordered(locals.GetList());
#endif

	resource *theobj = locals.GetResource(val);

	if (theobj)
	{
		if (source == "RSL"	// came from a script
					&& theobj->HierarchyContains(":csap_obj:"))
			((R_csap_obj *) theobj)->Close();

	//		if (!Testing)	// server cline opt -d will skip this delete

		RemoveByPointer(theobj);	// "delayed deletion"

#ifdef DEBUG
		cerr << "R_App::Close- deleted \"" << r->Value
			<< "\". App local table is now:\n";
		printordered(locals.GetList());
#endif
	}
	else
		logf->Log(_ERROR, LOGAPPENV, "App.Close: \"%S\" not found.", &val);
}


// NextObjectID
// - return the object ID as a String, and bump the counter
String R_App::NextObjectID(void)
{
	char *s = new char[12];
	// %lu = unsigned long int
	sprintf(s, "%lu", next_object_id++);

	String ret = s;
	delete[] s;

	return ret;
}

void R_App::print(void)
{

	cout << "Application ID \"" << instance_id << "\"";	//	<< username;

}



