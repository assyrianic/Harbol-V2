#ifndef HARBOL_UNILIST_INCLUDED
#	define HARBOL_UNILIST_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

struct HarbolUniNode {
	uint8_t *Data;
	struct HarbolUniNode *Next;
};

HARBOL_EXPORT NO_NULL struct HarbolUniNode *harbol_uninode_new(void *data, size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_uninode_free(struct HarbolUniNode **uninoderef, void dtor(void**));
HARBOL_EXPORT NO_NULL bool harbol_uninode_set(struct HarbolUniNode *uninode, void *data, size_t datasize);


struct HarbolUniList {
	struct HarbolUniNode *Head, *Tail;
	size_t Len, DataSize;
};


HARBOL_EXPORT struct HarbolUniList *harbol_unilist_new(size_t datasize);
HARBOL_EXPORT struct HarbolUniList harbol_unilist_create(size_t datasize);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_unilist_clear(struct HarbolUniList *list, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_unilist_free(struct HarbolUniList **listref, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_unilist_add_node_at_head(struct HarbolUniList *list, struct HarbolUniNode *node);
HARBOL_EXPORT NO_NULL bool harbol_unilist_add_node_at_tail(struct HarbolUniList *list, struct HarbolUniNode *node);
HARBOL_EXPORT NO_NULL bool harbol_unilist_add_node_at_index(struct HarbolUniList *list, struct HarbolUniNode *node, uindex_t index);

HARBOL_EXPORT NO_NULL bool harbol_unilist_insert_at_head(struct HarbolUniList *list, void *val);
HARBOL_EXPORT NO_NULL bool harbol_unilist_insert_at_tail(struct HarbolUniList *list, void *val);
HARBOL_EXPORT NO_NULL bool harbol_unilist_insert_at_index(struct HarbolUniList *list, void *val, uindex_t index);

HARBOL_EXPORT NO_NULL struct HarbolUniNode *harbol_unilist_index_get_node(const struct HarbolUniList *list, uindex_t index);
HARBOL_EXPORT NO_NULL struct HarbolUniNode *harbol_unilist_val_get_node(const struct HarbolUniList *list, void *val);

HARBOL_EXPORT NO_NULL void *harbol_unilist_get(const struct HarbolUniList *list, uindex_t index);
HARBOL_EXPORT NO_NULL bool harbol_unilist_set(struct HarbolUniList *list, uindex_t index, void *val);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_unilist_index_del(struct HarbolUniList *list, uindex_t index, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_unilist_node_del(struct HarbolUniList *list, struct HarbolUniNode **noderef, void dtor(void**));
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_UNILIST_INCLUDED */
