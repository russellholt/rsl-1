// R_csap_obj.cc
//
// PURPOSE:
//
// USAGE:
//
// PROGRAMMING NOTES:
//
// HISTORY:
//	12/18/95 RFH - added "field" to event adding/matching
//
// $Id: R_csap_obj.cc,v 1.7 1996/10/12 17:30:10 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.

#include "R_csap_obj.h"
#include "Event.h"

#include "resource.h"
#include "restable.h"
#include "R_Integer.h"
#include "R_String.h"
#include "slog.h"
#include "SLList_res_util.h"
#include "destiny.h"
#include "purify.h"

extern restable References, *gSysGlobals;
extern slog mylog;

// destructificatalizer
R_csap_obj::~R_csap_obj(void)
{
	// the event map should only be destroyed for CLONEs..
	// so there probably should be a "IAmAClone" flag in R_csap_obj...
	// .. which would have to be set for every R_csap_obj that was
	// declared in Main() ..
	// - otherwise, just doing the below would kill the event map for
	// all instantiations of the object f214, for example.
	//
	// Until then, it is commented out.
/*
	if (evtmap)
	{
		// delete all event map structures
		Pix temp = evtmap->first();
		struct emap *e = NULL;
	
		for(; temp; evtmap->next(temp))
		{
			e = (*evtmap)(temp);
			if (e)
				delete e;
		}
		// delete event map list
		delete evtmap;
	}
*/

	// DO NOT DELETE setup or owner!!!! They are not created by R_csap_obj.

	// Same as eventmap for system_id
/*
	if (system_id)
		delete system_id;
*/
}

// Initializifier
void R_csap_obj::Init(void)
{
	isOpen = isSetup = quitting = 0;
	current_event = NULL;
	owner = NULL;
	system_id = new R_String("SystemID", "");
	locals.install_resource(system_id);	// pointer will be copied for all instances

	instance_id.SetName("InstanceID");
	evtmap = NULL;
	setup = NULL;
	hierarchy += "csap_obj:";
	
	// Set the existence of all member resources so that they aren't
	// accidentally deleted from various restables that they might be
	// put into during execution: very important.
	
	instance_id.MakeExist(exist_perm);
	system_id->MakeExist(exist_perm);
	locals.MakeExist(exist_perm);
}

// Open virtual function
void R_csap_obj::Open(void)
{
	// does nothing.
}

// Close virtual function
void R_csap_obj::Close(void)
{
	// does nothing
}

// SetSetup
// Find a "setup" restable for this csap_obj
void R_csap_obj::SetSetup(void)
{
	if (gSysGlobals)
	{
		String x = name + "::Setup";	// eg, "f4a::Setup"
#ifdef DEBUG
		cerr << "R_csap_obj:SetSetup(): looking for " << x << '\n';
#endif
		resource *st = gSysGlobals->GetResource(x);
		if (st)
#ifdef DEBUG
		{
			cerr << "R_csap_obj: found script \"" << st->Name() << "\"\n";
#endif
			setup = (restable *) st;
#ifdef DEBUG
		}
		else
			cerr << "R_csap_obj: unable to find Setup script.\n";
#endif
	}
}

// Add
// 
void R_csap_obj::Add(SLList<resource *> &args)
{
	
#ifdef DEBUG
	mylog.Log(_DEBUG, "R_csap_obj: Add:\n");
	printordered(args);
#endif

	if (args.length() <= 0)
	{
		mylog.Log(_ERROR, "No arguments to AddEvent.\n");
		return;
	}
	
String event,	// eg, "ButtonPress"
	resname, resclass,
	item,		
	field;
restable *script = NULL;	// restable to be executed
resource *r = NULL;	// temp
Pix temp = args.first();

	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;
		resname = downcase(r->Name());

		if (resname == "event" && r->ClassName() == "String")
			event = r->Value();
		else
		if (resname == "item")
			item = r->Value();
		else
		if (resname == "field")
			field = r->Value();
		else
		if (r->ClassName() == "List")
			script = (restable *) r;
	}
	
	AddEvent(event, item, field, script);
}


