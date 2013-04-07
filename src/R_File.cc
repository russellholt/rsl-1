/* R_File
 *
 * $Id: R_File.cc,v 1.4 1996/03/29 21:41:49 holtrf Exp holtrf $
 *
 * Russell Holt, created July 24 1995
 */
#include <fstream.h>
#include "R_File.h"
#include "restable.h"
#include "../server/SLList_util.h"
#include "SLList_res_util.h"
extern "C"
{
#include <stdio.h>
}

void printfound(String* items, int itemsize, String* global_tags, int tagsize, int* Found);

resource *R_File::Create(String &nm, resource*& table)
{
	table = NULL;
	return new R_File(nm);
}


/* execute
 *  route named method calls with arguments.
 * RFH
 */
resource *R_File::execute(String& method, SLList<resource *>& args)
{
//	String &the_arg = (args.front()).right();
//	String &the_arg = ArgVal(args.front());

resource *f = NULL;
	if (args.first())
		f = args.front();

	String m = downcase(method);

	if (m == "send" || m == "print")
		PrintFiles(args);
	else
	if (m == "append" || m == "<<")
		Write(args, open_append);
	else
	if (m == "write")
		Write(args, open_write);
	else
	if (m == "open")
		Open(args);
	else
	if (m == "close")
		Close();
	else
	if (m == "file")	// File(..) ==> File.File(..)
	{
		if (!f) return NULL;
		R_File *r = new R_File();
		r->MakeExist(exist_temp);
		r->SetFilename(f->Value());
		return r;
	}
	else
	if (m == ">>" || m == "read")	// read the file
	{
		if (f)
			return _read_(f);
		else
			return NULL;
	}
	else
	{
		if (!f || f->ClassName() != "String")
			return NULL;

		String s = f->Value();

		if (m == "setname")	// rsl_control::make_resource for TYPE_FILE
		{
//			if (!the_stream)
			if (stream_state == closed)
				filename = s;
			else
			{
				cout << class_name << " " << name << ": "
					 << method << ": file already open.\n";
				return NULL;
			}
		}
		else
		if (m == "delete")
		{
			cout << class_name << ": " << method << "not implemented.\n";
			return NULL;
		}
		else
		if (m == "searchand")
			return SearchFile(args, op_and);
		else
		if (m == "searchor")
			return SearchFile(args, op_or);

	}
	
	return this;
}

// Write
// Write or append all arguments to the file, based on st.
// 1. Open the file, if not already opened in mode append.
// 2. append the Value of each argument to the file
void R_File::Write(SLList<resource *> &args, state st, int inlist)
{
#ifdef DEBUG
	cout << "write for " << filename << " with:\n";
	resource::print(args, "   ");
#endif
	
//	int ok = Open(st);	// make sure it is open
	
	if (stream_state != closed)
	{
		Pix temp = args.first();
		resource *r = NULL;
		for (; temp; args.next(temp))
		{
			r = args(temp);
			if (r)
			{
				if (r->ClassName() == "List")	// recursively append restable contents
					Write(((restable *)r)->GetList(), st, 1);
				else
				{
					String v = r->Value();
					if (inlist)
					{
						if (v.length() == 0 || v[v.length()-1] != '\n')
							v += '\n';
					}
//					*((ostream *)the_stream) << v;
//					the_stream->write(v.chars(), v.length());
					the_stream.write(v.chars(), v.length());
				}
			}
		}
	}
	else
		cout << class_name << " " << name << ": Write: stream closed\n";
}

// Open
// Opens a stream for the file named by R_File::filename
//  or verifies that it is already opened in the requested state.
// If the stream is already open but in a different state than
// requested, it will not change state or reopen, just fail.
int R_File::Open(state newstate)
{
	if (stream_state == newstate)
		return 1;	// ok
	else
	{
//		if (the_stream)	// already open in another state
//			return 0;	// = fail
		if (newstate == open_read)
		{
			// mode ios::in - open for input
//			the_stream = new fstream(filename.chars(), ios::in);
			the_stream.open(filename.chars(), ios::in);
			if (the_stream.good())	//	if (the_stream)
			{
				stream_state = newstate;
				return 1;
			}
		}
		else
			if (newstate == open_append)
			{
				// mode ios::app - append
//				the_stream = new fstream(filename.chars(), ios::app);
				the_stream.open(filename.chars(), ios::app);
				if (the_stream.good())	// if (the_stream)
				{
					stream_state = newstate;
					return 1;
				}
			}
			else
				if (newstate == open_write)
				{
					// mode ios::out - open for output
//					the_stream = new fstream(filename.chars(), ios::out);
					the_stream.open(filename.chars(), ios::out);
					if (the_stream.good())
					{
//						cout << "open " << filename << " for writing\n";
						stream_state = newstate;
						return 1;
					}
				}
				else
					cout << class_name << ' ' << name
						 << ": Open: Unknown file state, unable to open\n";
	}
	
	cout << "File: Unable to open " << filename << '\n';
	return 0;
}

