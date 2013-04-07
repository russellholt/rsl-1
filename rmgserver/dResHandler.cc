// dResHandler.cc
//
// PURPOSE: application/resource handler. see dResHandler.h
//
// PROGRAMMING NOTES:
//
// HISTORY: 11/14/95
//
// $Id: dResHandler.cc,v 1.6 1996/10/12 17:30:17 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.
#include "dResHandler.h"
#include "slog.h"
#include "Event.h"
#include "R_App.h"
#include "R_Integer.h"
#include <SLList.h>
#include <String.h>
#include "destiny.h"
#include "SLList_res_util.h"

#include "purify.h"

extern restable *gSysGlobals;
extern R_String *startup_app_name, *maxUserMessage;
extern R_Integer *maxUsers;
extern timeout_manager to_mgr;

extern "C" {
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
extern int getrusage(int who, struct rusage *rusage);
extern caddr_t sbrk(int);
}
extern edata;
extern etext;
extern end;

extern int Testing;	// rmgserver.cc

slog mylog;


void dResHandler::Init(void)
{
	outgoing_head = outgoing_tail = NULL;
	startupapp = NULL;	//// obsolete ////
	deleteable = NULL;
	current_apps = total_apps = max_apps = max_concurrent = 0;
	to_node = NULL;
	doneThisPeriod = doneLastPeriod = 0;
}


// HandleEvent
// Deals with create session and quit session events, all
// others are routed to the appropriate application (session) via
// Event::address1 string (which is expected to be a resource name
// of an application in the activeApps restable member!)
//
// --Should only be called from the server!!!!
void dResHandler::HandleEvent(Event *e)
{
	if (!e)
		return;
		
	DeleteOutgoingEvents();

static Regex matchPasscode("[0-9][0-9][0-9][0-9]");		
String type = e->type;
String app = e->address1;
SLList<resource *>& dp = e->DataPairs;

#ifdef DEBUG
	cerr << "ResHandler::Handle Event - chain: ";
	e->print();
#endif

	logf->Log(_NOTICE, LOGAPPENV, "dResHandler::InEvent %S {%S,%S,%S} %S %S",
	&(e->Type()), &(e->Address1()), &(e->Address2()), &(e->Address3()),
	&(e->Item()), &(e->Field()));

	if (e->DataPairs.length() > 0)
	{
		Pix temp = dp.first();
		resource *r = NULL;
		String log_nm, log_val;

		for(; temp; dp.next(temp))
		{
			r = dp(temp);
			if (!r)	continue;

			log_nm = r->Name();
			log_val = r->Value();

			/* BEGIN BoA RMG PASSCODE HACK */

				// This should eventually call an overridable function
				// (maybe just a function pointer) which would be set for
				// each specific environment.  Could be written in RSL.

				if (log_nm == "2" && log_val.matches(matchpasscode))
					log_val = "xxxx";	// mask important data (PIN)

			/* END RMG PASSCODE HACK */

			logf->Log(_NOTICE, LOGAPPENV, "  (%S, %S)",
			&log_nm, &log_val);
		}
	}

	
	if (type == EVENT_CreateSession)
	{
		// Event::item holds the new session id
		R_csap_obj *ra = CreateSession(e);

		// kill the event
		delete e;
		e = NULL;
	}
	else
	{
		/********************************************
		 *  Route event to an application session   *
		 ********************************************/

#ifdef DEBUG
		cerr << "ResHandler: routing event to ID " << app << '\n';
#endif
		// find the destination based on instance ID (session)
		resource *r = FindCSAPObject(app, &activeApps); //	gSysGlobals);
		// make sure it's a csap application
		int handled=0;
		if (r && r->HierarchyContains(":App:"))	// already know it's a csap obj
		{
			handled = ((R_App *) r)->HandleEvent(e);

			/**********************************************
			 *		Post- event handler processing
			 **********************************************/
			
			/* 1. Quit:
			 * This is executed if there are no more open windows in the
			 * application as well. */

		/********
			if (((R_App *) r)->OpenWindows() <= 0)
			{
				if (((R_csap_obj *) r)->IsQuitting() == 0)
				{
					cout << "   dResHandler: Calling C++ QuitSession()...\n";
					QuitSession(app);
				}
				handled = 1;
			}
		******/

			if (!handled)
				logf->Log(_DEBUG, LOGAPPENV,
					"dResHandler: Unhandled event: \"%S\" to \"%S\".",
					&type, &app);
		}
		else
			logf->Log(_NOTICE, LOGAPPENV,"dResHandler::HandleEvent- could not find app \"%S\".", &app);

	}
		
#ifdef DEBUG
	cerr << "================== ResHandler Handle event (done)::\n";
	if (outgoing_head)
		outgoing_head->print();
	cerr << "\n\n";
#endif

	if (deleteable)
	{
		if (purify_is_running())
			purify_printf("Deleting \"delayed\" app..");

		delete deleteable;
		deleteable = NULL;

		if (purify_is_running())
			purify_new_leaks();
	}
}

