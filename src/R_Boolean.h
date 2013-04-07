#ifndef _R_BOOLEAN_H_
#define _R_BOOLEAN_H_

#include <String.h>
#include <SLList.h>
#include "resource.h"

#define _undefined -1

class R_Boolean : public resource {
	int value;
	void Init(void) { class_name = "Boolean"; value = 0; }
public:

	R_Boolean(void) { Init(); }
	R_Boolean(int val) { Init(); value = val; }
	R_Boolean(String& nm, int val) { Init(); name = nm; value = val; }
	R_Boolean(char *nm, int val) { Init(); name = nm; value = val; }

	// resource virtual functions

	resource *Create(String &nm, resource*& table);
	
	resource *execute(String& method, SLList<resource *> &args);
	
	int LogicalValue(void);
	
	resource *Assign(resource *f);
	inline String Value(void) { return (value>0) ? String("true") : String("false"); }	

	resource *Equal(resource *f);
	resource *And(resource *f);
	resource *Or(resource *f);
	resource *LessThan(resource *f);
	resource *GreaterThan(resource *f);
	resource *NotEqual(resource *f);
	resource *LessThanEqual(resource *f);
	resource *GreaterThanEqual(resource *f);
	resource *Not(resource *f);
	
	/// multiple arguments
	resource *And(SLList<resource *>& args);
	resource *Or(SLList<resource *>& args);
	
	void print(void) { cout << Value(); }

	
	
	inline void Set(int v) { value = v; }
	int R_Boolean::FromString(String s);
	inline void operator=(int v) { value = v; }
	friend inline int operator==(R_Boolean &b, int v);
};

inline int operator==(R_Boolean &b, int v)
{
	return (b.value == v);
}

#endif