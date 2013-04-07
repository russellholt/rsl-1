#ifndef _R2_H_
#define _R2_H_

#define TYPE_RESCALL 0
#define TYPE_STRING 1
#define TYPE_VAR_REF 2
#define TYPE_DECL 3
#define TYPE_FILE 4
#define TYPE_EVAL 5
#define TYPE_INT 6
#define TYPE_RESTABLE 7
#define TYPE_SELECT 8
#define TYPE_BOOLEAN 9
#define TYPE_FLOAT 10
#define TYPE_HEXVAL 11
#define TYPE_POINTER 12
#define TYPE_OBJ_VAR_REF 13
#define TYPE_LOOP 14
#define TYPE_RETURN 15
#define TYPE_BREAK 16
#define TYPE_CONTINUE 17
#define TYPE_EXIT 18

typedef struct res_call {
	struct res_call *next;
	int type;
	char *res_name;	/* name or value, based on type */
	char *method_name;	/* null if type VAR_REF or STRING */
	struct res_call *args;
	struct res_call *eval;	/* pointer to rescall to evaluate */
	struct res_call *extra;
} res_call;

#endif