// AddAuto
// Add an "auto" event handler. This is a method name and argument list
// which is applied to the object which owns the event handler through
// the execute method.
// RFH 5:32 am Jan 26, 1996
void R_csap_obj::AddAuto(SLList<resource *>& args)
{
String event, item, field, resname;
String method;	// will be the first unrecognized resource name
SLList<resource *> arrgghhhs;

Pix temp = args.first();
resource *r = NULL;

	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;
		resname = downcase(r->Name());

		if (resname == "event" && r->ClassName() == "String" && event.length() == 0)
			event = r->Value();
		else
		if (resname == "item" && item.length() == 0)
			item = r->Value();
		else
		if (resname == "field" && field.length() == 0)
			field = r->Value();
		else
		if (r->ClassName() == "String" && method.length() == 0)
			method = r->Value();
		else
			arrgghhhs.append(r);
	}
	
	AddAutoEvent(event, item, field, method, arrgghhhs);
}


// AddEvent
// Add to the list of event type, item, and script restable.
void R_csap_obj::AddEvent(String &nm, String &it, String &fi, restable *sc)
{
	if (!sc)
	{
	/*
		logf->Log(_ERROR, LOGAPPENV,
			"%S.AddEvent: No script found for {%S,%S,%S}",
			&name, &nm, &it, &fi);
	*/
		cerr << name << ".AddEvent: No script found for "
			 << form("{%s,%s,%s}\n", nm.chars(), it.chars(), fi.chars());
		return;
	}

#ifdef DEBUG
	mylog.Log(_DEBUG,
		"AddEvent (event:\"%S\", item:\"%S\", field:\"%S\", \"%S\")\n",
		&nm, &it, &fi, &(sc->Name()));
#endif

	emap *e = new emap;
	if (e)
	{
		e->type = emap_script;	// RFH Jan 26, 1996
		e->evname = nm;
		e->item = it;
		e->field = fi;
		e->scr = sc;
		if (!evtmap)
			evtmap = new SLList<emap *>;
		if (evtmap)
			evtmap->append(e);
		else
		{
			/*
			cerr << name << ".AddEvent - unable to create event map.\n";
			logf->Log(_ERROR, LOGAPPENV,
				"AddEvent: out of memory for %S in %S", &nm, &name);
			*/
		}
	}
}


// AddAutoEvent
void R_csap_obj::AddAutoEvent(String& nm, String& it, String& fi,
	String& meth, SLList<resource *>& autoargs)
{
	emap_execute *em = new emap_execute;
	if (em)
	{
		em->type = emap_auto;
		em->evname = nm;
		em->item = it;
		em->field = fi;
		em->scr = NULL;	// no restable for auto events
		em->method = meth;
		em->args = autoargs;	// node-by-node copy-- resource pointers copied (ok)
		if (!evtmap)	// R_csap_obj::evtmap
			evtmap = new SLList<emap *>;
		if (evtmap)
			evtmap->append(em);
		else
		{
			// cerr << name << ".AddEvent - unable to create event map.\n";
			logf->Log(_ERROR, LOGAPPENV,
				"AddAutoEvent: out of memory for %S in %S", &nm, &name);
		}

	}
	
}

// MapEvent
// For a given set of event items, find a corresponding event map
// handler object, if it exists. This used to be GetScript which
// returned not the event map object but the restable owned by it.
// - This will still work fine if no "item" or "field" was specified in
//   AddEvent.
// RFH - modified from old GetScript Jan 26, 1996
R_csap_obj::emap *R_csap_obj::MapEvent(String& event, String& item, String& fi)
{
	if (!evtmap)
		return NULL;

String devent = downcase(event);
		
Pix temp = evtmap->first();
emap *e = NULL;

	for(;temp; evtmap->next(temp))
	{
		e = (*evtmap)(temp);
		if (!e) continue;
//		mylog.Log(_DEBUG, "Event: \"%S\", item: \"%S\", field: \"%S\"\n",
//			&(e->evname), &(e->item), &(e->field));

		if (downcase(e->evname) == devent)
		{
		int ok=1;	// ok so far

			if (e->item == "" || e->item == item )
				ok++;
			if (e->field == "" || e->field == fi)
				ok++;

			if (ok == 3)
				return e;
		}
	}

	return NULL;
}

// GetScript
// For a given event type and item, find them
// in the eventmap and return the corresponding restable (script)
restable *R_csap_obj::GetScript(String& event, String& item, String& fi)
{
emap *e = MapEvent(event, item, fi);
	if (e)
		return e->scr;
	
	return NULL;
}

