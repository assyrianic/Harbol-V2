#include "unilist.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


HARBOL_EXPORT struct HarbolUniNode *harbol_uninode_new(void *const data, const size_t datasize)
{
	struct HarbolUniNode *restrict node = harbol_alloc(1, sizeof *node);
	if( node != NULL ) {
		node->next = NULL;
		node->data = harbol_alloc(datasize, sizeof *node->data);
		if( node->data==NULL )
			return NULL;
		else memcpy(node->data, data, datasize);
	}
	return node;
}

HARBOL_EXPORT bool harbol_uninode_free(struct HarbolUniNode **const uninoderef, void dtor(void**))
{
	if( *uninoderef==NULL )
		return false;
	else {
		if( dtor != NULL )
			dtor((void**)&(*uninoderef)->data);
		
		harbol_free((*uninoderef)->data), (*uninoderef)->data=NULL;
		harbol_uninode_free(&(*uninoderef)->next, dtor);
		harbol_free(*uninoderef), *uninoderef = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_uninode_set(struct HarbolUniNode *const restrict uninode, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( uninode->data==NULL ) {
		uninode->data = harbol_alloc(datasize, sizeof *uninode->data);
		return( uninode->data==NULL ) ? false : memcpy(uninode->data, data, datasize) != NULL;
	} else {
		return memcpy(uninode->data, data, datasize) != NULL;
	}
}




HARBOL_EXPORT struct HarbolUniList *harbol_unilist_new(const size_t datasize)
{
	struct HarbolUniList *list = harbol_alloc(1, sizeof *list);
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
	harbol_uninode_free(&list->head, dtor);
	list->tail = NULL;
	list->len = 0;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_free(struct HarbolUniList **const listref, void dtor(void**))
{
	if( *listref==NULL )
		return false;
	else {
		const bool res = harbol_unilist_clear(*listref, dtor);
		harbol_free(*listref), *listref=NULL;
		return res && *listref==NULL;
	}
}


HARBOL_EXPORT bool harbol_unilist_add_node_at_head(struct HarbolUniList *const list, struct HarbolUniNode *const node)
{
	node->next = list->head;
	list->head = node;
	if( list->tail==NULL )
		list->tail = node;
	list->len++;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_add_node_at_tail(struct HarbolUniList *const list, struct HarbolUniNode *const node)
{
	if( list->tail != NULL ) {
		list->tail->next = node;
		list->tail = node;
	}
	else list->head = list->tail = node;
	list->len++;
	return true;
}

HARBOL_EXPORT bool harbol_unilist_add_node_at_index(struct HarbolUniList *const list, struct HarbolUniNode *const node, const uindex_t index)
{
	if( list->head==NULL || index==0 )
		return harbol_unilist_add_node_at_head(list, node);
	// if index is out of bounds, append at tail end.
	else if( index >= list->len )
		return harbol_unilist_add_node_at_tail(list, node);
	
	struct HarbolUniNode
		*curr=list->head,
		*prev=NULL
	;
	uindex_t i=0;
	while( curr->next != NULL && i != index ) {
		prev = curr;
		curr = curr->next;
		i++;
	}
	if( i>0 ) {
		if( prev == list->tail )
			list->tail->next = node;
		else prev->next = node;
		node->next = curr;
		list->len++;
		return true;
	}
	return false;
}


HARBOL_EXPORT bool harbol_unilist_insert_at_head(struct HarbolUniList *const restrict list, void *const restrict val)
{
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->datasize);
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
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->datasize);
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
	if( list->datasize==0 )
		return false;
	else {
		struct HarbolUniNode *node = harbol_uninode_new(val, list->datasize);
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
		return list->head;
	else if( index >= list->len )
		return list->tail;
	else {
		struct HarbolUniNode *node = list->head;
		for( uindex_t i=0; i<list->len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node;
			else node = node->next;
		}
		return NULL;
	}
}

HARBOL_EXPORT struct HarbolUniNode *harbol_unilist_val_get_node(const struct HarbolUniList *restrict list, void *val)
{
	if( list->datasize==0 )
		return NULL;
	else {
		for( struct HarbolUniNode *n=list->head; n != NULL; n=n->next )
			if( !memcmp(n->data, val, list->datasize) )
				return n;
		return NULL;
	}
}

HARBOL_EXPORT void *harbol_unilist_get(const struct HarbolUniList *const list, const uindex_t index)
{
	if( index==0 && list->head != NULL )
		return list->head->data;
	else if( index >= list->len && list->tail != NULL )
		return list->tail->data;
	else {
		struct HarbolUniNode *node = list->head;
		for( uindex_t i=0; i<list->len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return node->data;
			else node = node->next;
		}
		return NULL;
	}
}

HARBOL_EXPORT bool harbol_unilist_set(struct HarbolUniList *const restrict list, const uindex_t index, void *const val)
{
	if( list->datasize==0 )
		return false;
	else if( index==0 && list->head != NULL )
		return harbol_uninode_set(list->head, val, list->datasize);
	else if( index >= list->len && list->tail != NULL )
		return harbol_uninode_set(list->tail, val, list->datasize);
	else {
		struct HarbolUniNode *node = list->head;
		for( uindex_t i=0; i<list->len; i++ ) {
			if( node==NULL )
				break;
			else if( i==index )
				return harbol_uninode_set(node, val, list->datasize);
			else node = node->next;
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
		if( node==list->head )
			list->head = node->next;
		else {
			struct HarbolUniNode *travnode = list->head;
			for( uindex_t i=0; i<list->len; i++ ) {
				if( travnode->next == node ) {
					if( list->tail == node ) {
						travnode->next = NULL;
						list->tail = travnode;
					}
					else travnode->next = node->next;
					break;
				}
				travnode = travnode->next;
			}
		}
		
		if( dtor != NULL )
			dtor((void**)&node->data);
		harbol_free(node->data);
		harbol_free(node); node=NULL;
		
		list->len--;
		if( !list->len && list->tail != NULL )
			list->tail = NULL;
		return true;
	}
}

HARBOL_EXPORT bool harbol_unilist_node_del(struct HarbolUniList *const list, struct HarbolUniNode **const noderef, void dtor(void**))
{
	if( *noderef==NULL )
		return false;
	else {
		struct HarbolUniNode *node = *noderef;
		if( node==list->head )
			list->head = node->next;
		else {
			struct HarbolUniNode *travnode = list->head;
			for( uindex_t i=0; i<list->len; i++ ) {
				if( travnode->next == node ) {
					if( list->tail == node ) {
						travnode->next = NULL;
						list->tail = travnode;
					}
					else travnode->next = node->next;
					break;
				}
				else travnode = travnode->next;
			}
		}
		
		if( dtor != NULL )
			dtor((void**)&node->data);
		harbol_free(node->data);
		
		harbol_free(*noderef); *noderef=NULL;
		list->len--;
		return true;
	}
}