// OutEvent
// What to do with events that are going outside.
// Could be overriden in a more dynamic res handler, that sends events
// immediately instead of queueueueueueueing them up.
void dResHandler::OutEvent(Event *e)
{
	if (!e)
	{
		logf->Log(_ERROR, LOGAPPENV, "ResHandler::OutEvent - null event.");
		return;
	}

#ifdef DEBUG
	cerr << "dResHandler::OutEvent: ";
	e->print(-1);
#endif

	logf->Log(_NOTICE, LOGAPPENV, "ResHandler::OutEvent %S {%S,%S,%S} %S %S",
		&(e->Type()), &(e->Address1()), &(e->Address2()), &(e->Address3()),
		&(e->Item()), &(e->Field()));
	
	if (outgoing_tail == NULL)
		outgoing_head = outgoing_tail = e;
	else
	{
		outgoing_tail->next = e;
		outgoing_tail = e;
	}
	
}

// CreateSession
// installs a new R_App to the active App list
R_csap_obj *dResHandler::CreateSession(Event *in_ev)
{
	if (!in_ev)
		return NULL;

String sessionid = in_ev->item;
/*
	logf->Log(_NOTICE, LOGAPPENV, "   Process data limit: %d",
		(int) sbrk(0));
*/
	struct rusage rut;
	getrusage(RUSAGE_SELF, &rut);

	logf->Log(_NOTICE, LOGAPPENV, "   Process RSS: %d, BRK: %d",
		(int) rut.ru_idrss, (int) sbrk(0));

	if (purify_is_running())
	{
		// get ready to record the amount of new memory in use
		purify_printf("Creating Session %s...", sessionid.chars());
		purify_clear_inuse();
		purify_clear_leaks();
	}
	
String id;

	// if (maxUsers && current_apps >= maxUsers->intval())
	if (current_apps >= max_apps)
	{
		logf->Log(_NOTICE, LOGAPPENV, "ResHandler: User limit of %d has been reached.",
			max_apps);

		/* Send quit event */
		Event *e = new Event(EVENT_Quit);
		if (maxUserMessage)
			e->AddDataPair(DFN_Text, maxUserMessage->Value());
		e->address1 = sessionid;
		OutEvent(e);

		return NULL;
	}
	
	if (startupapp)
		id = startupapp->Value();
	else
	{
		logf->Log(_ERROR, "ResHandler: no startup app defined.");
		return NULL;
	}
	

	logf->Log(_NOTICE, LOGAPPENV, "ResHandler: New Session: id \"%S\":", &sessionid);
	
	
	// Check to see if this name is already an "active application"
    //resource *r = activeApps.GetResource(id);

	// look for a CSAP object (application) in the active application table,
	// searching by instance id (sessionid) to see if it already exists.
	resource *r = FindCSAPObject(sessionid, &activeApps);
	if (r)
		logf->Log(_ERROR, LOGAPPENV, "Session \"%S\" already exists.", &sessionid);
	else
	{
		// find the "template" application object
		if (startup_app_name)
			r = gSysGlobals->GetResource(startup_app_name->Value());
		if (r && r->HierarchyContains(":csap_obj:App:"))
		{
#ifdef DEBUG
			logf->Log(_DEBUG, LOGAPPENV,
				"ResHandler: copying the template application object.");
#endif

			R_csap_obj *appcopy = ((R_csap_obj *) r)->Clone();	// copy it
			if (appcopy)
			{
				logf->Log(_DEBUG, LOGAPPENV, "Creating session \"%S\"", &id);
				
				// Set the instance identifier
				appcopy->SetInstanceID(sessionid);

				// install the copy to the list of active apps
				activeApps.Add(appcopy);	// Add --- allow duplicate names

				// Run its setup script
				((R_App *) appcopy)->Setup(in_ev);

				// Record the new app total
				current_apps++;
				total_apps++;

				// record concurrency stats.
				if (current_apps > max_concurrent)
					max_concurrent = current_apps;

				logf->Log(_NOTICE, LOGAPPENV, "Active Sessions: %d now, %d top, %d total",
					current_apps, max_concurrent, total_apps);


				if (purify_is_running())
				{
					// print message about the amount of new memory allocated
					purify_new_inuse();
				}
				return appcopy;
			}
		}
		else
			logf->Log(_ERROR, LOGAPPENV, "ResHandler::CreateSession: Cannot find template object.");
	}
	
	/* maybe do other stuff here */
	return NULL;
}

