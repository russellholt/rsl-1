/*
 * SLList_res_util.h
 * Russell Holt
 */
#ifndef _SLLIST_RES_UTIL_H_
#define _SLLIST_RES_UTIL_H_

#include <String.h>
#include <fstream.h>
#include "restable.h"
#include "R_String.h"

// RLD = Resource List Delete
#define RLD_INDEX 'i'
#define RLD_NAME 'n'
#define RLD_POINTER 'p'
#define RLD_VALUE 'v'

void GetStringResources(fstream& in, SLList<resource *>& stuff);
void printreslist(SLList<resource *>& stuff);
void printresnamelist(SLList<resource *>& stuff);
void printordered(SLList<resource *>& stuff);
void printordered(SLList<resource *>& stuff, const char *type);

resource *ResListDelete(SLList<resource *>& elements, char how, ...);

resource *ResListDelete(SLList<resource *>& elements, String what,
	char how, resource *res);
	
resource *RLD_realwork(SLList<resource *>& elements, String& what, char how,
	int ind, resource *& res);


#endif
