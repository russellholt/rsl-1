#ifndef _LOOP_RESCALL_H_
#define _LOOP_RESCALL_H_

#include <stream.h>
#include <String.h>
#include <SLList.h>
#include "resource.h"
#include "restable.h"
#include "rescall.h"


class loopResCall : public ResCall {
private:
	Init(void) { loop_res = NULL; }
protected:
	resource *loop_res;
public:
	loopResCall(void) : ResCall() { Init(); }
	loopResCall(String &rn, String &mn) : ResCall(rn, mn) { Init(); }
	loopResCall(char *rn, char *mn) : ResCall(rn, mn) { Init(); }
	loopResCall(String &rn, String &mn, resource *l)
		: ResCall(rn, mn) { Init(); loop_res = l; }

	resource *execute(String& method, SLList<resource *> &args);
	resource *execute(restable *arguments, restable *globals,
		restable *globals2, restable *locals,
		restable *result_temps=NULL);

	inline resource *LoopRes(void) { return loop_res; }
	inline void SetLoopRes(resource *r) { loop_res = r; }

	void print(void);
	void TextEquiv(String &text, int showname=0);
	
};

#endif