#include "unilist.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolUniNode *harbol_uninode_new(void *const data, const size_t datasize)
{
	struct HarbolUniNode *restrict node = calloc(1, sizeof *node);
	if( node != NULL ) {
		node->Next = NULL;
		node->Data = calloc(datasize, sizeof *node->Data);
		if( node->Data==NULL )
			return NULL;
		else memcpy(node->Data, data, datasize);
	}
	return node;
}

HARBOL_EXPORT bool harbol_uninode_free(struct HarbolUniNode **const uninoderef, void dtor(void**))
{
	if( *uninoderef==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&(*uninoderef)->Data);
		
		free((*uninoderef)->Data), (*uninoderef)->Data=NULL;
		harbol_uninode_free(&(*uninoderef)->Next, dtor);
		free(*uninoderef), *uninoderef = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_uninode_set(struct HarbolUniNode *const restrict uninode, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( uninode->Data==NULL ) {
		uninode->Data = calloc(datasize, sizeof *uninode->Data);
		return( uninode->Data==NULL ) ? false : memcpy(uninode->Data, data, datasize) != NULL;
	} else {
		return memcpy(uninode->Data, data, datasize) != NULL;
	}
}




HARBOL_EXPORT struct HarbolUniList *harbol_unilist_new(const size_t datasize)
{
	struct HarbolUniList *list = calloc(1, sizeof *list);
	if( list != NULL )
		*list = harbol_unilist_create(datasize);
	return list;
}

HARBOL_EXPORT struct HarbolUniList harbol_unilist_create(const size_t datasize)
{
	return (struct HarbolUniList){NULL, NULL, 0, datasize};
}

HARBOL_EXPORT bool harbol_unilist_clear(struct HarbolUniList *const list, void dtor(void**))
{
	harbol_uninode_free(&list->Head, dtor);
	list->Tail = NULL;
	list->Len = 0;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_free(struct HarbolUniList **const listref, void dtor(void**))
{
	if( *listref==NULL )
		return false;
	else {
		const bool res = harbol_unilist_clear(*listref, dtor);
		free(*listref), *listref=NULL;
		return res && *listref==NULL;
	}
}


HARBOL_EXPORT bool harbol_unilist_add_node_at_head(struct HarbolUniList *const list, struct HarbolUniNode *const node)
{
	node->Next = list->Head;
	list->Head = node;
	if( list->Tail==NULL )
		list->Tail = node;
	list->Len++;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_add_node_at_tail(struct HarbolUniList *const list, struct HarbolUniNode *const node)
{
	if( list->Head != NULL ) {
		node->Next = NULL;
		list->Tail->Next = node;
		list->Tail = node;
	}
	else list->Head = list->Tail = node;
	list->Len++;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_add_node_at_index(struct HarbolUniList *const list, struct HarbolUniNode *const node, const uindex_t index)
{
	if( list->Head==NULL || index==0 )
		return harbol_unilist_add_node_at_head(list, node);
	// if index is out of bounds, append at tail end.
	else if( index >= list->Len )
		return harbol_unilist_add_node_at_tail(list, node);
	
	struct HarbolUniNode
		*curr=list->Head,
		*prev=NULL
	;
	uindex_t i=0;
	while( curr->Next != NULL && i != index ) {
		prev = curr;
		curr = curr->Next;
		i++;
	}
	if( i>0 ) {
		if( prev == list->Tail )
			list->Tail->Next = node;
		else prev->Next = node;
		node->Next = curr;
		list->Len++;
		return true;
	}
	return false;
}


HARBOL_EXPORT bool harbol_unilist_insert_at_head(struct HarbolUniList *const restrict list, void *const restrict val)
{
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->DataSize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_unilist_add_node_at_head(list, node);
			if( !result )
				harbol_uninode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT bool harbol_unilist_insert_at_tail(struct HarbolUniList *const restrict list, void *restrict val)
{
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->DataSize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_unilist_add_node_at_tail(list, node);
			if( !result )
				harbol_uninode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT bool harbol_unilist_insert_at_index(struct HarbolUniList *const restrict list, void *const restrict val, const uindex_t index)
{
	if( list->DataSize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->DataSize);
		if( node==NULL )
			return false;
		else {
			const bool result = harbol_unilist_add_node_at_index(list, node, index);
			if( !result )
				harbol_uninode_free(&node, NULL);
			return result;
		}
	}
}

HARBOL_EXPORT struct HarbolUniNode *harbol_unilist_index_get_node(const struct HarbolUniList *const list, const uindex_t index)
{
	if( index==0 )
		return list->Head;
	else if( index >= list->Len )
		return list->Tail;
	else {
		struct HarbolUniNode *node = list->Head;
		for( uindex_t i=0; i<list->Len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node;
			else node = node->Next;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolUniNode *harbol_unilist_val_get_node(const struct HarbolUniList *restrict list, void *val)
{
	if( list->DataSize==0 )
		return NULL;
	else {
		for( struct HarbolUniNode *n=list->Head; n != NULL; n=n->Next )
			if( !memcmp(n->Data, val, list->DataSize) )
				return n;
		return NULL;
	}
}

HARBOL_EXPORT void *harbol_unilist_get(const struct HarbolUniList *const list, const uindex_t index)
{
	if( index==0 && list->Head != NULL )
		return list->Head->Data;
	else if( index >= list->Len && list->Tail != NULL )
		return list->Tail->Data;
	else {
		struct HarbolUniNode *node = list->Head;
		for( uindex_t i=0; i<list->Len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node->Data;
			else node = node->Next;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_unilist_set(struct HarbolUniList *const restrict list, const uindex_t index, void *const val)
{
	if( list->DataSize==0 )
		return false;
	else if( index==0 && list->Head != NULL )
		return harbol_uninode_set(list->Head, val, list->DataSize);
	else if( index >= list->Len && list->Tail != NULL )
		return harbol_uninode_set(list->Tail, val, list->DataSize);
	else {
		struct HarbolUniNode *node = list->Head;
		for( uindex_t i=0; i<list->Len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return harbol_uninode_set(node, val, list->DataSize);
			else node = node->Next;
		}
		return false;
	}
}

HARBOL_EXPORT bool harbol_unilist_index_del(struct HarbolUniList *const list, const uindex_t index, void dtor(void**))
{
	struct HarbolUniNode *node = harbol_unilist_index_get_node(list, index);
	if( node==NULL )
		return false;
	else {
		if( node==list->Head )
			list->Head = node->Next;
		else {
			struct HarbolUniNode *travnode = list->Head;
			for( uindex_t i=0; i<list->Len; i++ ) {
				if( travnode->Next == node ) {
					if( list->Tail == node ) {
						travnode->Next = NULL;
						list->Tail = travnode;
					}
					else travnode->Next = node->Next;
					break;
				}
				travnode = travnode->Next;
			}
		}
		
		if( dtor != NULL )
			dtor((void**)&node->Data);
		free(node->Data);
		free(node); node=NULL;
		
		list->Len--;
		if( !list->Len && list->Tail != NULL )
			list->Tail = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_unilist_node_del(struct HarbolUniList *const list, struct HarbolUniNode **const noderef, void dtor(void**))
{
	if( *noderef==NULL )
		return false;
	else {
		struct HarbolUniNode *node = *noderef;
		if( node==list->Head )
			list->Head = node->Next;
		else {
			struct HarbolUniNode *travnode = list->Head;
			for( uindex_t i=0; i<list->Len; i++ ) {
				if( travnode->Next == node ) {
					if( list->Tail == node ) {
						travnode->Next = NULL;
						list->Tail = travnode;
					}
					else travnode->Next = node->Next;
					break;
				}
				else travnode = travnode->Next;
			}
		}
		
		if( dtor != NULL )
			dtor((void**)&node->Data);
		free(node->Data);
		
		free(*noderef); *noderef=NULL;
		list->Len--;
		return true;
	}
}