// Setup
// execute the setup script.
void R_csap_obj::ExecuteSetup(restable *appobjects)
{
	if (!setup)	// no setup script declared in this::create (rsl)
	{
		logf->Log(_INFO, LOGAPPENV, "ExecuteSetup: No setup script for %S.",
			&name);	// resource::name
		return;
	}

//	if (!locals)
//		locals = new restable;

	/* Setup needs access to the system globals
	 * no arguments (NULL) */
	if (!isSetup)
	{
		// object order to execute is important:
		// params are <args>, <globals_1>, <globals_2>, <locals>
		// Object search order is:
		// 1. locals, 2. arguments, 3. globals_1, 4. globals_2
		// Objects in the ``appobjects'' global table should override
		// those in gSysGlobals (eg, an application object!)
		// -- A possible enhancement is to provide an ``arguments'' table
		// (to replace the NULL in param 1) for passing in data other than
		// through global variables.
		restable scargs;
		scargs.MakeExist(exist_perm);
		if (current_event)
		{
			logf->Log(_DEBUG, LOGAPPENV,
				"R_csap_obj::ExecuteSetup - creating EventData");
			restable evdata("EventData");
			evdata.MakeExist(exist_perm);
			evdata.Add(current_event->DataPairs);
			scargs.Add(&evdata);
			//	printordered(scargs.GetList());
			current_event = NULL;
			setup->execute(&scargs, appobjects, gSysGlobals, &locals);
		}
		else
		{
			logf->Log(_DEBUG, LOGAPPENV, "R_csap_obj::ExecuteSetup - no current event");
			setup->execute(NULL, appobjects, gSysGlobals, &locals);
		}
		isSetup = 1;
	}

#ifdef DEBUG
	else
		cerr << "ExecuteSetup: this object has already run its setup script...\n";
#endif

}

// HandleEvent
// For an event type, find it in the event list and execute the
// corresponding RSL script.
// Default functionality - override to change
int R_csap_obj::HandleEvent(Event *e, restable *obj_globals)
{
	if (!e)
	{
		logf->Log(_NOTICE, LOGAPPENV, "R_csap_obj: HandleEvent- NULL event to %S!",
			&name);
		return 0;
	}

	int handled = RunEventHandler(e, name, obj_globals);

	if (!handled)
		logf->Log(_NOTICE, LOGAPPENV,
			"%S %S: HandleEvent - no event handler for \"%S\" for item %S.",
			&class_name, &name, &(e->type), &(e->item));

	return handled;
}

// RunEventHandler
// Find an event handling script for the given event
// and execute it, using obj_globals as a global object table.
// RFH Jan 23/96 Extracted from ::HandleEvent
//   - set automatic resources to exist_perm so they won't be accidentally
//     destroyed by RSL.. (hate those errors)
int R_csap_obj::RunEventHandler(Event *e, String caller, restable *obj_globals)
{
//	restable *scr = GetScript(e->type, e->item, e->field);
	if (!e)
	{
		/*
		cout << "R_csap_obj::RunEventHandler NULL event from \"" << resource::name
			<< '\n';
		*/
		return 1;	// handled (but it was null!!)
	}
	emap *evhandler = MapEvent(e->type, e->item, e->field);
	if (!evhandler)
		return 0;	// not handled
	
	if (evhandler->type == emap_auto)
	{	/* Auto Event: call ::execute with give method and args!! (this is cool)
		 * -This means that in response to the event, a method in the "this"
		 *  object will be invoked (with optional arguments) */

		/* may want to send event data to method somehow?? */
		current_event = e;	// that's how
		emap_execute *ee = (emap_execute *) evhandler;

		resource *result = execute(ee->method, ee->args);
		
		/* what to do with the result...? Check for error or status or something..? */
		if (result && result->Exists() == exist_temp)
		{
			// cerr << "R_csap_obj::RunEventHandler - autoevent: deleting temp result.\n";
			delete result;
		}
		current_event = NULL;	// clear it
		return 1;	// handled
	}
	else
	if (evhandler->scr)
	{	/* event Handler: execute script in response. */
		/* Set eventdata variables */
		R_String ei, et, ef, ob;
		ei = e->item; et = e->type; ef = e->field;
//		ob = name;	// the name of the object
		ob = caller;

		ei.MakeExist(exist_perm);
		et.MakeExist(exist_perm);
		ef.MakeExist(exist_perm);
		ob.MakeExist(exist_perm);

		restable scriptargs, datapairs;
		scriptargs.MakeExist(exist_perm);
		datapairs.MakeExist(exist_perm);

		ei.SetName("EventItem");
		et.SetName("EventType");
		ef.SetName("EventField");
		ob.SetName("EventObject");
		scriptargs.Add(&ei);
		scriptargs.Add(&et);
		scriptargs.Add(&ef);
		scriptargs.Add(&ob);
		datapairs.Add(e->DataPairs);
		datapairs.SetName("EventData");
		scriptargs.Add(&datapairs);
	/*
		scriptargs.SetName("EvHandlerInfo");
		locals.Add(&scriptargs);
	*/

//		if (!locals)	// should be allocated in Setup
//			locals = new restable;	// but here for safety
		/* no arguments, no globals2 */
		evhandler->scr->execute(&scriptargs, obj_globals, gSysGlobals, &locals);
	//	ResListDelete(locals.GetList(), RLD_POINTER, &scriptargs);
		
		if (purify_is_running())
			purify_printf("    finished event handler \"%s\"", 
				(evhandler->scr->Name()).chars());

		return 1;	// handled
	}
	logf->Log(_ERROR, LOGAPPENV, "%S: Event script missing for {%S,%S,%S}",
		&name, &(e->type), &(e->item), &(e->field));
	return 0;	// not handled
}


