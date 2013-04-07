// enum test
// compile with g++ enumtest.cc -o enumtest
// RFH 11/95
#include <stream.h>

class X {
public:
	enum state { closed, open_read, open_write,
		open_append };
	X(state s) : stream_state(s) { }
	int sstate(void) { return stream_state; }
private:
	state stream_state;
};


main()
{
	X a(X::open_write);	// will be 2
	
	cout << "a is " << a.sstate() << "\n\n";
	
}
