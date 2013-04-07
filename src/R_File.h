/* R_File
 *
 * $Id: R_File.h,v 1.3 1996/03/29 21:42:26 holtrf Exp holtrf $
 *
 * Russell Holt, Sept 12 1995
 */

#ifndef _R_FILE_H_
#define _R_FILE_H_

#include <SLList.h>
#include <String.h>

#include <fstream.h>
#include "resource.h"
#include "restable.h"
//#include "rFile.h"

class R_File : public resource {
public:
	enum state { closed, open_read, open_write, open_append };
	enum oper { op_or=0, op_and=1 };

    R_File(void) { Init(); name = "File"; }
    R_File(String& nm) { Init(); name = nm; }
    R_File(char *nm) { Init(); name = nm; }

    int Open(state newstate);
	void Open(SLList<resource *> &args);
	void Close(void);

	void Write(SLList<resource *> &args, state st, int inlist=0);

    void SetFilename(String &nm) { filename = nm; }
    void SetFilename(char *nm) { filename = nm; }
	inline String Filename(void) { return filename; }
	
	void PrintFiles(SLList<resource *> &args);
	void PrintFile(String fname="");
	
	resource *execute(String &method, SLList<resource *> &args);
	resource *Create(String &nm, resource*& table);

	String Value(void)  { return filename; }
	resource *_read_(resource *thearg);

	/* searching functions */
	resource *R_File::SearchFile(SLList<resource *>& args, R_File::oper Op);

	int s_Match(String* items, int nitems, String* items2,
			int nitems2, int matchcase, int contains, int *Found);
	void s_Read(ifstream& in, String& s);
	void s_searchfile(ifstream&, String*, oper, int col_width,
		int matchcase, restable *format_list, String sep, int contains);
	int rs_printfound(String* items, int itemsize, String* global_tags, int tagsize,
		int* Found, String* format);

private:
	void Init(void)
		{ class_name = "File"; /* the_stream = NULL;*/ stream_state = closed; }
protected:
	String filename;
	state stream_state;	// see "enum state {...}" above
	fstream the_stream;
};

#endif
