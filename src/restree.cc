#include "restree.h"
#include "destiny.h"

// restree basic constructor
restree::restree(void)
{
	root = NULL;
	count = 0;
}

// restree tree-building/assignment constructor
restree::restree(restree *r)
{
	cout << "restree(restree *) undefined constructor.";
}

// Insert
// external entrance version (no specified node: begins at root.)
//
// - insert a resource into the tree
// - `method' determines the data member of `res' to be used as the
//   comparison key. A zero length string indicates that `res' itself
//   is to be used as the comparison key.
// - `ascending' determines which direction the insertion in the tree is done.
//   This is handled here rather than in a traversal order because it is
//   expected that in the future Insert will handle comparison by multiple fields,
//   each of which may ascend or descend. 1 = ascending order, 0 = descending
//
// - method is default "", order is default `ascend'
//
restree::result_code restree::Insert(resource* res, restree::order theOrder, String method)
{
	return Insert(root, res, NULL, method, theOrder);	
}

// Same as above, except order is `ascend'.
restree::result_code restree::Insert(resource* res, String method)
{
	return Insert(root, res, NULL, method, ascend);	
}

// Insert
// Adds a node to the tree
//
restree::result_code restree::Insert(nodePtrRef where, resource* res, resource* resval,
	String method, restree::order theOrder)
{
	if (!res)
		return error;
		
	SLList<resource *> argl;	// a zero-length argument list

	if (!resval)
	{
		if (method.length() == 0)
			// the value to compare with is the resource itself, ie an Integer
			resval = res;	
		else
			resval = res->execute(method, argl);	// get the value to compare with.
	}
	if (!resval)	// nothing to compare with!
		return error;

	if (!where)
	{
		AddAt(res, where);
		return found;
	}
	
	/*---------------------------------------------------------------*
	 * `datamember' is from the resource in the `where' node. as if, *
	 * in RSL, the expression "var.fieldname()", if `var' is the     *
	 * resource in the node in question (`where')                    *
	 *---------------------------------------------------------------*/

	resource* datamember = NULL;
	if (method.length() == 0)
		datamember = where->item;	// no method, so it's the resource itself.
	else
		datamember = where->item->execute(method, argl);
	nodePtrRef lnode = where->left;	// left child
	nodePtrRef rnode = where->right;	// right child
	int lessthan=0;

#ifdef DEBUG
	cout << "Comparing \"" << resval->Value() << "\" and \"" << datamember->Value() << "\":\n";
#endif
	/* compare the node data member*/
	resource *truth = resval->LessThan(datamember);	// NULL datamember OK
	if (truth && truth->LogicalValue())
	{
		lessthan=1;

#ifdef DEBUG
		cout << " ---> Less than\n";
#endif

	}
	
#ifdef DEBUG
	else
		cout << " ---> Not less than\n";
#endif
	
	
	
	if ((theOrder == ascend && lessthan) || (theOrder == descend && !lessthan))
		return Insert(lnode, res, resval, method, theOrder);
	else	// equal or greater values to the right
		return Insert(rnode, res, resval, method, theOrder);

	return error;	// will not get here.
}

// AddAt
// Add the given resource as the item for the given node.
void restree::AddAt(resource * r, nodePtrRef n)
{
	if (n == NULL)
	{
		n = new node(r);
		count++;
	}
	else
		n->item = r;	// shouldn't happen
}

// GetList (public)
// interface to the private method, starting at the root node.
void restree::GetList(SLList<resource *>& thelist)
{	
	GetList(root, thelist);
}

// GetList (private)
// Depth first traversal, adding each node to the given list.
void restree::GetList(nodePtrRef pnode, SLList<resource *>& thelist)
{
	if (!pnode)
		return;
	
	if (pnode->left)
		GetList(pnode->left, thelist);	// recurse left subtree

	if (pnode->item)	// parent node
		thelist.append(pnode->item);	// add the node item to the list!

	if (pnode->right)
		GetList(pnode->right, thelist);	// recurse right subtree
}

void restree::print(void)
{
	InOrder(root);
}

void restree::InOrder(nodePtrRef pnode)	// , SLList<resource *r>& newlist)
{
	if (!pnode)
		return;
	
	InOrder(pnode->left);	// left child (subtree)

	if (pnode->item)	// parent node
		pnode->item->print();	// will be: thelist.append(pnode->item);

	InOrder(pnode->right);	// right child (subtree)
}


resource *restree::FindByName(String what)
{
	return Find(root, what);
}

// Find
// Search the tree for the named item.
// There will be other variations soon, like comparison by value, pointer,
// result of method call, etc.
// This version is intended mainly to be used from inside the RSL system
// itself (C++) where searching by name is most useful. The more
// general Find, complement to the general Insert, will be added later.
// RFH March 12, 1996
resource *restree::Find(nodePtrRef where, String what)
{
	if (!where)
		return NULL;	// not found.
		
	resource *comparewith = where->item;
	
	if (comparewith == NULL)
		return NULL;	// error....
		
	if (what == comparewith->Name())
		return comparewith;	// found
			
	if (what < comparewith->Name())
		return (where->left)? Find(where->left, what) : NULL;
	else
		return (where->right)? Find(where->right, what) : NULL;
}



