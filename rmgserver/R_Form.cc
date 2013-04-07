// $Id: R_Form.cc,v 1.10 1996/10/12 17:30:03 holtrf Exp $
//
// PURPOSE:
//
// PROGRAMMING NOTES:
//
// HISTORY: 
//
// Copyright 1995 by Destiny Software Corporation.

#include "R_Form.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "restable.h"
#include "Event.h"
#include "slog.h"
#include "SLList_res_util.h"
#include "destiny.h"
#include "R_App.h"
#include "R_ListBox.h"

#include "purify.h"

extern restable References, *gSysGlobals;
extern slog mylog;
extern int Testing;	// server command line -d
extern void DestroyResList(resource *owner, SLList<resource *>& thelist);
extern R_Boolean *debug_titles;

const char * const rcsID = "$Id: R_Form.cc,v 1.10 1996/10/12 17:30:03 holtrf Exp $";

void R_Form::Init(void)
{
	class_name = "Form";
	hierarchy += "Form:";	// becoming ":csap_obj:Form:"
	useappevents = 0;	// false
	/* system_id was installed in R_csap_obj */
	
	state = closed;
}

resource *R_Form::Create(String &nm, resource*& table)
{
	table = NULL;
	R_csap_obj *r = new R_Form(nm);	// may be Clone(*this);
	if (r)
	{
		// find the "Setup" script, as in (in rsl) "f4a::Setup"
		r->SetSetup();	// R_csap_obj::SetSetup()	
		table = r->Locals();	// where to put resource declared in rsl Create
		((restable *)table)->install_resource(r);	// add pointer to itself
	}
	return r;
}

R_csap_obj *R_Form::Clone(void)
{
	return new R_Form(*this);
}

// Destructor
R_Form::~R_Form(void)
{
	if (purify_is_running())
		purify_printf("~R_Form() for id %s", name.chars());

#ifdef SHOW_FORMS
	cout << "----------------BEGIN ~R_Form() for " << name << " instance " << instance_id.chars() << '\n';
	cout << "-- the following resources will be DELETED:\n";
	printordered(locals.GetList());

	cout << "-- remove SystemID and \"" << name << "\" from the local table..\n";
#endif

	ResListDelete(locals.GetList(), RLD_NAME, "SystemID");
	ResListDelete(locals.GetList(), RLD_NAME, "InstanceID");
	ResListDelete(locals.GetList(), RLD_NAME, name.chars());

#ifdef SHOW_FORMS
	cout << "-- Locals is now:\n";
	printordered(locals.GetList());
	cout << "-- locals.FreeAll()..\n";
#endif

	locals.FreeAll();
	//	DestroyResList(this, locals.GetList());

#ifdef SHOW_FORMS
	cout << "-- local table is now:\n";
	printordered(locals.GetList());

	cout << "----------------END ~R_Form()\n";
#endif

}


// Setup
// executed for each clone
// e is unused.
void R_Form::Setup(Event *e)
{
#ifdef SHOW_FORMS
	cout << "R_Form::Setup() --- adding instance ID to locals...\n";
#endif
	
	// Make the instance id available to rsl scripts
	locals.install_resource(&instance_id);
	
#ifdef SHOW_FORMS
	cout << "R_Form::Setup() --- deleting original object pointer from locals...\n";
#endif
	// delete the pointer to the original object (we're a clone remember)
	locals.Delete(name, String("name"));	// delete by name
	
//#ifdef SHOW_FORMS
//	cout << "R_Form::Setup() --- adding a pointer to this to locals...\n";
//#endif

	// ..and insert a pointer to the clone object
	
/* moved to App.Open etc : */
	
//	locals.Insert(this);	// insert at head.
}


