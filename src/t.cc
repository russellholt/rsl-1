#include <stream.h>
#include <SLList.h>

main()
{
SLList<int *> x;

	int *o = NULL;
	if (x.first())
		o = x.front();

}
