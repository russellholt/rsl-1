// rescall_xadditions.cc
// -experimental or unstable routines
// for rescall.
//
//
#ifndef XPERIMENTAL
#define XPERIMENTAL
#endif

#include <SLList.h>
#include "rescall.h"

extern void DestroyResList(resource *owner, SLList<resource *>& thelist);

ResCall::~ResCall(void)
{
	DestroyResList(this, args);
	
	/* what to do about resptr?? */
}