// HandleEvent
// 
int R_Form::HandleEvent(Event *e, restable *obj_globals)
{
int h = 0;
	if (!e)
		return 1;	// yes it was handled - just ignored

	// bump state -- but only for incoming event chains from the res handler
	// (ie, only here in HandleEvent)
	if (state == closing)
		state = closed;
	else
	if (state == opening)
		state = open;

	/* Special -- only process incoming close windows if we're open! */
	if (e->type == EVENT_CloseWindow && (state == closed || state == closing))
		return 1;	// yes it was handled - just ignored

	// Find a form event handler
	h = RunEventHandler(e, name, obj_globals);

	// Not a form event, so check app events (if applicable)
	if (!h && useappevents && owner)
	{
		logf->Log(_INFO, LOGAPPENV, "Form %S: %S/%S not a form event, checking App events..",
			&name, &(e->type), &(e->item));
#ifdef DEBUG
		cerr << "R_Form::HandleEvent- looking for owner events\n";
#endif
		h = owner->RunEventHandler(e, name, obj_globals);
	}
		
	return h;
}

resource *R_Form::execute(String& method, SLList<resource *>& args)
{

	String m = downcase(method);
	resource *f = NULL;
	if (args.first())
		f = args.front();
	
	if (m == "addevent")
	{	Add(args); return NULL; }
	if (m == "addautoevent")
	{	AddAuto(args); return NULL; }
	else
	if (m == "setformnumber" || m == "setsysid")
	{	SetSysID(args); return NULL; }
	else
	if (m == "sendlist")
	{
		SendList(args, 0);
		return NULL;
	}
	else
	if (m == "sendiconlist")
	{
		SendList(args, 1);
		return NULL;
	}
	else
	if (m == "setselection")
	{
		/* 
		 * SetSelection
		 *	   Accepts 2 String	arguments: field, item.
		 */
	
		Pix p=args.first();
		if (f && (args.next(p), p != NULL))
		{
			resource *g = args(p);

			Event *e = new Event(EVENT_SetSelection);
			e->field = f->Value();
			e->AddDataPair(DFN_Set, g->Value());
			Open();	// open this form if not already open(ing)
			OutEvent(e);
		}
		else
			cerr << "Form." << method << "() requires 2 arguments: field and item.\n";

		return NULL;
	}
	
	else
	if (m == "displayitem")
	{
		DisplayItem(args);
		return NULL;
	}
/*
	else
	if (m.contains("display"))
	{
		DisplayItem(args, m.after("display"));
		return NULL;
	}
*/
	else
	if (m == "clearlist" || m == "clearlistbox")
	{
		ClearListBox(args);
		return NULL;
	}
	else
	if (m == "enable" || m == "disable")
	{
		Enable(f, m[0]);
		return NULL;
	}
	else
	if (m == "checkbox")
	{
		Checkbox(args);
		return NULL;
	}
	else
	if (m == "useappevents")
	{
		return GetSet(useappevents, f);
	}
	else
	if (m == "title")
		return GetSet(title, f);
	else
	if (m == "open")
	{
		Open(args);
		return NULL;
	}
	else
	if (m == "nextform")
	{
		NextForm(args);
		return NULL;
	}
	else
	if (m == "close")
	{
		Close();
		return NULL;
	}
	else
	if (m == "exe")
	{
		int l = args.length();
		Pix temp = args.first();
		resource *execlist=NULL;

		if (temp)
			 execlist = args(temp);	// restable to execute is first arg.

		if (!execlist)
		{
#ifdef DEBUG
			cerr << "Form:exe -- no procedure/method to run..\n";
#endif
			logf->Log(_ERROR, LOGAPPENV, "%S.exe() -- no script to run.",
				&name);
			return NULL;
		}
#ifdef DEBUG
		else
			cerr << "form:exe -- attempt to run "
				<< execlist->ClassName() << "  \""
				<< execlist->Name() << "\"\n\n";
#endif

		args.next(temp);
		restable args_to_fn;
		if (temp)
		{
			args.remove_front();	// remove the restable; left with args.
			args_to_fn.GetList() = args;	// set
		}
		args_to_fn.SetName("Args");	// the argument table, just like in regular RSL.
		if (execlist && execlist->HierarchyContains("List"))
		{
			restable *the_argl = temp? &args_to_fn : (restable *) NULL;
			// need to give them access to our local variables.
			((restable *) execlist)->execute(the_argl, owner->Locals(), NULL, &locals);
		}
		else
		{
			logf->Log(_ERROR, LOGAPPENV, "%S.exe( ? ) -- no object/method/procedure given.",
				&name);
#ifdef DEBUG
			cerr << "form::exe -- Cannot run procedure...\n";
#endif
		}
	}
	else
	if (m == "error")
	{
		SendSimpleEvent("Error", f?f->Value():String(""), "RSL");	// type, text, source
		return NULL;
	}

	logf->Log(_INFO, LOGAPPENV, "Form: message \"%S\" unknown: %s",
		&m, "checking local restable.");
		
	// Ok, so the requested "method" is not specifically listed here.
		
	// Try looking in local table - for variable "requests" (cool, eh?)
		resource *rft = locals.GetResource(method);
		if (rft)
			return rft;
			
//	// Ok, that's not it. As a final resort, generate an outgoing event
//	// for whatever the method is.
//	Event *ne = new Event(
//		method,		// type
//		String(""),	// address1: set in out event
//		InstanceID(),	// address2: object instance id (eg form id)
//		SystemID(),	// item: object system id (eg form number)
//		String("RSL"));		// source: RSL
//	
//	if (ne)
//	{
//		cerr << "Form: Auto outgoing event for " << method << "\n";
//		ne->address3 = InstanceID();	// address3: <window id>
//		OutEvent(ne);	// send it
//	}

	return NULL;
}