// QuitSession
// kills a session
void dResHandler::QuitSession(String id)
{
	if (purify_is_running())
	{
		purify_printf("QuitSession for %s..", id.chars());
		purify_new_inuse();
	}

	logf->Log(_DEBUG, LOGAPPENV, "ResHandler: QuitSession for %S.", &id);

	// Remove the App from the active list
#ifdef DEBUG	
	cout << "Active App table BEFORE delete:\n";
	printordered(activeApps.GetList());
	cout << "----------(end of active app table)--\n";
#endif
	resource *theobj = NULL, *r = NULL;
	R_App *theapp = NULL;
	
	r = FindCSAPObject(id, &activeApps);
	if (!r || !r->HierarchyContains(":App:"))
	{
		logf->Log(_INFO, LOGAPPENV,
			"ResHandler: QuitSession for %S: not found!", &id);
		return;
	}

	theapp = (R_App *) r;
	// Don't want to accidentally do all this twice!
	if (theapp->IsQuitting() || theapp == deleteable)
		return;

	theapp->IsQuitting() = 1;

	// end timeout processing (not really necessary here
	//  because it is handled in the destructor below)
	theapp->ForgetTimeout();

	// delete from list of active apps
	theobj = ResListDelete(activeApps.GetList(), RLD_POINTER, r);


#ifdef SHOW_FORMS
	cout << "Active App table AFTER delete:\n";
	printordered(activeApps.GetList());
	cout << "----------(end of active app table)--\n";
	cout.flush();
#endif

	if (theobj && theobj->HierarchyContains(":App:"))
	{
		if (((R_App *) theobj)->OpenWindows() <= 0)
			logf->Log(_DEBUG, LOGAPPENV,
				"dResHandler: QuitSession %S: no open windows.", &id);

		current_apps--;
		// delete (R_App *)theobj;	// no virtual destructor in resource yet
		deleteable = (R_csap_obj *)theobj;
	}
	else
		logf->Log(_INFO, LOGAPPENV,
			"ResHandler: QuitSession for %S: not found!", &id);

	//************************
	//* Logging & Statistics *
	//************************

	logf->Log(_NOTICE, LOGAPPENV, "Active Sessions: %d now, %d top, %d total",
		current_apps, max_concurrent, total_apps);

	struct rusage rut;
	getrusage(RUSAGE_SELF, &rut);
	logf->Log(_NOTICE, LOGAPPENV, "   Process RSS: %d, BRK: %d",
		(int) rut.ru_idrss, (int) sbrk(0));

	/*
	if (purify_is_running())
		purify_new_leaks();
	*/
		
}

