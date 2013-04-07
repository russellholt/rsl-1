#ifndef _R_HTML_
#define _R_HTML_

#include <String.h>
#include <SLList.h>
#include "resource.h"

class R_html : public resource {
private:
	Init(void) { class_name = "HTML"; }
	
public:
	R_html(void) { Init(); }
	R_html(String &nm) { Init(); name = nm; }
	R_html(const char *nm) { Init(); name = nm; }
		
	resource *Create(String &nm, resource*& table);
	resource *execute(String& method, SLList<resource *> &args);
	resource *list_front(SLList<resource *> &args);
	resource *xlist(const char *tag, SLList<resource *> &args);
	String makexlist(SLList<resource *> &args, int tagit, const char *tag);
	String makeItemString(resource *r);
	resource *title(resource *r);
	void print(void);

};

#endif