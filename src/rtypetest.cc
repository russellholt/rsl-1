#include "R_String.h"
#include "R_Boolean.h"
#include "stream.h"

main()
{
R_String a;

	a = "hello";

	cout << a << '\n';
	if (a == "hello")
		cout << "a is hello\n";
	else
		cout << "a is not hello\n";

	R_Boolean b = 1;
	
	cout << "b is " << ((b == 1)? "true" : "false") << '\n';
}