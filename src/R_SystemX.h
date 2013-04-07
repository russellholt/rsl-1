// R_System.h
//
// PURPOSE: system utilities
//
// PROGRAMMING NOTES:
//
// HISTORY:
//		1/8/96 Russell Holt
//
// $Id: R_SystemX.h,v 1.1 1996/03/18 13:21:44 holtrf Exp $
// Copyright 1995 by Destiny Software Corporation.
#ifndef _R_System_H_
#define _R_System_H_

#include <String.h>
#include <SLList.h>
#include "../crb/resource.h"

#ifndef SYSTEM_EMAIL
#define SYSTEM_EMAIL
#endif

class R_System : public resource {
private:
	void Init(void);

protected:
		// any data members go here

public:
// constructors	
	R_System(void) { Init(); }
	R_System(String &nm) { Init(); name = nm; }
	R_System(char *nm) { Init(); name = nm; }

// virtual functions, overriding those in resource
	resource *Create(String &nm, resource*& table);
	resource *execute(String& method, SLList<resource *> &args);

	String Value(void);
	void print(void);
	int LogicalValue(void);

// R_System-specific member functions

#ifdef SYSTEM_EMAIL
	void email(SLList<resource *> &args);
#endif


};

#endif
