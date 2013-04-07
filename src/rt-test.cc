
#include "restable.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "R_File.h"
#include <String.h>

main()
{
R_String x, y, z;

	x = "first element";
	y = "second";
	z = "number three";

restable rt;

	rt.Add(&x);
	rt.Add(&y);
	rt.Add(&z);
	
	cout << "The table:\n";
	rt.print();
	
	cout << "\nelement 0: ";
	rt[0]->print();
	
	cout << "\nelement 1: ";
	rt[1]->print();
	
	cout << "\nelement 2: ";
	rt[2]->print();

	cout << "\n\n";
}

