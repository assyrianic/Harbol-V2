#include "tree.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

HARBOL_EXPORT struct HarbolTree *harbol_tree_new(void *const restrict val, const size_t datasize)
{
	struct HarbolTree *tree = harbol_alloc(1, sizeof *tree);
	if( tree != NULL )
		*tree = harbol_tree_create(val, datasize);
	return tree;
}

HARBOL_EXPORT struct HarbolTree harbol_tree_create(void *const restrict val, const size_t datasize)
{
	struct HarbolTree tree = EMPTY_HARBOL_TREE;
	tree.data = harbol_alloc(datasize, sizeof *tree.data);
	if( tree.data != NULL ) {
		memcpy(tree.data, val, datasize);
		tree.children = harbol_vector_create(sizeof tree, VEC_DEFAULT_SIZE);
	}
	return tree;
}

HARBOL_EXPORT bool harbol_tree_clear(struct HarbolTree *const tree, void dtor(void**))
{
	if( dtor != NULL )
		dtor((void**)&tree->data);
	if( tree->data != NULL )
		harbol_free(tree->data), tree->data=NULL;
	
	for( uindex_t i=tree->children.count - 1; i<tree->children.count; i-- ) {
		struct HarbolTree *const child = harbol_tree_index_get_child(tree, i);
		harbol_tree_clear(child, dtor);
	}
	harbol_vector_clear(&tree->children, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_tree_free(struct HarbolTree **const treeref, void dtor(void**))
{
	if( *treeref==NULL )
		return false;
	else {
		const bool res = harbol_tree_clear(*treeref, dtor);
		harbol_free(*treeref), *treeref=NULL;
		return res;
	}
}

HARBOL_EXPORT void *harbol_tree_get(const struct HarbolTree *const tree)
{
	return tree->data;
}

HARBOL_EXPORT bool harbol_tree_set(struct HarbolTree *const restrict tree, void *const restrict val, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( tree->data==NULL ) {
		tree->data = harbol_alloc(datasize, sizeof *tree->data);
		return( tree->data==NULL ) ? false : memcpy(tree->data, val, datasize) != NULL;
	}
	else return memcpy(tree->data, val, datasize) != NULL;
}

HARBOL_EXPORT bool harbol_tree_insert_child_val(struct HarbolTree *const restrict tree, void *const restrict val, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else {
		struct HarbolTree node = harbol_tree_create(val, datasize);
		return( node.data==NULL ) ? !harbol_tree_clear(&node, NULL) : harbol_vector_insert(&tree->children, &node);
	}
}

HARBOL_EXPORT bool harbol_tree_insert_child_node(struct HarbolTree *const tree, struct HarbolTree *const child)
{
	return( child->data==NULL ) ? false : harbol_vector_insert(&tree->children, child);
}

HARBOL_EXPORT bool harbol_tree_del_child_node(struct HarbolTree *const tree, struct HarbolTree *const child, void dtor(void**))
{
	const index_t i = harbol_vector_index_of(&tree->children, child, 0);
	if( i<0 )
		return false;
	else {
		harbol_tree_clear(child, dtor);
		harbol_vector_del(&tree->children, i, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_tree_del_child_index(struct HarbolTree *const tree, const uindex_t index, void dtor(void**))
{
	struct HarbolTree *const child = harbol_vector_get(&tree->children, index);
	if( child==NULL )
		return false;
	else {
		harbol_tree_clear(child, dtor);
		harbol_vector_del(&tree->children, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_tree_del_child_val(struct HarbolTree *const restrict tree, void *const restrict val, const size_t datasize, void dtor(void**))
{
	if( datasize==0 )
		return false;
	else {
		for( uindex_t i=0; i<tree->children.count; i++ ) {
			struct HarbolTree *const child = harbol_vector_get(&tree->children, i);
			if( !memcmp(child->data, val, datasize) ) {
				harbol_tree_clear(child, dtor);
				harbol_vector_del(&tree->children, i, NULL);
				return true;
			}
		}
		return false;
	}
}

HARBOL_EXPORT struct HarbolTree *harbol_tree_index_get_child(const struct HarbolTree *const tree, const uindex_t index)
{
	return harbol_vector_get(&tree->children, index);
}

HARBOL_EXPORT struct HarbolTree *harbol_tree_val_get_child(const struct HarbolTree *const restrict tree, void *const restrict val, const size_t datasize)
{
	if( datasize==0 )
		return NULL;
	else {
		for( uindex_t i=0; i<tree->children.count; i++ ) {
			struct HarbolTree *const restrict child = harbol_vector_get(&tree->children, i);
			if( !memcmp(child->data, val, datasize) )
				return child;
		}
		return NULL;
	}
}
