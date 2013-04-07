// dResHandler.h
//
// PURPOSE: attempt 2 at a generic resource handler class
// that deals with applications and resources. Right now this
// class is mainly for testing the app and form classes
// and will be fleshed out later or removed. (11/14/95)
//
// PROGRAMMING NOTES: maybe this would be a subclass of some
// other resource handler class, or maybe this is the superclass
// of others. The question is which is more generic. Can this
// approach - using resource "template" objects and Event
// routing - be general enough (use this approach in the HTTP
// server?)
//
// Also, could this be a subclass of R_csap_obj? is that
// useful at all? It would give it rsl scriptability - rsl
// event handlers, and event handling the same way as
// other csap objects... at least...
//
//
// HISTORY: 11/14/95
//
// $Id: dResHandler.h,v 1.3 1996/10/12 17:30:21 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.
#ifndef dRES_HANDLER_H
#define dRES_HANDLER_H

#include "resource.h"
#include "restable.h"
#include "Event.h"
#include "R_csap_obj.h"
#include "timeout_manager.h"

class dResHandler : public resource  {
public:
	dResHandler(void) { Init(); }
	dResHandler(R_String *start) { Init(); startupapp = start; }
	
	// resource virtuals
	resource *execute(String& method, SLList<resource *>& args);
	resource *Create(String &nm, resource*& table);
	inline String Value(void) { return String("dResHandler"); }
	void print(void);
	void print(ostream& out);

	// native
	virtual void HandleEvent(Event *e);
	virtual void OutEvent(Event *e);

	virtual R_csap_obj *CreateSession(Event *e);
	void QuitSession(String id);
	Event *GetOutgoingEvents(void);
	void DeleteOutgoingEvents(void);
	
	void PrintActiveInfo(int detail=0);
	void LogPeriodicInfo(void);
	void SendBroadcast(String m);
	
	resource* GetApp(String str)	// Instance ID
		{ return FindCSAPObject(str, &activeApps); }

	int CurrentApps(void) { return current_apps; }	//	activeApps.length(); }
	int TotalApps(void) { return total_apps; }
	int& MaxApps(void) { return max_apps; }
	int MaxConcurrent(void) { return max_concurrent; }

	virtual int IsUnique(SLList<resource *>& args);
	
	void registerTimeout(int secs);
	void forgetTimeout(void);
	
protected:
	restable activeApps;
	Event *outgoing_head, *outgoing_tail;
	R_String *startupapp;	/* obsolete: go away soon */
	R_csap_obj *deleteable;	// app to delete
	int current_apps, max_apps, total_apps, max_concurrent;
	int doneThisPeriod, doneLastPeriod;
	int timeout_interval;
	timeout_node *to_node;

	void Init(void);
	void _LogPeriodic(void);
};

#endif
