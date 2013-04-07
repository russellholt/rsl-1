// Test of the restree class. Heh.
//

#include "R_String.h"
#include "restree.h"
#include <stream.h>

main()
{
R_String a, b, c, d, e, f;

	a = "aaa\n";
	b = "bbb\n";
	c = "Ccc\n";
	d = "Ddd\n";
	e = "eee\n";
	f = "fff\n";

	a.SetName("a");
	b.SetName("b");
	c.SetName("c");
	d.SetName("d");
	e.SetName("e");
	f.SetName("f");

restree tree;

	tree.Insert(&d);	//	, "GetUpcase");
	tree.Insert(&a);	//	, "GetUpcase");
	tree.Insert(&c);	//	, "GetUpcase");
	tree.Insert(&b);	//	, "GetUpcase");
	tree.Insert(&f);	//	, "GetUpcase");
	tree.Insert(&e);	//	, "GetUpcase");
	
	cout << "\n\n\n";
	tree.print();
	cout << "\n\n\n";
	
	cout << "Testing Find:\n"
		 << "Find 'e':\n";
	
	resource *r = tree.FindByName("e");
	
	if (r)
		r->print();

	cout << "\nFind 'c':\n";
	r = tree.FindByName("c");
	
	if (r)
		r->print();
		
		
}