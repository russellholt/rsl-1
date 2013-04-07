/* -------------------------------------------------------
 * ResCall (rescall.h)
 * represents one call to a resource method with arguments
 *
 *
 * $Id: rescall.h,v 1.7 1996/03/18 13:26:14 holtrf Exp $
 *
 * RFH 9/95
 * -------------------------------------------------------*/
#ifndef _RESCALL_H_
#define _RESCALL_H_

#include <stream.h>
#include <String.h>
#include <SLList.h>
#include "resource.h"
#include "restable.h"
/*
#define _domain_expression 0
#define _domain_statement 1
*/

//#define rescall_standard 0
//#define rescall_if 1
//#define rescall_for 2


#define rescall_allocation 14
#define rescall_var_ref 15
#define rescall_obj_var_ref 16

class ResCall : public resource {
private:
	Init(void)
	{
		class_name = "ResCall"; hierarchy += "ResCall:";
		resptr = NULL;	// domain = _domain_statement;
		type = 0;
	}	
protected:
	//	String res_name;
	//	SLList<stringpair> args;
	//	int domain;
	resource *resptr;
	String method_name;
	SLList<resource *> args;
	int type;

public:
	static int SaveResptr;
	static restable References;
	
	ResCall(void) { Init(); }
	ResCall(String &rn, String &mn) { Init(); name = rn; method_name = mn; }
	ResCall(char *rn, char *mn) { Init(); name = rn; method_name = mn; }

	void AddArgument(resource *arg);
	void AddArgument(stringpair &arg);		// both text calls default
	void AddArgument(const char *nm, const char *arg);	// to String resources.
	void AddArgument(const char *arg);
	void AddArgument(String &arg);
	
	void InsertArg(resource *arg);

	inline void SetResPtr(resource *rsrc) { resptr = rsrc; }
	
	resource *_execute(SLList<resource *>& args);
	resource *execute(String& method, SLList<resource *> &args);

//	int FindResource(restable *vars);
	resource *FindResource(restable*& arguments, restable*& globals,
		restable*& globals2, restable*& locals,restable*& result_temps,
		int& scope_resolved);

	virtual resource *execute(restable *arguments, restable *globals, restable *globals2,
		restable *locals, restable *result_temps=NULL);
	void evaluate_args(SLList<resource *>& args, SLList<resource *>& results,
		restable *arguments, restable *globals, restable *references,
		restable *locals, restable *result_temps);
	void StoreResult(resource *theresult, restable *locals, restable *result_temps);

	//	inline String& ResName(void) { return res_name; }
	inline resource *ResPtr(void) { return resptr; }
	inline String& MethodName(void) { return method_name; }
	//	inline SLList<stringpair> &Args(void) { return args; }
	inline SLList<resource *> &Args(void) { return args; }

	//	friend inline int operator==(ResCall &r, String& s);
	void print(void);
	virtual void TextEquiv(String &text, int showname=0);
	
	inline int Type(void) { return type; }
	inline void Type(int i) { type = i; }

	String Value(void);
};

/*
// ??? useful?
inline int operator==(ResCall &r, String& s)
	{ return(r.res_name == s); }
*/

#endif