// Open
// Opens a file with a mode
// expects a filename and a mode string, one of "read", "write", "append".
// If no arguments are given, it tries to use R_File::filename with open_append
// as the default mode.
void R_File::Open(SLList<resource *> &args)
{
	
	if (stream_state != closed)
	{
		cout << class_name << " " << name << ": stream already open.\n";
		return;
	}

state st = open_read;	// default mode

	if (args.length() == 0)
	{
		if (filename == "")
		{
			cout << class_name << " " << name << ": Open: no filename specified\n";
			return;
		} 
	}
	else
	{
		Pix temp = args.first();
		resource *r = args(temp); args.next(temp);
		String a,b;
		if (r) a= r->Value();
			else return;

		if (temp)
		{
			r = args(temp);
			if (r) b = r->Value();
		}
		
		// a is expected to have the filename, b is expected to have the mode.
		b.downcase();

		// analyze the mode
		if (b == "")
		{
			// give useful error message
			cout << class_name << " " << name << ": Open: requires a file mode, "
				 << "one of \"read\", \"write\", or \"append\"\n";
			return;
		}
		else
			if (b == "read")
			{
				st = open_read;
			}
		else
			if (b == "write")
			{
				st = open_write;
			}
		else
			if (b == "append")
				st = open_append;
		else
			{
				cout << class_name << " " << name
					 << ": Open: unknown file mode \"" << b << "\"\n";
				return;
			}
		// at this point, the mode should be ok.
		// check the filename
		if (a == "")
		{
			cout << class_name << " " << name
					 << ": Open: requires a filename\n";
			return;
		}
		// now, everything should be ok.
		
		filename = a;	// R_File::filename
	}
	
	if (!Open(st))
	cout << class_name << " " << name << ": Open: unable to open file "
		 << filename << '\n';

}

// Close
// close the stream, set its state to 'closed', and blank the file name.
void R_File::Close(void)
{
	if (the_stream.good())
	{
//		the_stream->close();
//		delete the_stream;	// files are closed in destructors
//		the_stream = NULL;
		the_stream.close();
	}
	stream_state = closed;
	filename = "";
}

// PrintFiles
// print a list of files named in args to stdout
// if there are no args, then print the file named by R_File::filename
// Calls R_File::PrintFile to print the files.
// - maybe this is trying to do too many things in one class, and this
//   method probably won't be used very often anyway, if ever.
//   Even though it is legal and would work, it doesn't really make much
//   sense to say (in rsl):
//
//        File a;
//        a.open("myfile", read);
//        a.printfiles("file1", "file2", "file3");
//
//   although it does make sense to say:
//
//        File.printfiles("file1", "file2", "file3");
//
void R_File::PrintFiles(SLList<resource *> &args)
{
	if (args.length() > 0)
	{
		Pix temp = args.first();
		resource *r = NULL;
		for(; temp; args.next(temp))
		{
			r = args(temp);
			if (r && r->ClassName() == "String")
				PrintFile(r->Value());	// R_File::PrintFile()
		}
	}
	else PrintFile();
}

// PrintFile
// use ifstream to read a named file. Print it to stdout.
// This is easy but not efficient. Rewrite to use system calls
// read and write to be fast.
// - if fname == "" then use R_File::filename
void R_File::PrintFile(String fname)
{
String &fn = (fname.length()>0)? fname : filename;

	ifstream ifile(fn.chars());
	if (!ifile)
	{
		cout << class_name << ": unable to open file "
			 << fn << "\n";
		return;
	}
	SLList<String> d;
	ifile >> d;
	cout << d;
}


// _read_
// Called with method ">>" in the form:
//			expression >> myfile
// which becomes myfile.>>(expression)
// So this means take the contents of myfile and put it into
// whatever resource "expression" evaluates to.
// This means we must do different things based on the type
// of the "expression" (the resource).
// RFH 9/23/95
resource *R_File::_read_(resource *thearg)
{
	if (!thearg)
		return NULL;
String rtype = thearg->ClassName();

	// first check arg type.  expecting only one arg.
	if (rtype == "List")	// restable
	{
		// read into a restable - one line per resource
		// - move this to restable::rslRead() ???
		restable &rt = *((restable *)thearg);
		SLList<resource *> &where = rt.GetList();
		if (stream_state == closed)
		{
			fstream ifile(filename, ios::in);
			if (ifile)
				GetStringResources(ifile, where);
			ifile.close();
		}
		else
			GetStringResources(the_stream, where);
	}
	else
	{	// Other resources - call rslRead()
		if (Open(open_read) && the_stream.good())
			return thearg->rslRead(the_stream);
	}

	return NULL;
}

