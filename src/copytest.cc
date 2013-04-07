// copytest.cc
// copy constructor test - copy an object and see what is the same
// between them
// compile with g++ copytest.cc -o copytest
// RFH 10/95
#include <stream.h>

class X {
public:
	int *i, *j;
};


main()
{
X *a=NULL, *b=NULL;

	a = new X;
	a->i = new int(5);
	a->j = new int(10);

	// want to copy the addresses of a->i and a->j
	b = new X(*a);	// call default copy constructor
	
	// print the addresses of the class members,
	// see if they're the same
	cout << "a->i == " << a->i << '\n';
	cout << "a->j == " << a->j << "\n\n";
	cout << "b->i == " << b->i << '\n';
	cout << "b->j == " << b->j << "\n\n";
	
}