// NextForm
// Close this resource, and open the named resource with the title.
// String resname, String title
// RFH Jan 26, 1996
void R_Form::NextForm(SLList<resource *>& args)
{
	if (!owner)	// can't propogate events!
	{
		logf->Log(_ERROR, LOGAPPENV,
			"%S.NextForm(): lost application context (can't send events)..",
			&name);
#ifdef DEBUG
		cerr << "R_Form::NextForm -- no owner (App) to propogate outgoing event!!\n";
#endif
		return;
	}

	
char order = 'c';	// close first by default
resource *r = NULL;
String nextformname, thetitle;

	// first arg is the name of the new form to open
	Pix temp = args.first();
	if (temp)
	{	nextformname = args(temp)?args(temp)->Value():String("");
		args.next(temp);
	}

	// other args are the title to set or the order, both are optional.
	while (temp)	// arg #2, #3 -- title or order
	{
		r = args(temp);
		if (r)
		{
			if (downcase(r->Name()) == "order")
				order = (r->Value())[0];	// either 'c' or 'o' hopefully
			else	// assume to be title
				thetitle = r->Value();
		}
		args.next(temp);
	}

	
	// Close this form: 1. Send close event 2. Tell application to remove me.
	if (order == 'c')
		Close();	// Send close event for this form first

	// "init" and send open event for specified form
	// Tell the owner (most likely an R_App) to InitObject with the
	// given args (it will only pay attention to arg #1, the name though)
//	resource *newobj = owner->execute(String("InitObject"), args);
	resource *newobj = ((R_App *) owner)->InitObject(nextformname);
	
	// Assign the title and send the open event.
	if (newobj)
	{
		logf->Log(_INFO, LOGAPPENV, "Form %S: NextForm %S", &name, &(newobj->Name()));
		if (newobj->HierarchyContains(":Form:"))
		{
//			// Get the title from second argument:
//			Pix temp = args.first();	// arg 1
//			if (temp)
//				args.next(temp);
//
//			resource *r=NULL;
//			if (temp)
//				r = args(temp);	// arg #2 -- title

//			if (r)
				((R_Form *) newobj)->Title() = thetitle;	// set the title

			// Open event moved after close
			// ((R_Form *) newobj)->Open();	// open it
		}
	}

	// Send Open event
	// if the order is 'c' (close first), then the close event for this already went out,
	// so now we send the open event for both `order' cases.
	if (newobj)
		((R_Form *) newobj)->Open();	// open it

	// now send close event if the Open was to go first.
	if (order == 'o')	
		Close();	// send close event for this one

	// Remove this Form now.
	if (owner->HierarchyContains(":App:"))	// make sure it's the right type
		((R_App *) owner)->RemoveByPointer(this);	// ->Remove(name);

}