// DeleteOutgoingEvents
// destroys all outgoing event objects.
void dResHandler::DeleteOutgoingEvents(void)
{
	if (outgoing_head)
	{
		outgoing_head->kill();	// kills the whole chain (except the first)
		delete outgoing_head;	// kill the first one
	}
	outgoing_head = outgoing_tail = NULL;
}

// GetOutgoingEvents
//   Returns the outgoing event list, and forgets about them.
Event *dResHandler::GetOutgoingEvents(void)
{
	Event *e = outgoing_head;
	outgoing_head = outgoing_tail = NULL;
	return e;
}

// Send Broadcast
// alert to all apps.
void dResHandler::SendBroadcast(String m)
{
	SLList<resource *> apps = activeApps.GetList();
	Pix temp = apps.first();
	resource *r = NULL;
	for(; temp; apps.next(temp))
	{
		r = apps(temp);
		if (r && r->HierarchyContains(":App:"))
		{
			((R_App *) r)->SendSimpleEvent("Alert", m, "RSL");
		}
	}
}

// PrintActiveInfo
// Log accumulated statistics.
// - Active sessions, total sessions.
// - Give information about each active session if the argument `detail'
//   is non-zero. 
void dResHandler::PrintActiveInfo(int detail)
{
	logf->Log(_INFO, LOGAPPENV, "ResHandler: %d active sessions of %d total.",
		current_apps, total_apps);

	if (detail)
	{
		SLList<resource *>& alist=activeApps.GetList();
		Pix temp=alist.first();
		resource *r=NULL;
		String nm, id;
		int ow = 0;

		for(;temp; alist.next(temp))
		{
			r = alist(temp);
			if (r && r->HierarchyContains(":csap_obj:App:"))
			{
				//	nm = r->Name();
				id = ((R_csap_obj *) r)->InstanceID();
				ow = ((R_App *) r)->OpenWindows();

				logf->Log(_INFO, LOGAPPENV, "   ID %S, %d windows", &id, ow);
			}
		}
	}
}

// IsUnique
// arg list must be R_String * (name, value)
// Checks each App ( by asking dResHandler
int dResHandler::IsUnique(SLList<resource *>& args)
{
	if (args.length() != 2)
	{
		logf->Log(_ERROR, LOGAPPENV, "ResHandler: IsUnique requires 2 arguments.");
		return -1;	// error: requires 2 arguments
	}

	resource *nm=NULL, *val=NULL;
	Pix temp = args.first();
	nm = args(temp);
	args.next(temp);
	val = args(temp);

	if (!nm || !val)
	{
		logf->Log(_ERROR, LOGAPPENV, "ResHandler: IsUnique found a NULL argument.");
		return -1;	// error: null pointers
	}

	String snm=nm->Value(), sval=val->Value();
	if (snm == "")
	{
		logf->Log(_ERROR, LOGAPPENV, "ResHandler: IsUnique: blank 'name' argument not allowed");
		return -1;	// error: blank name
	}

	// Search the local table of each App for a match
	// (local is relative - it is globa across each csap resource -
	//  such as a Form - that is owned by the App)

	resource *r=NULL, *r2=NULL;
	SLList<resource *>& apps = activeApps.GetList();
	Pix temp2;
	for(temp = apps.first(); temp; apps.next(temp))
	{
		r = apps(temp);
		if (r && r->ClassName() == "App")
		{
			logf->Log(_DEBUG, LOGAPPENV, "ResHandler: Searching App ID \"%S\" for \"%S\"=\"%S\"",
				&(((R_csap_obj *) r)->InstanceID()), &snm, &sval);

			// Search the local table (which is global across each
			// csap resource owned by the App)

			SLList<resource *>& appglobals = (((R_App *) r)->Locals())->GetList();
			for(temp2=appglobals.first(); temp2; appglobals.next(temp2))
			{
				r2 = appglobals(temp2);
				if (!r2) continue;
				if (r2->Name() == snm && r2->Value() == sval)
				{
					logf->Log(_DEBUG, LOGAPPENV, "ResHandler: %S=\"%S\" NOT unique in App local tables",
						&snm, &sval);
					return 0;	// notunique
				}
			}
		}
	}

	logf->Log(_DEBUG, LOGAPPENV, "ResHandler: %S=\"%S\" is unique in App local tables",
		&snm, &sval);
	return 1;	// unique
}


