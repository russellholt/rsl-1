// Memory analysis tests

#include "purify.h"

#include <stream.h>
#include <SLList.h>
#include <String.h>

#include "resource.h"
#include "R_String.h"
#include "rescall.h"
#include "R_Integer.h"
#include "SLList_res_util.h"
#include "rsl_control.h"

extern void DestroyResList(resource *owner, SLList<resource *>& thelist);

main(int argc, char **argv)
{
	purify_printf("Resource memory analysis\n");

	purify_printf("%s","0. new R_String(\"String1\", \"0123456789\") :\n");
	purify_clear_inuse();
	resource *r = new R_String("String1", "0123456789");
	purify_new_inuse();

	purify_printf("%s","1. delete R_String, check leaks:\n");
	purify_clear_leaks();
	delete r;
	purify_new_leaks();

	purify_printf("%s","2. SLList of 10 R_String *  (10 chars each) :\n");
	int i;
	purify_clear_inuse();
	SLList<resource *> thelist;
	for (i=0; i<10; i++)
		thelist.append(new R_String("String1", "0123456789"));
	purify_new_inuse();
	
	purify_printf("%s","3. DestroyResList, check leaks:\n");
	purify_clear_leaks();
	DestroyResList(NULL, thelist);
	purify_new_leaks();
	
	if (argv[1])	// rsl filename to parse
	{
		purify_printf("4. RSL program %s:\n", argv[1]);
		purify_clear_inuse();
		restable *globals = new restable("Globals");
		rsl_control curb(globals);
		curb.ParseFile(argv[1]);
		purify_new_inuse();
		curb.printstats();
		
		if (argv[2])	// execute the rsl if there is another arg
		{
			purify_printf("5. Execute the rsl program....");
			purify_clear_inuse();
			curb.execute("Main");
			purify_new_inuse();
		}
	}
	
}