// Close
// Generates outgoing close event to close the window (and stuff)
// this object remains in the Application local table.
// RFH
void R_Form::Close(void)
{
	if (!owner)
	{
		logf->Log(_ERROR, LOGAPPENV, "Form.Close in \"%S\": no App to send outevent!",
			&name);
		return;
	}
	
	// Can only close a form that is already open.
	if (state == closing || state == closed || !isOpen)
	{
		logf->Log(_INFO, LOGAPPENV, "Form::Close: cannot close unopened form \"%S\".",
			&name);
		return;
	}
	
	Event *ne = new Event("CloseWindow");
	if (ne)
	{
		ne->address3 = instance_id.Value(); // wid- what to close
		ne->source = "RSL";
		OutEvent(ne);
		state = closing;
		isOpen = 0;
	}
}

// MakeOpenEvent
// Open or ReOpen, depending on the value of isOpen
// Sets title from ::Title
// returns an Event*
// RFH
Event *R_Form::MakeOpenEvent()
{
	Event *ne = new Event(
	(isOpen? String("ReOpen") : String("Open")),		// type
	String(""),	// address1: set in out event
	InstanceID(),	// address2: object instance id (eg form id)
	SystemID(),	// item: object system id (eg form number)
	String("RSL"));		// source: RSL

	if (ne)
	{
		ne->address3 = InstanceID();	// address3: <window id>
		
		String ti;

		if (debug_titles && debug_titles->LogicalValue())
			ti = name + String(": ") + SystemID() + String(" ");
		ti += title;
		
		ne->AddDataPair("Title", (const char *) ti);
	}
	else
		logf->Log(_ERROR, LOGAPPENV, "Form: Unable to create Open event for %S!",
			&name);

	return ne;
}

// Simple Open
void R_Form::Open(void)
{
	if (state == opening)
	{
#ifdef DEBUG
		cout << "R_Form::Open --- " << InstanceID() << " is currently opening."
			<< " (cannot send another open event).\n";
#endif
		logf->Log(_DEBUG, LOGAPPENV, "%S[%S].Open(): already opening.",
			&name, &(InstanceID()));
		return;
	}
	
	Event *ne = MakeOpenEvent();

	state = opening;
	isOpen = 1;	// record state
	OutEvent(ne);
}

// Open
// Get title from argument list.
void R_Form::Open(SLList<resource *>& args)
{
	if (!owner)
	{
		logf->Log(_ERROR, LOGAPPENV, "%S.Open(): lost application context.", &name);
#ifdef DEBUG
		cerr << "R_Form::Open -- no owner (App) to propogate outgoing event!!\n";
#endif
		return;
	}
	
	// Set appropriate title
	if (args.length() > 0)
	{
		resource *f = args.front();

		if (f && f->ClassName() == "String");
			title = f->Value();
	}
	Open();	// call simple open

}


// Checkbox
// 
//void Checkbox(item#, group: groupValue, newValue)
//    item#: Integer representing the item to be checked.
//    groupValue: Optional integer representing the group that the check box belongs to.
//    newValue: Boolean containing the status of the checkbox.
void R_Form::Checkbox(SLList<resource *>& args)
{	
	if (args.length() < 1)
		return;

resource *r = NULL;
String item, newvalue, group;
int c = 0;
Pix temp = args.first();
String n;

	// This code is major ugg. The thing is we're mixing tagged and non-tagged
	// parameters. The optional parameter has a tag. It may be stuck anywhere
	// between any other parameters, but the order of the non-tagged parameters
	// relative to each other must be constant.
	for (c=0; temp; c++, args.next(temp))
	{
		r = args(temp);
		if (!r)
			continue;
		n = downcase(r->Name());

		if (n == "group")	// tagged parameter
			group = r->Value();
		else		// it's a non-tagged parameter
		{
			if (c == 0)	// use order to assign to appropriate value (`switch' for more)
				item = r->Value();
			else
				newvalue = r->Value();
		}
	}
	
	if (item.length() > 0 && newvalue.length() > 0)
	{
		Event *ne = new Event("CheckBox");
		if (ne)
		{
			ne->source = "RSL";
			if (group.length() > 0)
				ne->AddDataPair("GroupID", group);
			ne->AddDataPair(item, newvalue);
			Open();
			OutEvent(ne);
		}
		else
			logf->Log(_ERROR, LOGAPPENV,
				"Form \"%S\": Unable to create Checkbox outevent.", &name);
	}
	else
		logf->Log(_ERROR, "Form.Checkbox for \"%S\": needs an item and a new value.",
			&name);
}

