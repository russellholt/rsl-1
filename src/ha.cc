#include <String.h>
#include <stream.h>

unsigned int ha(const char *s0);
unsigned int collatz(unsigned int i);
String OpToString(String op);


main(int argc, char **argv)
{
int docase=0, def=0, count=0, i=0, nums=0;
	for (i=1; i<argc; i++)
	{
		String ar = argv[i];
		if (ar == "-case")
			docase = 1;
		if (ar == "-def")
			def = 1;
		else if (ar == "-n")
			nums=1;
		else if (ar == "-h")
		{
			cerr << "Usage: " << argv[0]
				 << "\n\t -case: print C case statement entries"
				 << "\n\t -def : print C #define's"
				 << "\n\t -n   : print hash mapping\n";
			cerr << "To produce defines sorted numerically:\n\t"
				 << argv[0] << " -def < infile | sort +2 -n\n";
			cerr << "To visually check for collisions:\n\t"
				 << argv[0] << " -n < infile | sort -n\n";
			exit(1);
		}
	}

String s, out;
unsigned int wh;

	while(!cin.eof())
	{
		cin >> s;
		if (cin.eof())
			break;

		wh = ha(s.chars());
		
		if (docase || def)
		{
			if (s.matches(RXidentifier))
			{
				out = upcase(s);
			//	cout << "#define " << s << ' ' << wh << '\n';
			}
			else
				out = String("Op") + OpToString(s);

			if (def) 	// print `#define's
				cout << "#define _h" << out << ' ' << wh
					 << "\t// " << s << '\n';
			else	// print case statement entries
				cout << "\t\tcase _h" << out << ":\t// \""
					 << s << "\"\n\t\t\tbreak;\n";
		}
		else
		if (nums)
			cout << wh << '\t' << s << '\n';
		else
		{
			cout << wh << ' ';
			if (++count > 10)
			{
				cout << '\n';
				count = 0;
			}
		}
	}	// while
	cout<< '\n';
}

// ha
// the hash
unsigned int  ha(const char *s0)
{
String s = s0;
int len = s.length(), i;
unsigned int what=s[0], pwhat=0;

	for(i=0; i<len; i++,pwhat=what)
		what ^= collatz(pwhat ^ s[i]);
	return (unsigned int) what;
}

// collatz
// compute "hailstone" sequence
unsigned int collatz(unsigned int i)
{
	if (i&1)	// if odd
		return 3*i + 1;
	return i>>1;	// divide by 2
}

String OpToString(String op)
{
	if (op == "==")
		return "EQ2";
	if (op == "!=")
		return "NE";
	if (op == "+")
		return "Add";
	if (op == "-")
		return "Subt";
	if (op == "*")
		return "Mult";
	if (op == "/")
		return "Div";
	if (op == "<")
		return "LT";
	if (op == ">")
		return "GT";
	if (op == "<=")
		return "LE";
	if (op == ">=")
		return "GE";
	if (op == "&&")
		return "And";
	if (op == "||")
		return "Or";
	if (op == "!")
		return "Not";
	if (op == "++")
		return "PP";
	if (op == "--")
		return "MM";
	if (op == ">=")
		return "GE";
	if (op == "<<")
		return "LS";
	if (op == ">>")
		return "RS";
	if (op == "=")
		return "EQ";
	if (op == "+=")
		return "PE";
	if (op == "-=")
		return "ME";
	if (op == "*=")
		return "TE";
	if (op == "/=")
		return "DE";
	if (op == "%=")
		return "ModE";
	if (op == "/=")
		return "DE";
	if (op == "/=")
		return "DE";
	if (op == "%")
		return "Mod";
	return "ERR";
}
