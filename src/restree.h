#ifndef RESTREE
#define RESTREE

#include "resource.h"
#include <String.h>
#include <SLList.h>

class restree {
	
/*************************** PUBLIC ********************************/
public:
	enum result_code { error=-1, notfound=0, found=1 };
	enum order { descend=0, ascend };

	restree(void);
	restree(restree *r);
	
	result_code Insert(resource *r, order theOrder=ascend, String fieldname="");
	result_code Insert(resource *r, String fieldname);
	resource *FindByName(String what);
	void GetList(SLList<resource *>& thelist);
	void print(void);

/************************** PROTECTED ******************************/
protected:
	struct node {
		resource *item;
		node *left, *right;
		node(void) { left=right=NULL; }
		node(resource *r) { item = r; left=right=NULL; }
	};
	typedef node* nodePtr;	// pointer to node
	typedef nodePtr& nodePtrRef;	// reference to pointer to node
	
	node *root;
	int count;

	void AddAt(resource *r, nodePtrRef n);
	result_code Insert(nodePtrRef where, resource *r, resource *rval,
		String fieldname, order ascending);
	void InOrder(nodePtrRef pnode);
	void GetList(nodePtrRef pnode, SLList<resource *>& thelist);
	resource *Find(nodePtrRef where, String what);

};

#endif

