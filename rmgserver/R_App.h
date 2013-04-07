/*
	$Id: R_App.h,v 1.5 1996/10/12 17:29:59 holtrf Exp $
*/

/*****************************************************************************

 R_App.h
 
 Application (user session)

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: App

RELATED RESOURCES: Form

DESCRIPTION: This is the application. Manages application-level resources like
	forms and windows. Routes incoming events to the resources it owns.

PUBLIC MEMBER FUNCTIONS:

	AddAutoEvent
		See "csap_obj".

	AddEvent
		See "csap_obj".

	Alert(String message)
		Display a normal alert dialog box with a message.

	Close(String res_name)
		Closes and destroys the named resource.

	CloseAll(String except_name)
		Closes all open windows except for the named argument.

	Error(String message)
		Display an Error alert dialog box with a message.

	Exit
		Sends an outgoing "quit" event. The system will respond with an incoming
		quit event, which should be handled by a "Quit" event handler. In response
		to this incoming quit event, after the application has done its final
		cleanup procedures, call the Exit method again with the argument "noeventsend"
		which does the final cleanup and exit.

	ForgetTimeout
		Tell the timeout manager that this application is no longer interested
		in activity timeout events.

	InitObject (String res_name)
		Creates an instance of the named resource and executes the "Setup" method
		but does not send the Open event.

	Open (String object_name, [ <variables to set> ] )
		Creates an instance of the named object (See InitObject)
		and sends an Open event. If other arguments are present,
		they are copied to variables local to the new object.

			HomeBanking.Open("f210");

		is effictively equivalent to:

			HomeBanking.InitObject("f210");
			f210.Open();

	OpenWindows
		Returns an Integer idicating the number of windows that are currently open.

	RegisterTimeout(Integer seconds)
		Tell the timeout manager that this application wishes to be notified via
		a "Timeout" event when there has been no activity for the duration `seconds'.

	Remove(String res_name)
		Destroys the object (does not send a close event -- see Form.Close())

	SetSystemID(String id)
		Set the reference ID for this resource which the foreign system will
		use to refer to it (eg, for AOL this is the "form number").

	setdata
	timeout
	userdata
	instanceseed
	openwindows
	getuserdata


 <<End Resource Documentation>>
 ****************************************************************************/

 
/*****************************************************************************
 <<Begin Class Documentation>>


 CLASS NAME:

 DERIVED FROM:

 RELATED CLASSES:

 DESCRIPTION:
 

 PUBLIC DATA MEMBERS:

	<name>
		documentation including data definition, valid values


 PROTECTED DATA MEMBERS:

	<name>
		documentation


 PRIVATE DATA MEMBERS:

	<name>
		documentation


 CONSTRUCTORS AND DESTRUCTORS

	<function prototype>

		documentation

 OPERATORS:
	
	<operator function prototype>

		documentation

 PUBLIC MEMBER FUNCTIONS:

	<function prototype>
		documentation including input/output explanation

 PROTECTED MEMBER FUNCTIONS:

	<function prototype>
		documentation including input/output explanation

 SEE ALSO: resource.h
 

 <<End Class Documentation>>
 ****************************************************************************/
// $Id: R_App.h,v 1.5 1996/10/12 17:29:59 holtrf Exp $
//
// PURPOSE: A generic resource application class.
//    - a csap class
//
// PROGRAMMING NOTES:
//    - can have an event map via csap_obj interface (but would have
//      to be implemented in EventCentral
//
// HISTORY:
//    .. created week of 11/6
//    - 11/18 consolidate DApp and R_App into an R_csap_obj subclass
//
// Copyright 1995, 1996 by Destiny Software Corporation.
#ifndef _R_App_h_
#define _R_App_h_

#include <String.h>
#include "restable.h"
// #include "DApp.h"
#include "R_String.h"
#include "R_csap_obj.h"
#include "Event.h"
#include "timeout_manager.h"

class dResHandler;

class R_App : public R_csap_obj {
public:
	R_App(void) { Init(); }
	R_App(String &nm) { name = nm; Init(); }
	R_App(char *nm) { name = nm; Init(); }
	virtual ~R_App(void);
	

// resource virtual functions
	resource *execute(String& method, SLList<resource *>& args);
	resource *Create(String &nm, resource*& table);
	inline String Value(void) { return name+ String(" ") + instance_id.chars(); }
	void print(void);
	
// csap_obj virtual functions
	int HandleEvent(Event *e, restable *objs);
	int Handle1Event(Event *e, restable *objs);
	void Setup(Event *e=NULL);
	R_csap_obj *Clone(void);
	void OutEvent(Event *e);
	
// R_App specific functions
	int EventCentral(String& evname, SLList<resource *>& args,
		String source, resource*& result);
	resource *Open(SLList<resource *>& args);
	resource *InitObject(String object_name);
	void Quit(String text);
	void RemoveByName(resource *resname);
	void RemoveByPointer(resource *respointer);
	void Remove(String what);
	void Close(resource *r, String source);
	void CloseAll(resource *& fa);
	String NextObjectID(void);
	inline int OpenWindows(void) { return open_windows; }
	int QuitSession(int sendevent, String source);

	void CleanUp(void);

	void RegisterTimeout(resource *arg);
	void ForgetTimeout(void);
	
	static dResHandler *rh_owner;
	
private:
	unsigned long next_object_id;
	void Init(void);
	int open_windows;	// number of open windows
	void innerRemove(resource *theobj);
	void EventHandled(void);
	
protected:
	timeout_node *to_node;
	SLList<resource *> GoingAway;
};

#endif