// Enable
// Enable or Disable based on `method'.
void R_Form::Enable(resource *field, char which)
{
String &method = which=='d'? EVENT_DisableField : EVENT_EnableField;
	
	if (!field)
	{
		logf->Log(_ERROR, LOGAPPENV, "Form: %s- no field.", &method);
		return;
	}
	Event *ne = new Event(method.chars(),
		"", "", "", "RSL");
	if (ne)
	{
		ne->field = field->Value();
		
		Open();
		OutEvent(ne);
	}
	else
		logf->Log(_ERROR, LOGAPPENV, "Form: %s- Unable to create outgoing event.",
			&method);
}

// DisplayItem
// type:<string> itemid:<string> field:<string> text:<string>
void R_Form::DisplayItem(SLList<resource *>& args)
{
Pix temp=args.first();
resource *r=NULL;
String type, field, itemid, text;

#ifdef DEBUG
	cerr << "R_Form::DisplayItem-- args are:\n";
	printordered(args);
#endif

	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;

		if (r->ClassName() == "String")
		{
			String n = downcase(r->Name());
			if (n == "type")
				type = r->Value();
			else if (n == "field")
				field = r->Value();
			else if (n == "itemid")
				itemid = r->Value();
			else text = r->Value();	// was the "text" tag
		}	
	}
	if (type.length() > 0)	// && itemid.length() > 0)
	{
		Event *ne = new Event("DisplayItem", "", "",
			(const char *) type, "RSL");
		if (ne)
		{
			ne->field = field;
			String bean = downcase(type);
			// switch on the type of DisplayItem
			if (bean.contains("graphic"))
				ne->AddDataPair(DFN_ItemId, text);
			else
			if (bean.contains("text"))	// text or appendtext
				 ne->AddDataPair(DFN_Text, text);
			else
			{
				logf->Log(_ERROR, LOGAPPENV, "Form.DisplayItem: unkown type \"%S\".",
					&type);
				// send error event ..?
			}

			Open();
			OutEvent(ne);
		}
		else
			logf->Log(_ERROR, LOGAPPENV, "Form.DisplayItem: Event object allocation error.");
	}
	else
		logf->Log(_ERROR, LOGAPPENV, "Form.DisplayItem: requires a type");
}

