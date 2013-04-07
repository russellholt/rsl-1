#ifndef RC_COMMAND_H_
#define RC_COMMAND_H_

#include "resource.h"

//#define rescall_allocation 14
//#define rescall_var_ref 15
//#define rescall_obj_var_ref 16
//


class rc_command : public resource {
public:
	enum command { NoOp=0, Return, Break, Continue, Exit };
	
	rc_command(void) { Init(); }
	rc_command(command c) { Init(); type = c; }

	command Type(void) { return type; }
	void Type(command c) { type = c; }
//	resource *& RetVal(void) { return retval; }

protected:	
	command type;
//	resource *retval;

private:
	Init(void)
	{
		class_name = "rc_command";
		hierarchy += "rc_command:";
		type = NoOp;
//		retval=NULL;
	}
};


#endif