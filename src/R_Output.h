/*********************************************
*                                            *
*   An "output" class for the CRB language   *
*   Russell Holt                             *
*   Sept 12, 1995                            *
*                                            *
*********************************************/

#ifndef _R_OUTPUT_H_
#define _R_OUTPUT_H_

#include <SLList.h>
#include <String.h>

#include "resource.h"

class R_Output : public resource  {
	Init(void) { resource::class_name = "Output"; }

public:
	
	R_Output(void)  { Init(); }
	R_Output(String& nm) { Init(); resource::name = nm; }
    R_Output(char *nm) { Init(); resource::name = nm; }

	resource *execute(String &method, SLList<resource *> &args);

	inline void PutTo(String &arg) { cout << arg; }
	void PutTo(SLList<stringpair> &args);
	void PutTo(SLList<resource *> &args);

	void SendFile(SLList<stringpair> &args);
	void SendFile(SLList<resource *> &args);
};

#endif