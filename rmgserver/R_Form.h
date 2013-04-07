/*
	$Id: R_Form.h,v 1.6 1996/10/12 17:30:06 holtrf Exp $
*/

/*****************************************************************************

 R_Form.h
 
 A "Form" resource

 Copyright (c) 1995, 1996 by Destiny Software Corporation

 ****************************************************************************/



/*****************************************************************************
 <<Begin Resource Documentation>>


RESOURCE NAME: Form

RELATED RESOURCES: csap_obj, App

DESCRIPTION: 

PUBLIC MEMBER FUNCTIONS:
sendlist
sendiconlist
displayitem
clearlistbox
enable
disable
checkbox
useappevents
title
open
nextform
close
exe
error


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
// $Id: R_Form.h,v 1.6 1996/10/12 17:30:06 holtrf Exp $
//
// PURPOSE: General Form resource
//
// PROGRAMMING NOTES:
//
// HISTORY: Ugg
//
// Copyright 1995, 1999 by Destiny Software Corporation.

#ifndef _FORM_H_
#define _FORM_H_

#include "resource.h"
#include "restable.h"
#include <SLList.h>
#include <String.h>
#include "Event.h"
#include "R_String.h"
#include "R_csap_obj.h"
#include "stream.h"

class R_Form : public R_csap_obj {
public:
	R_Form(void) { Init(); }
	R_Form(String nm) { Init(); name = nm;}
	R_Form(char *nm) { Init(); name = nm;}
	virtual ~R_Form(void);
	
// resource virtual functions
	resource *Create(String &nm, resource*& table);
	resource *execute(String& method, SLList<resource *>& args);
	int LogicalValue(void);
	inline String Value(void)
		{
			return String(form("%s %s:%s", name.chars(),
			system_id?system_id->chars():"*", instance_id.chars()));
		}
	void print(void) { cout << name; }

// csap_obj virtual functions
	R_csap_obj *Clone(void);
	//	void Setup(void);
	void Setup(Event *e=NULL);
	int HandleEvent(Event *e, restable *obj_globals=NULL);
	void OutEvent(Event *e);
	void Open(void);
	void Close(void);

// R_Form functions
	void SetSysID(SLList<resource *>& args);
	void SendList(SLList<resource *>& args, int);
	void ClearListBox(SLList<resource *>& args);
	void DisplayItem(SLList<resource *>& args);
	void Enable(resource *field, char which);
	void Checkbox(SLList<resource *>& args);

	Event *MakeOpenEvent(void);
	void Open(SLList<resource *>& args);
	void NextForm(SLList<resource *>& args);
	
	inline String &Title(void) { return title; }

private:
	enum { closed, closing, open, opening };
	void Init(void);
	String title;
	int useappevents;
	int state;
};


#endif