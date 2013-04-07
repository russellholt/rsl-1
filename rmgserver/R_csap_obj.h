/*
	$Id: R_csap_obj.h,v 1.6 1996/10/12 17:30:14 holtrf Exp $
*/

/*****************************************************************************

 R_csap_obj.h
 
 A distributed object

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: csap_obj

RELATED RESOURCES: App, Form

DESCRIPTION: User interface abstract class, not directly accessible.
	Provides a common base and interface for the properties and attributes
	that all "csap_obj" resources, such as App and Form provide.

PUBLIC MEMBER FUNCTIONS:

	AddEvent(String event: eventname, [String item: itemID],
		[String field: fieldID], script )

	AddAutoEvent (String event: eventname, [String item: itemID],
		[String field: fieldID],
		[ String method, String objectname, [ <arguments> ] ] )

		Automatic Event Handler: when the event {eventname, itemID, fieldID} is
		matched, the object objectname is found, and method is applied
		to it with any arguments that follow. For example,
		
			f210.AddAutoEvent ( event: "action", item: 1003,
				"NextForm", "f212", "This is the window title")
		
		would send the NextForm message to f212. The above is equivalent to:
		
			f210.AddEvent(event: "action", item 1003, f210.DoNextThing);
			f210::DoNextThing
			{
				HomeBanking.InitObject("f212");
				f212.Title("This is the window title");
				f212.Open();
				f210.Close();
			}


 <<End Resource Documentation>>
 ****************************************************************************/

 
/*****************************************************************************
 <<Begin Class Documentation>>


 CLASS NAME: R_csap_obj

 DERIVED FROM: resource

 RELATED CLASSES: R_Form, R_App

 DESCRIPTION: User interface abstract class, not directly accessible.
	Provides a common base and interface for the properties and attributes
	that all "csap_obj" resources, such as App and Form provide.
 

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
// $Id: R_csap_obj.h,v 1.6 1996/10/12 17:30:14 holtrf Exp $
//
// PURPOSE: csap object base class.
//
// PROGRAMMING NOTES: although a resource, this class overrides
// no resource methods. Subclasses should do so, in order to
// be used and specified in RSL.
//
// HISTORY: 11/14/95 - created, & most of these routines were moved here from
// old "form" class.
// 12/18/95 RFH- added "field" to event mapping & add/match
//
// Copyright 1995, 1996 by Destiny Software Corporation.

#ifndef _RCSAP_OBJ_H_
#define _RCSAP_OBJ_H_
#include "resource.h"
#include "restable.h"
#include "R_String.h"
#include "Event.h"

resource *FindCSAPObject(String& obj, restable *where, resource *ignore=NULL);
const Regex rmatch = "[0-9a-zA-Z_]*";	// almost an identifier

class R_csap_obj : public resource {
public:
	enum emtype { emap_script, emap_auto };

	struct emap {
		emtype type;
		String evname, item, field;
		restable *scr;
		emap(void) { scr = NULL; }
	};

	struct emap_execute : public emap {
		String method;
		SLList<resource *> args;
		emap_execute(void) { }
	};

	R_csap_obj(void) { Init(); }
	virtual ~R_csap_obj(void);

	void Add(SLList<resource *>& args);
	void AddEvent(String& nm, String& it, String& fi, restable* sc);
	void AddAuto(SLList<resource *>& args);
	void AddAutoEvent(String& nm, String& it, String& fi,
		String& meth, SLList<resource *>& autoargs);
	emap *R_csap_obj::MapEvent(String& event, String& item, String& fi);
	restable* GetScript(String& event, String& item, String& field);
	inline restable* Locals(void) { return &locals; }
	
	//	virtual void Setup(void) { /* subclass would allocate 'locals' */ }
	virtual void Setup(Event *e=NULL) { /* subclass would allocate 'locals' */ }
	void ExecuteSetup(restable *appobjects);
	void SetSetup(void);
	restable* GetSetup(void) { return setup; };
	void SetDataPairVars(SLList<resource *> &datapairs);

	virtual int HandleEvent(Event *e, restable *obj_globals=NULL);
	int RunEventHandler(Event *e, String caller, restable *obj_globals=NULL);
	virtual R_csap_obj *Clone(void) { return new R_csap_obj(*this); }
	virtual void OutEvent(Event *e) { if (owner) owner->OutEvent(e); }
	void SendSimpleEvent(const char *evtype, String text, const char *src);

	// object identifiers
	String InstanceID(void) { return instance_id.Value(); }
	void SetInstanceID(String id) { instance_id = id; }
	inline int CompareInstanceID(String& id)
		{  return (id == instance_id.Value()); }
	String SystemID(void)
		{ return (system_id? system_id->Value() : String("")); }
	void SetSystemID(String id)
		{ if (system_id) (*system_id) = id; /* else... would be bad */ }
	inline int CompareSystemID(String& id)
		{  return (system_id? (id == system_id->Value()) : 0); }
	R_csap_obj *Owner(void) { return owner; }
	void SetOwner(R_csap_obj *ow) { owner = ow; }

	virtual void Open(void);
	virtual void Close(void);
	
	inline int& IsOpen(void) { return isOpen; }
	inline int& IsSetup(void) { return isSetup; }
	inline int& IsQuitting(void) { return quitting; }
	
protected:
	SLList<emap *> *evtmap;
	restable locals, *setup;
	R_String *system_id, instance_id;
	R_csap_obj *owner;
	Event *current_event;
	int isOpen, isSetup, quitting;

private:
	void Init(void);
};


#endif