// SendList
// the sendlist event
void R_Form::SendList(SLList<resource *>& args, int whichlist)
{
Pix temp=args.first();
resource *r=NULL;
restable *thelist = NULL;
String field, damn_icon;
int i=0, using_icon=0;	// list item count
int new_restable = 0;	// whether a new restable was created here or not

#ifdef DEBUG
	cerr << "R_Form::SendList-- args are:\n";
	printordered(args);
#endif

	for(; temp; args.next(temp))
	{
		r = args(temp);
		if (!r) continue;

		if (r->ClassName() == "String")
		{
		    if (r->Name() == "field")
		    	field = r->Value();
		    else
		    	if (r->Name() == "icon")
		    	{
		    		 using_icon=1;
		    		 damn_icon = r->Value();
		    	}
		}
		else
		if (r->HierarchyContains("List"))
			thelist = (restable *) r;
	}
//			i = ((R_ListBox *) r)->WindowIndex();
	
	if (thelist)
	{
		restable *thereallist = thelist;
		if (thelist->ClassName() == "ListBox")
		{
			/* The following statement is of questionable value, because we have to
			 * introduce an R_Form dependency (not so bad in itself given the nature
			 * of R_Form itself - exemplified by the very fact that a Form object has
			 * a SendList method!!!) but the GetFormattedWindow implies that the user can
			 * only send the formatted window of a ListBox via SendList. What about
			 * the unformatted window, or the whole thing? It all ties into the
			 * incremental updating of form listboxes.
			 */
			thereallist = (restable *) ((R_ListBox *) thelist)->GetFormattedWindow();
			i = ((R_ListBox *) thelist)->WindowIndex();
			new_restable = 1;	// "GetFormattedWindow" returns a new restable

			/* ? error from GetFormattedWindow ? */
			if (!thereallist)
			{
				logf->Log(_INFO, LOGAPPENV, "Form.SendList(ListBox ..) -- error returned by GetFormattedWindow");
				return ;
			}
		}
		else
			i = 0;

		SLList<resource *>& tlist = thereallist->GetList();
		// This constructor is really lame.
		Event *re = new Event(((whichlist || using_icon)? EVENT_SendIconList : EVENT_SendList).chars(),
			"", "", "", "RSL");
		if (re)
		{
			re->field = field;
			re->AddDataPair("NumItems", dec(tlist.length()));

			// add the contents of the list as datapairs
			// in the outgoing event
			String thetext;
			for(temp=tlist.first(); temp; tlist.next(temp))
			{
				r = tlist(temp);
				if (r)
				{
					thetext = "";
					/* Warning: RMG specific hack coming.. */
					if (using_icon)
						thetext = damn_icon + " ";
					/* the previous statement is a hack because the RMG command is
					 * structured something like  .si <item> <icon> <text>
					 * and RMG puts in the <item> already, but not <icon> -- there's
					 * no easy place to put the icon in the SLList<resource *>
					 * So recognizing that there is a space between <icon> and <text>,
					 * I simply prepend the icon # to the text with a space in
					 * between them. Ugly but it works. -- RFH
					 */

					thetext += r->Value();
					re->AddDataPair(dec(i++), thetext);
				}
				
			}
			
			Open();
			OutEvent(re);

			// Delete restable & elements returned by ListBox::GetFormattedList
			if (new_restable)
			{
				DestroyResList(this, tlist);
				delete thereallist;
			}
		}
	}
	else
		logf->Log(_ERROR, LOGAPPENV, "Form.SendList in \"%S\": no List argument found.",
			&name);
}


// OutEvent
// Send the event up the ownership chain
// - add the form # to the event structure (address 2)
void R_Form::OutEvent(Event *e)
{
	if (!e)
		return;

	if (e->type == EVENT_Open)
		IsOpen() = 1;

	e->address2 = instance_id.Value();	// set the "form id"
	if (owner)
		owner->OutEvent(e);
	else
		logf->Log(_ERROR, LOGAPPENV, "Form.OutEvent: \"%s\": no App owner?",
			&instance_id);

}

// SetFormNum
// - This is the number in AOL's database for this form
// - It is set from within a form Create script which is executed
//     when the "template" forms are created at server startup time.
// - Contrast with the fid (form id) which is set on a per {form instance}
//     basis and is not user settable.
void R_Form::SetSysID(SLList<resource *>& args)
{

resource *r = args.front();
	if (r)
		{ if (system_id) system_id->Assign(r->Value()); }
/*
	else
		cerr << "Error: Form: bad argument for " << name << ".SetSysID\n";
*/

}

// ClearListBox outgoing event.
// First (and only) argument is expected to be the field.
void R_Form::ClearListBox(SLList<resource *>& args)
{
	resource *f = NULL;
	Pix temp=args.first();
	if (temp)
		f = args(temp);
	else
	{
		logf->Log(_ERROR, LOGAPPENV,
			"Form::ClearListBox expects a field argument.");
		return;
	}

	Event *ne= new Event(EVENT_ClearListBox);
	ne->source = "RSL";
	ne->field = f->Value();

	Open();	// Open or ReOpen event first
	OutEvent(ne);
}

int R_Form::LogicalValue(void)
{
	return 1;
}