/*****************************************
 *
 *  Searching functions
 *
 *****************************************/

// SearchFile
// arguments: String name, List match-list, String found-format
resource *R_File::SearchFile(SLList<resource *>& args, R_File::oper Op)
{
Pix temp = args.first();
	if (!temp)
		return NULL;

String fname, found_format, sep;
restable *matchlist=NULL, *format_list=NULL;
resource *r = NULL;
int i=0, match_case = 0, contains=0;

	/*******************************
	 * Interpret arguments
	 *******************************/
	for(; temp; args.next(temp))
	{
#ifdef DEBUG
		cout << "\n----\n";
#endif
		r= args(temp);
		if (!r) continue;	//does this EVER really happen...???

		switch(i)
		{
			case 0:	// file name
				if (r->ClassName() == "String")
					fname = r->Value();
				else
					cerr << "SearchFile: argument 1 must be a filename String.\n";
				break;
			case 1:
				if (r->ClassName() == "List")
					matchlist = (restable *) r;
				else
					cerr << "SearchFile: argument 2 must be a List of match Strings\n";
				break;
			case 2:
						/*
							if (r->ClassName() == "String")
								found_format = r->Value();
						*/
				if (r->ClassName() == "List")
					format_list = (restable *) r;
				else
					// cerr << "SearchFile: argument 3 must be an output format String\n";
					cerr << "SearchFile: argument 3 must be a List of format Strings.\n";
				break;
			case 3:
				if (r->ClassName() == "String")
					sep= r->Value();
				else
					cerr << "SearchFile: argument 4 must be a separator String\n";
				break;
			case 4:
				match_case = r->LogicalValue();
				break;
			case 5:
				contains = r->LogicalValue();
				break;
			default: ;
		}
		i++;
	}
	if (!matchlist || !format_list || fname.length() <= 0)
	{
		cerr << "File " << name << ".SearchFile: requires filename & match List.\n";
		return NULL;
	}

	/*******************************
	 * Open the read stream
	 *******************************/
	ifstream inf(fname.chars());
	if (!inf)
	{
		cerr << "File " << name << ".SearchFile: unable to open "
			 << fname << '\n';
		return NULL;
	}
	
	/*******************************
	 * Build match against array
	 *******************************/
	int sz = matchlist->length() + 1;
	String *match_against = new String[sz];
	resource *w = matchlist->BeginIteration();

	for(i=0; w; i++,w=matchlist->GetNextResource())
		match_against[i] = w->Value();


	/*******************************
	 * Do the search
	 *******************************/

	s_searchfile(inf, match_against, Op, sz, match_case, format_list,
		sep, contains);

	// Clean up
	inf.close();
	delete[] match_against;
}

// s_searchfile
// File consists multiple records, one per line (row), in the following format:
//
//  category1 <tab> category2 <tab> ... categoryN
//  item 1,1  <tab> item 1,2  <tab> ... item 1,N
//  item 2,1  <tab> item 2,2  <tab> ... item 2,N
//  ...
//  item M,1  <tab> item M,2  <tab> ... item M,N
//
// - A search array is provided in the `match_against' parameter, whose
//	 size is `match_width'. This is the maximum number of columns that will be
//	 used in the match. It *should* be greater than the max number of columns
//	 in the file.
// - The search array is matched against each record in the file, column by
//   column. `Op' determines when a record is "found":
//      op_or:  one or more of the items is matched;
//      op_and: all of the items must match.
// - `matchcase' set to 0 allows case insensitive item comparisions.
//
// RFH Jan-March 1996. Put into R_File on March 28 1996.
void R_File::s_searchfile(ifstream& inf, String* match_against,
	oper Op, int match_width, int matchcase, restable* format_list,
	String sep, int contains)
{
String line;
static String TAB = "\t";

	// Get first line -- contains categories
	
	s_Read(inf, line);
	
#ifdef DEBUG
	cout << "Categories are: " << line << '\n';
#endif
	
	// categories are separated by tabs
	int width = line.freq("\t") + 1;

	/*******************************
	 * Build format string array
	 *******************************/
	String *flist = new String[width];	// needs to be max'd
	resource *w = format_list->BeginIteration();

	int i;
	for(i=0; w; i++,w=format_list->GetNextResource())
		flist[i] = w->Value();
	
	String *categories = new String[width];
	String *line_items = new String[width];
	int *Found = new int[width];

	split(line, categories, width, TAB);	// get individual categories.
	
	int row_width, nfound;

	while (!inf.eof())
	{
		line = "";
		s_Read(inf, line);	//	inf >> line;

		if (inf.eof())
			break;

		row_width = split(line, line_items, width, TAB);	// items from the line
#ifdef DEBUG
		cout << "\n----Read: " << line << '\n';
		cout << row_width << " items in the line.\n";
#endif

		nfound = s_Match(line_items, row_width, match_against,
			match_width, matchcase, contains, Found);

		// Print the items that were matched for this line, if any,
		// based upon the given boolean operation.               
		if (((Op == op_and) && nfound == width) || (Op == op_or && nfound >0))
		{
			if (rs_printfound(line_items, row_width, categories, match_width,
				Found, flist))
				// print separator
				cout << sep;
		}
			
	}
	inf.close();

	delete[] categories;
	delete[] line_items;
	delete[] Found;
}

