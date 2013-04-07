// rsl_control.h
//
// PURPOSE:
//	A scripting environment for rsl.
//	Use this class to augument programs with rsl.
//
// USAGE:
//
// PROGRAMMING NOTES:
//
// HISTORY: 
//	9/28 - began
//	9/29 - working version 1
//
// $Id: rsl_control.h,v 1.2 1996/03/18 13:27:20 holtrf Exp holtrf $
// Copyright 1995 by Destiny Software Corporation.


#ifndef _RSL_CONTROL_H_
#define _RSL_CONTROL_H_

#include "resource.h"

#include "y.tab.h"
#include "r2.h"
#include <String.h>
#include <SLList.h>
#include <stream.h>
#include <fstream.h>

#include "resource.h"
#include "rescall.h"
#include "R_String.h"
#include "R_Output.h"
#include "R_File.h"
#include "restable.h"
#include "../server/cgi.h"
#include "../server/stringpair.h"


class rsl_control {
	int is_setup;
	restable *globals;
	String ScriptFileName;

	void Init(void) { globals = NULL; is_setup=0; }
	void make_program(struct res_call *rc, restable *globals);
	resource *make_resource(struct res_call *r);
	restable *make_restable(struct res_call *rc);

public:
	rsl_control(void) { Init(); }
	rsl_control(restable *g) { Init(); globals = g; Setup(); }

	void Setup(void);
	void add_vars(SLList<stringpair> &vars);
	void add_var_table(String nm, SLList<stringpair> &vars);
	inline void AddType(resource *r);
	int ParseFile(int &use_stdin);
	int  ParseFile(const char *fn);

	void SetGlobalTable(restable *g);
	inline restable *GetGlobalTable(void) { return globals; }
	
	void execute(const char *nm, restable *locals=NULL);

	void AddReferenceObject(resource *r);
	void AddGlobalObject(resource *r);
	
	resource *GetResource(char *nm);
	
	void printstats(void);

	virtual void InstallReferences(void);
};

#endif