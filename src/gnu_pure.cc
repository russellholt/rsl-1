// Memory analysis tests

#include "purify.h"

#include <stream.h>
#include <SLList.h>
#include <String.h>

main()
{
	cerr << "Memory analysis of common operations.\n";

	cerr << "0. 10 character char * declaration/assignment:\n";
	purify_clear_inuse();
	char *abc = new char[10];
	purify_new_inuse();
	
	{
		cerr << "1. 10 character String declaration:\n";
		purify_clear_inuse();
		String x = "0123456789";
		purify_new_inuse();
	
		cerr << "\n-----------\n2. String goes out of scope, check for leaks:\n";
		purify_clear_leaks();
	}
	purify_new_leaks();
	
	
	cerr << "\n-----------\n3. SLList<String> declaration:\n";
	purify_clear_inuse();
	SLList<String> y;
	purify_new_inuse();

	
	cerr << "\n-----------\n4. Add 10  10-character Strings to the list:\n";
	int i;
	purify_clear_inuse();
	
	for(i=0; i<10; i++)
		y.append(String("0123456789"));
	purify_new_inuse();

	cerr << "\n-----------\n2. Clear list, check for leaks:\n";
	purify_clear_leaks();
	y.clear();
	purify_new_leaks();


}