// s_Read
// Unlimited length read into a given String.
void R_File::s_Read(ifstream& in, String& s)
{
char *inp=NULL;
	in.gets(&inp);
	s = inp;
	delete inp;	//	(??)
}

// s_Match
// for each i up to min(nitems, nitems2)
//    Set Found[i] to 1 when items[i] == items2[i].
//
// Want to know whether the strings in the `items' array match those
// in the `items2' array,  where position is important. The paramter
// `matchcase' will do exactly that...
// 
// Returns the total number of matched strings.
int R_File::s_Match(String* items, int nitems, String* items2,
	int nitems2, int matchcase, int contains, int *Found)
{
int i, found=0;
String sd, sd2;

	for (i=0; i<nitems && i < nitems2; i++)
	{
#ifdef DEBUG
		cout << "{ " << items[i] << " ? " << items2[i] << "} ";
#endif
		if (matchcase)
			Found[i] = contains?items[i].contains(items2[i])
				:	(items[i] == items2[i]);
		else
		{
			sd2 = downcase(items2[i]);
			sd = downcase(items[i]);
			Found[i] = contains?
					(sd.contains(sd2))
				:	(sd == sd2);
		}

		// inc the total found
		// -- allow the match against items to be blank though!
		found += (Found[i] || (items2[i] == ""));
	}
	return found;
}

// s_printfound
//   print the items that were found in a single row,
//   according to a format string.
// 
// Items in the format string can be in two forms:
//	1. "%C" --  print category corresponding 
//	2. "%F" -- found item
//  3. "%Innn" --- item in column #nnn in the current row. All three digits
//     must be present, as in %I000 for the left-most column, %I001, etc.
//
int R_File::rs_printfound(String* items, int itemsize, String* global_tags,
	int tagsize, int* Found, String* flist)
{
//	cout << "FOUND:   sizes are " << itemsize << ", " << tagsize << "\n";

int i, fslen, f, printed=0;
String format;

	for(i=0; i<itemsize && i<tagsize; i++)
	{
		if (Found[i])	//	(items[i].found)
		{
			/*************************
			 * Scan the format string
			 *************************/
			format = flist[i];
			fslen = format.length();

#ifdef DEBUG
			cout << " [ " << global_tags[i] << ": "
				 << items[i] /*.item */ << " ]\n";
#endif

			for(f=0; f < fslen; f++)
			{

				/** `%' substitution **/

				if (format[f] == '%' && ++f<fslen)
				{
					char c = format[f];	
					switch(c)
					{
						case 'C':	// current category
							cout << global_tags[i];
							printed++;
							break;
						case 'F':	// current item
							cout << items[i];
							printed++;
							break;
						case 'I':
						{
							String snum;
							int num;
							snum = format[++f];	// 10's digit
							if (++f < fslen)
								snum+= format[f];	// 1's digit
							num = atoi(snum.chars());	// convert to int
#ifdef DEBUG
							cout << "\n\t\tNumber is " << num << '\n';
#endif
							if (num >= 0 && num < itemsize) 	// verify in range
							{
								cout << items[num];	// print corresponding item
								printed++;
							}
						}
							break;

						default:
							cout << format[f];
							printed++;
					}	// switch

				}	// % subst.
				else
					cout << format[f];

			}	// for

		}	// found

		//	items[i].found = 0;
		Found[i] = 0;
	}
	return printed;
}

// s_printfound
// print the items that were found
void printfound(String* items, int itemsize, String* global_tags, int tagsize,
	int* Found)
{
	cout << "FOUND:\n";
	int i;
	for(i=0; i<itemsize && i<tagsize; i++)
	{
		if (Found[i])	//	(items[i].found)
		{
			cout << global_tags[i] << ": " << items[i] /*.item */ << '\n';
		}
		//	items[i].found = 0;
		Found[i] = 0;
	}
}
 
 