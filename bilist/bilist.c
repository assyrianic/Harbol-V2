#include "bilist.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolBiNode *harbol_binode_new(void *const data, const size_t datasize)
{
	struct HarbolBiNode *restrict node = harbol_alloc(1, sizeof *node);
	if( node != NULL ) {
		node->next = node->prev = NULL;
		node->data = harbol_alloc(datasize, sizeof *node->data);
		if( node->data==NULL )
			return NULL;
		else memcpy(node->data, data, datasize);
	}
	return node;
}

HARBOL_EXPORT bool harbol_binode_free(struct HarbolBiNode **const binoderef, void dtor(void**))
{
	if( *binoderef==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&(*binoderef)->data);
		
		harbol_free((*binoderef)->data), (*binoderef)->data=NULL;
		harbol_binode_free(&(*binoderef)->next, dtor);
		harbol_free(*binoderef), *binoderef = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_binode_set(struct HarbolBiNode *const restrict binode, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( binode->data==NULL ) {
		binode->data = harbol_alloc(datasize, sizeof *binode->data);
		return( binode->data==NULL ) ? false : memcpy(binode->data, data, datasize) != NULL;
	} else {
		return memcpy(binode->data, data, datasize) != NULL;
	}
}




HARBOL_EXPORT struct HarbolBiList *harbol_bilist_new(const size_t datasize)
{
	struct HarbolBiList *list = harbol_alloc(1, sizeof *list);
	if( list != NULL )
		*list = harbol_bilist_create(datasize);
	return list;
}

HARBOL_EXPORT struct HarbolBiList harbol_bilist_create(const size_t datasize)
{
	return (struct HarbolBiList){NULL, NULL, 0, datasize};
}

HARBOL_EXPORT bool harbol_bilist_clear(struct HarbolBiList *const list, void dtor(void**))
{
	harbol_binode_free(&list->head, dtor);
	list->tail = NULL;
	list->len = 0;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_free(struct HarbolBiList **const listref, void dtor(void**))
{
	if( *listref==NULL )
		return false;
	else {
		const bool res = harbol_bilist_clear(*listref, dtor);
		harbol_free(*listref), *listref=NULL;
		return res && *listref==NULL;
	}
}


HARBOL_EXPORT bool harbol_bilist_add_node_at_head(struct HarbolBiList *const list, struct HarbolBiNode *const node)
{
	if( list->head==NULL ) {
		list->head = list->tail = node;
	} else {
		// x(node)-> x(CurrHead)
		node->next = list->head;
		// NULL <-(node)-> x(CurrHead)
		node->prev = NULL;
		// NULL <-(node)-> <-(CurrHead)
		list->head->prev = node;
		// NULL <-(CurrHead)-> <-(OldHead)
		list->head = node;
	}
	list->len++;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_add_node_at_tail(struct HarbolBiList *const list, struct HarbolBiNode *const node)
{
	if( list->len>0 ) {
		// <-(CurrTail)x <-(node)x
		node->prev = list->tail;
		// <-(CurrTail)x <-(node)-> NULL
		node->next = NULL;
		// <-(CurrTail)-> <-(node)-> NULL
		list->tail->next = node;
		// <-(OldTail)-> <-(CurrTail)-> NULL
		list->tail = node;
	} else {
		list->head = list->tail = node;
	}
	list->len++;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_add_node_at_index(struct HarbolBiList *const list, struct HarbolBiNode *const node, const uindex_t index)
{
	if( list->head==NULL || index==0 )
		return harbol_bilist_add_node_at_head(list, node);
	// if index is out of bounds, append at tail end.
	else if( index >= list->len )
		return harbol_bilist_add_node_at_tail(list, node);
	else {
		const bool prev_dir = ( index >= list->len/2 );
		struct HarbolBiNode *curr = prev_dir ? list->tail : list->head;
		uindex_t i=prev_dir ? list->len-1 : 0;
		while( (prev_dir ? curr->prev != NULL : curr->next != NULL) && i != index ) {
			curr = prev_dir ? curr->prev : curr->next;
			prev_dir ? i-- : i++;
		}
		if( i>0 ) {
			// P-> <-(curr)
			// P-> x(node)x
			curr->prev->next = node;
			// P-> <-(node)x
			node->prev = curr->prev;
			// P-> <-(node)-> x(curr)
			node->next = curr;
			// P-> <-(node)-> <-(curr)
			curr->prev = node;
			list->len++;
			return true;
		}
		else return false;
	}
}


HARBOL_EXPORT bool harbol_bilist_insert_at_head(struct HarbolBiList *const restrict list, void *const restrict val)
{
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->datasize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_bilist_add_node_at_head(list, node);
			if( !result )
				harbol_binode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT bool harbol_bilist_insert_at_tail(struct HarbolBiList *const restrict list, void *restrict val)
{
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->datasize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_bilist_add_node_at_tail(list, node);
			if( !result )
				harbol_binode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT bool harbol_bilist_insert_at_index(struct HarbolBiList *const restrict list, void *const restrict val, uindex_t index)
{
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->datasize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_bilist_add_node_at_index(list, node, index);
			if( !result )
				harbol_binode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT struct HarbolBiNode *harbol_bilist_index_get_node(const struct HarbolBiList *const list, const uindex_t index)
{
	if( index==0 )
		return list->head;
	else if( index >= list->len )
		return list->tail;
	else {
		const bool prev_dir = ( index >= list->len/2 );
		struct HarbolBiNode *node = prev_dir ? list->tail : list->head;
		for( uindex_t i=prev_dir ? list->len-1 : 0; i<list->len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node;
			else node = prev_dir ? node->prev : node->next;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolBiNode *harbol_bilist_val_get_node(const struct HarbolBiList *restrict list, void *val)
{
	if( list->datasize==0 )
		return NULL;
	else {
		for( struct HarbolBiNode *n=list->head; n != NULL; n=n->next )
			if( !memcmp(n->data, val, list->datasize) )
				return n;
		return NULL;
	}
}

HARBOL_EXPORT void *harbol_bilist_get(const struct HarbolBiList *const list, const uindex_t index)
{
	if( index==0 && list->head != NULL )
		return list->head->data;
	else if( index >= list->len && list->tail != NULL )
		return list->tail->data;
	else {
		const bool prev_dir = ( index >= list->len/2 );
		struct HarbolBiNode *node = prev_dir ? list->tail : list->head;
		for( uindex_t i=prev_dir ? list->len-1 : 0; i<list->len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node->data;
			else node = prev_dir ? node->prev : node->next;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_bilist_set(struct HarbolBiList *const restrict list, const uindex_t index, void *const val)
{
	if( list->datasize==0 )
		return false;
	else if( index==0 && list->head != NULL )
		return harbol_binode_set(list->head, val, list->datasize);
	else if( index >= list->len && list->tail != NULL )
		return harbol_binode_set(list->tail, val, list->datasize);
	else {
		const bool prev_dir = ( index >= list->len/2 );
		struct HarbolBiNode *node = prev_dir ? list->tail : list->head;
		for( uindex_t i=prev_dir ? list->len-1 : 0; i<list->len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return harbol_binode_set(node, val, list->datasize);
			else node = prev_dir ? node->prev : node->next;
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_bilist_index_del(struct HarbolBiList *const list, const uindex_t index, void dtor(void**))
{
	struct HarbolBiNode *node = harbol_bilist_index_get_node(list, index);
	if( node==NULL )
		return false;
	else {
		node->prev ? (node->prev->next = node->next) : (list->head = node->next);
		node->next ? (node->next->prev = node->prev) : (list->tail = node->prev);
		
		if( dtor != NULL )
			dtor((void**)&node->data);
		harbol_free(node->data);
		harbol_free(node), node=NULL;
		list->len--;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bilist_node_del(struct HarbolBiList *const list, struct HarbolBiNode **const noderef, void dtor(void**))
{
	if( *noderef==NULL )
		return false;
	else {
		struct HarbolBiNode *node = *noderef;
		node->prev ? (node->prev->next = node->next) : (list->head = node->next);
		node->next ? (node->next->prev = node->prev) : (list->tail = node->prev);
		
		if( dtor != NULL )
			dtor((void**)&node->data);
		harbol_free(node->data);
		harbol_free(*noderef), *noderef=NULL;
		list->len--;
		return true;
	}
}
