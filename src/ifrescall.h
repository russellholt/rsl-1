#ifndef _IF_RESCALL_
#define _IF_RESCALL_

#include <stream.h>
#include <String.h>
#include <SLList.h>
#include "resource.h"
#include "restable.h"
#include "rescall.h"


class ifResCall : public ResCall {
private:
	Init(void) { true_res = false_res = NULL; }
protected:
	resource *true_res, *false_res;
public:
	ifResCall(void) : ResCall() { Init(); }
	ifResCall(String &rn, String &mn) : ResCall(rn, mn) { Init(); }
	ifResCall(char *rn, char *mn) : ResCall(rn, mn) { Init(); }
	ifResCall(String &rn, String &mn, resource *t, resource *f)
		: ResCall(rn, mn) { Init(); true_res = t; false_res = f; }

	resource *execute(String& method, SLList<resource *> &args);
	resource *execute(restable *arguments, restable *globals,
		restable *globals2, restable *locals,
		restable *result_temps=NULL);

	inline resource *TrueRes(void) { return true_res; }
	inline resource *FalseRes(void) { return false_res; }
	inline void SetTrueRes(resource *r) { true_res = r; }
	inline void SetFalseRes(resource *r) { false_res = r; }

	void print(void);
	void TextEquiv(String &text, int showname=0);
	
};

#endif