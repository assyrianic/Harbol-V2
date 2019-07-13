#include "bilist.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolBiNode *harbol_binode_new(void *const data, const size_t datasize)
{
	struct HarbolBiNode *restrict node = calloc(1, sizeof *node);
	if( node != NULL ) {
		node->Next = node->Prev = NULL;
		node->Data = calloc(datasize, sizeof *node->Data);
		if( node->Data==NULL )
			return NULL;
		else memcpy(node->Data, data, datasize);
	}
	return node;
}

HARBOL_EXPORT bool harbol_binode_free(struct HarbolBiNode **const binoderef, void dtor(void**))
{
	if( *binoderef==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&(*binoderef)->Data);
		
		free((*binoderef)->Data), (*binoderef)->Data=NULL;
		harbol_binode_free(&(*binoderef)->Next, dtor);
		free(*binoderef), *binoderef = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_binode_set(struct HarbolBiNode *const restrict binode, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( binode->Data==NULL ) {
		binode->Data = calloc(datasize, sizeof *binode->Data);
		return( binode->Data==NULL ) ? false : memcpy(binode->Data, data, datasize) != NULL;
	} else {
		return memcpy(binode->Data, data, datasize) != NULL;
	}
}




HARBOL_EXPORT struct HarbolBiList *harbol_bilist_new(const size_t datasize)
{
	struct HarbolBiList *list = calloc(1, sizeof *list);
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
	harbol_binode_free(&list->Head, dtor);
	list->Tail = NULL;
	list->Len = 0;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_free(struct HarbolBiList **const listref, void dtor(void**))
{
	if( *listref==NULL )
		return false;
	else {
		const bool res = harbol_bilist_clear(*listref, dtor);
		free(*listref), *listref=NULL;
		return res && *listref==NULL;
	}
}


HARBOL_EXPORT bool harbol_bilist_add_node_at_head(struct HarbolBiList *const list, struct HarbolBiNode *const node)
{
	if( list->Head==NULL ) {
		list->Head = list->Tail = node;
	} else {
		// x(node)-> x(CurrHead)
		node->Next = list->Head;
		// NULL <-(node)-> x(CurrHead)
		node->Prev = NULL;
		// NULL <-(node)-> <-(CurrHead)
		list->Head->Prev = node;
		// NULL <-(CurrHead)-> <-(OldHead)
		list->Head = node;
	}
	list->Len++;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_add_node_at_tail(struct HarbolBiList *const list, struct HarbolBiNode *const node)
{
	if( list->Len>0 ) {
		// <-(CurrTail)x <-(node)x
		node->Prev = list->Tail;
		// <-(CurrTail)x <-(node)-> NULL
		node->Next = NULL;
		// <-(CurrTail)-> <-(node)-> NULL
		list->Tail->Next = node;
		// <-(OldTail)-> <-(CurrTail)-> NULL
		list->Tail = node;
	} else {
		list->Head = list->Tail = node;
	}
	list->Len++;
	return true;
}

HARBOL_EXPORT bool harbol_bilist_add_node_at_index(struct HarbolBiList *const list, struct HarbolBiNode *const node, const uindex_t index)
{
	if( list->Head==NULL || index==0 )
		return harbol_bilist_add_node_at_head(list, node);
	// if index is out of bounds, append at tail end.
	else if( index >= list->Len )
		return harbol_bilist_add_node_at_tail(list, node);
	else {
		const bool prev_dir = ( index >= list->Len/2 );
		struct HarbolBiNode *curr = prev_dir ? list->Tail : list->Head;
		uindex_t i=prev_dir ? list->Len-1 : 0;
		while( (prev_dir ? curr->Prev != NULL : curr->Next != NULL) && i != index ) {
			curr = prev_dir ? curr->Prev : curr->Next;
			prev_dir ? i-- : i++;
		}
		if( i>0 ) {
			// P-> <-(curr)
			// P-> x(node)x
			curr->Prev->Next = node;
			// P-> <-(node)x
			node->Prev = curr->Prev;
			// P-> <-(node)-> x(curr)
			node->Next = curr;
			// P-> <-(node)-> <-(curr)
			curr->Prev = node;
			list->Len++;
			return true;
		}
		else return false;
	}
}


HARBOL_EXPORT bool harbol_bilist_insert_at_head(struct HarbolBiList *const restrict list, void *const restrict val)
{
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->DataSize);
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
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->DataSize);
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
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolBiNode *node = harbol_binode_new(val, list->DataSize);
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
		return list->Head;
	else if( index >= list->Len )
		return list->Tail;
	else {
		const bool prev_dir = ( index >= list->Len/2 );
		struct HarbolBiNode *node = prev_dir ? list->Tail : list->Head;
		for( uindex_t i=prev_dir ? list->Len-1 : 0; i<list->Len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node;
			else node = prev_dir ? node->Prev : node->Next;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolBiNode *harbol_bilist_val_get_node(const struct HarbolBiList *restrict list, void *val)
{
	if( list->DataSize==0 )
		return NULL;
	else {
		for( struct HarbolBiNode *n=list->Head; n != NULL; n=n->Next )
			if( !memcmp(n->Data, val, list->DataSize) )
				return n;
		return NULL;
	}
}

HARBOL_EXPORT void *harbol_bilist_get(const struct HarbolBiList *const list, const uindex_t index)
{
	if( index==0 && list->Head != NULL )
		return list->Head->Data;
	else if( index >= list->Len && list->Tail != NULL )
		return list->Tail->Data;
	else {
		const bool prev_dir = ( index >= list->Len/2 );
		struct HarbolBiNode *node = prev_dir ? list->Tail : list->Head;
		for( uindex_t i=prev_dir ? list->Len-1 : 0; i<list->Len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node->Data;
			else node = prev_dir ? node->Prev : node->Next;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_bilist_set(struct HarbolBiList *const restrict list, const uindex_t index, void *const val)
{
	if( list->DataSize==0 )
		return false;
	else if( index==0 && list->Head != NULL )
		return harbol_binode_set(list->Head, val, list->DataSize);
	else if( index >= list->Len && list->Tail != NULL )
		return harbol_binode_set(list->Tail, val, list->DataSize);
	else {
		const bool prev_dir = ( index >= list->Len/2 );
		struct HarbolBiNode *node = prev_dir ? list->Tail : list->Head;
		for( uindex_t i=prev_dir ? list->Len-1 : 0; i<list->Len; prev_dir ? i-- : i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return harbol_binode_set(node, val, list->DataSize);
			else node = prev_dir ? node->Prev : node->Next;
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
		node->Prev ? (node->Prev->Next = node->Next) : (list->Head = node->Next);
		node->Next ? (node->Next->Prev = node->Prev) : (list->Tail = node->Prev);
		
		if( dtor != NULL )
			dtor((void**)&node->Data);
		free(node->Data);
		free(node), node=NULL;
		list->Len--;
		return true;
	}
}

HARBOL_EXPORT bool harbol_bilist_node_del(struct HarbolBiList *const list, struct HarbolBiNode **const noderef, void dtor(void**))
{
	if( *noderef==NULL )
		return false;
	else {
		struct HarbolBiNode *node = *noderef;
		node->Prev ? (node->Prev->Next = node->Next) : (list->Head = node->Next);
		node->Next ? (node->Next->Prev = node->Prev) : (list->Tail = node->Prev);
		
		if( dtor != NULL )
			dtor((void**)&node->Data);
		free(node->Data);
		free(*noderef), *noderef=NULL;
		list->Len--;
		return true;
	}
}
