//
// NAME:	Event.h
//
// PURPOSE:	Definitions for Events for the Destiny Server
//		Defines the META actions that can be mapped to or
//		from for any protocol
//
// USAGE:	include this file in any protocol or event related
//		program
//
// PROGRAMMING NOTES:	data members are not allowed to be set
// 			except when an object is constructed.
//
// HISTORY:	Written 11/06/1995 by Les Shuda (Wrong! this one was Russell)
//
// LAST REVISION:
//  0.  Russell- original working version
//	1.  12/15/1995   by LS Weber	Reason:  Added std definitions
//	2.  12/21/1995   by LS Weber	Reason:  Revised Definitions
//	3.  12/26/1995   by LS Weber	Reason:  Changed to const String
//	4.  01/06/1996   by LS Weber	Reason:  Added SelectIcon
//	5.  01/12/1996   by LS Weber	Reason:  Revised DFN per spec
//
// $Id: Event.h,v 1.13 1996/03/06 21:38:06 weber Exp $
//
// Copyright 1995 by Destiny Software Corporation.


#ifndef _EVENT_H_
#define _EVENT_H_

#include	<String.h>		//Strings
#include	<SLList.h>		//Single Linked List(s) 
#include	<DLList.h>		//Doubly Linked List(s)
#include	<resource.h>		//RSL Resources
#include	"R_String.h"
#include	"SLList_res_util.h"


//------------------------------------------------------------------------------
//  These are the Destiny Server EVENT Names
//------------------------------------------------------------------------------
const String	EVENT_Action		= "Action";
const String	EVENT_Alert		= "Alert";
const String	EVENT_CheckBox		= "CheckBox";
const String	EVENT_ClearListBox	= "ClearListBox";
const String	EVENT_CloseWindow	= "CloseWindow";
const String	EVENT_CreateSession	= "CreateSession";
const String	EVENT_DisableField	= "DisableField";
const String	EVENT_DisableImmediate	= "DisableImmediate";
const String	EVENT_DisplayItem	= "DisplayItem";
const String	EVENT_EnableField	= "EnableField";
const String	EVENT_EnableImmediate	= "EnableImmediate";
const String	EVENT_Error		= "Error";
const String	EVENT_GetUserData	= "GetUserData";
const String	EVENT_Input		= "Input";
const String	EVENT_Open		= "Open";
const String	EVENT_Quit		= "Quit";
const String	EVENT_Radio		= "Radio";
const String	EVENT_ReOpen		= "ReOpen";
const String	EVENT_SelectItem	= "SelectItem";
const String	EVENT_SendList		= "SendList";
const String	EVENT_SendIconList	= "SendIconList";
const String	EVENT_UserData		= "UserData";
const String	EVENT_WaitCursor	= "WaitCursor";


//------------------------------------------------------------------------------
//	These are the Destiny Server EVENT DataFieldNames
//------------------------------------------------------------------------------

const String	DFN_AppendText		= "AppendText";
const String	DFN_Button		= "Button";
const String	DFN_Confirm		= "Confirm";
const String	DFN_Default		= "Default";
const String	DFN_Entry		= "Entry";
const String	DFN_ItemId		= "ItemId";
const String	DFN_GlobalGraphic	= "GlobalGraphic";
const String	DFN_Graphic		= "Graphic";
const String	DFN_GroupId		= "GroupId";
const String	DFN_Machine		= "Machine";
const String	DFN_MessageText		= "MessageText";
const String	DFN_Name		= "Name";
const String	DFN_NumItems		= "NumItems";
const String	DFN_Paying		= "Paying";
const String	DFN_Service		= "Service";
const String	DFN_Speed		= "Speed";
const String	DFN_Text		= "Text";
const String	DFN_TextId		= "TextId";
const String	DFN_Title		= "Title";
const String	DFN_Version		= "Version";

//------------------------------------------------------------------------------
//	The   "String"  Booleans
//------------------------------------------------------------------------------

const String	DFN_False		= "False";
const String	DFN_True		= "True";


//------------------------------------------------------------------------------
//	The D E S T I N Y   E V E N T     S T R U C T U R E
//------------------------------------------------------------------------------

struct Event {
	Event *next;
	String type,	// button press, etc.
		address1, address2, address3,	// address 1, address 2
		item,	// ie button number, new form id, etc.
		field,
		source;	// who sent the event - protocol, form, etc.
	int eaten;	// this event has been processed or not

	SLList<resource *> DataPairs;	// R_String

	Event(const char *t) : type(t) { Init(); }
	Event(String t) : type(t) { Init(); }
	
	Event(String t, String a1, String a2, String i, String src="")
		: type(t), address1(a1), address2(a2), item(i), source(src)
		{ Init(); }

	Event(const char *t, const char *a1, const char *a2,
		const char *i, const char *src="")
		: type(t), address1(a1), address2(a2), item(i), source(src)
		{ Init(); }

	Event(const char *t, const char *a1, const char *a2)	// no item
		: type(t), address1(a1), address2(a2)
		{ Init(); }

	~Event();
	
	void Init(void);
	void kill(void);
	
	friend inline ostream& operator<<(ostream& out, Event &e);
	
	void AddDataPair(String nm, String val);
	void AppendDataPair(String nm, String val);

	// member access. Use these like
	//     theevent->Type() = "Action"; 	// set
	//     cout << theevent->Type();	// get
	inline Event*& Next(void) { return next; }
	inline String& Type(void) { return type; }
	inline String& Address1(void) { return address1; }
	inline String& Address2(void) { return address2; }
	inline String& Address3(void) { return address3; }
	inline String& Item(void) { return item; }
	inline String& Field(void) { return field; }
	inline String& Source(void) { return source; }
	inline int& Eaten(void) { return eaten; }
		
	void print(int index=0);
};

// print an event
inline ostream& operator<<(ostream& out, Event &e)
{
	out << "Event type " << e.type << " for " << e.address1
		<< " . " << e.address2;
//	if (e.DataPairs.length() > 0)
//	{
//		out << " with the following data pairs:\n";
//		resource::print(e.DataPairs);
//		out << '\n';
//	}
}


#endif