// SetDataPairVars
// given a list of R_String (expected) resources, install
// each one into the {csap_obj instance}-wide restable.
// For example, the "userdata" event, having a list of
// items such as {"name", nameval}, {"service", serviceval} etc,
// each of these will be a String resource available to all
// scripts of an application, automagically
void R_csap_obj::SetDataPairVars(SLList<resource *>& datapairs)
{
resource *r = NULL;
	for (Pix temp = datapairs.first(); temp; datapairs.next(temp))
	{
		r = datapairs(temp);

		if (r)		//	&& r->ClassName() == "String")
			// probably should explicitly set resource existence
			// - are they permanent? when created by the
			locals.install_resource(r);
	}
}

// SendSimpleEvent
// Send a simple outgoing event.
void R_csap_obj::SendSimpleEvent(const char *evtype, String text, const char *src)
{
	Event *e = new Event(evtype);
	if (e)
	{
		e->source = src;
		if (text.length() > 0)
			e->AddDataPair("MessageText", text);
		OutEvent(e);	// OutEvent will stamp the event with a session ID.
	}
}


// FindCSAPObject
// non-member function
resource *FindCSAPObject(String& obj, restable *where, resource *ignore)
{
#ifdef DEBUG
	cerr << "FindCSAPObject: ";
#endif
	if (!where)
	{
#ifdef DEBUG
		cerr << "NULL restable.\n";
#endif
		return NULL;
	}
	
	SLList<resource *>& rlist = where->GetList();	//	gSysGlobals->GetList();
	Pix temp = rlist.first();
	resource *r = NULL;

#ifdef DEBUG
	cerr << "looking for \"" << obj << "\":\n";
#endif
	int count=0;	// strictly for debugging

	for(; temp; rlist.next(temp))
	{
#ifdef DEBUG //----------------------------------
		if (r)
		{		// print debugging info
			cerr << '(' << r->Name() << ") ";
			if (++count % 5 == 0)
				cerr << '\n';
		}
#endif //----------------------------------------

		r = rlist(temp);
		if (r && r->HierarchyContains(":csap_obj:"))
		{
			// Specify a resource to ignore, such as R_App::Handle1Event()
			// which searches for CSAP objects, and calls HandleEvent on them:
			// it may have a pointer to itself in its local table, and may
			// be searching for a CSAP object which has the same InstanceID()
			// as itself!!!
			if (ignore && ignore == r)
				continue;

			/*
			cerr << r->Name() << "=("
				 << ((R_csap_obj *) r)->InstanceID() << ")  ";
			*/

			if (((R_csap_obj *) r)->CompareInstanceID(obj))
			{
#ifdef DEBUG
				cerr << "--> found " << ((R_csap_obj *) r)->InstanceID() << '\n';
#endif
				return r;
			}
		}
	}
#ifdef DEBUG
	cerr << "could not find " << obj << '\n';
#endif

	logf->Log(_DEBUG, LOGAPPENV, "FindCSAPObj: \"%S\" not found in %s.",
		&obj, (where? (where->Name()).chars() : "<NULL resource table!>"));

	return NULL;
}