// registerTimeout
// tell the timeout manager about me
void dResHandler::registerTimeout(int secs)
{
	timeout_interval = secs;
	if (!to_node)
		to_node = to_mgr.AddObject((resource *) this);

	if (to_node)
		to_node->max_idle = secs;
	else
		logf->Log(_ERROR, LOGAPPENV,
		"ResHandler: Timeout manager returned error: Unable to log periodic stats");
}

// forgetTimeout
// tell the timeout manager to forget about me
void dResHandler::forgetTimeout(void)
{
	if (to_node)
	{
		to_mgr.BeGone(to_node);
		to_node = NULL;
	}
}


// _LogPeriodicInfo
// PRIVATE method, to be called at the close of each logging period.
void dResHandler::_LogPeriodic(void)
{
	
	LogPeriodicInfo();
	
	/* reset the timeout interval value */
	if (to_node)
		to_mgr.Touch(to_node);
		
	doneLastPeriod += doneThisPeriod;	// accumulate previous totals
}

// LogPeriodicInfo
// Public method, ok to be called at any time (within a logging period)
// if desired.
void dResHandler::LogPeriodicInfo(void)
{
	// compute the number of completed sessions since the start
	// of this period
	// (current_apps are included in total_apps)
	doneThisPeriod = total_apps - current_apps - doneLastPeriod;

	// adjust for starting boundary condition
	if (doneThisPeriod < 0)
		doneThisPeriod = 0;

	float fMin;
	fMin = (float) timeout_interval / 60;
	char sMin[20];
	sprintf(sMin, "%.1f", fMin);
	logf->Log(_NOTICE, LOGAPPENV,
		"PERIODIC: %d Completed in %s min; %d now, %d top, %d total",
		doneThisPeriod, sMin, current_apps, max_concurrent, total_apps);

//	/* compute a sessions-per-minute period average */
//	float fPerMin;
//	fPerMin = (float) doneThisPeriod / ((float) to_node->max_idle / 60);
//	char sPerMin[20];
//	sprintf(sPerMin, "%.1f", fPerMin);
//	logf->Log(_NOTICE, LOGAPPENV, "PERIODIC: %d Completed sessions this period, %s/min",
//		doneThisPeriod, sPerMin);


}

/* 
 * 
 * 
 *  resource	virtuals
 * 
 * 
 */

/***********************************
*execute						   *
*	Knows:						   *
*		loginfo	(Boolean detail)   *
*		timeout	(Boolean detail)   *
*			-- logs	active info.   *
***********************************/
resource *dResHandler::execute(String& method, SLList<resource *>& args)
{
resource *f=NULL;
	if (args.first() != NULL)
		f = args.front();

String m = downcase(method);

	if (m == "timeout")
		_LogPeriodic();	// periodic
	else if (m == "loginfo")
		LogPeriodicInfo();	// non-periodic
	else if (m == "registertimeout" && f && f->ClassName() == "Integer")
		registerTimeout(((R_Integer *) f)->intval());
	else if (m == "forgettimeout")
		forgetTimeout();
}

// Create -- cannot create new ones for RSL at this time.
resource *dResHandler::Create(String &nm, resource*& table)
{
	table = NULL;
	return this;
}

void dResHandler:: print(void)
{
	
}

void dResHandler:: print(ostream& out)
{
	
}
