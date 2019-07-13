#ifndef HARBOL_BILIST_INCLUDED
#	define HARBOL_BILIST_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HarbolBiNode {
	uint8_t *Data;
	struct HarbolBiNode *Next, *Prev;
};

HARBOL_EXPORT NO_NULL struct HarbolBiNode *harbol_binode_new(void *data, size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_binode_free(struct HarbolBiNode **binoderef, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_binode_set(struct HarbolBiNode *binode, void *data, size_t datasize);


struct HarbolBiList {
	struct HarbolBiNode *Head, *Tail;
	size_t Len, DataSize;
};


HARBOL_EXPORT struct HarbolBiList *harbol_bilist_new(size_t datasize);
HARBOL_EXPORT struct HarbolBiList harbol_bilist_create(size_t datasize);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_bilist_clear(struct HarbolBiList *list, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_bilist_free(struct HarbolBiList **listref, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_bilist_add_node_at_head(struct HarbolBiList *list, struct HarbolBiNode *node);
HARBOL_EXPORT NO_NULL bool harbol_bilist_add_node_at_tail(struct HarbolBiList *list, struct HarbolBiNode *node);
HARBOL_EXPORT NO_NULL bool harbol_bilist_add_node_at_index(struct HarbolBiList *list, struct HarbolBiNode *node, uindex_t index);

HARBOL_EXPORT NO_NULL bool harbol_bilist_insert_at_head(struct HarbolBiList *list, void *val);
HARBOL_EXPORT NO_NULL bool harbol_bilist_insert_at_tail(struct HarbolBiList *list, void *val);
HARBOL_EXPORT NO_NULL bool harbol_bilist_insert_at_index(struct HarbolBiList *list, void *val, uindex_t index);

HARBOL_EXPORT NO_NULL struct HarbolBiNode *harbol_bilist_index_get_node(const struct HarbolBiList *list, uindex_t index);
HARBOL_EXPORT NO_NULL struct HarbolBiNode *harbol_bilist_val_get_node(const struct HarbolBiList *list, void *val);

HARBOL_EXPORT NO_NULL void *harbol_bilist_get(const struct HarbolBiList *list, uindex_t index);
HARBOL_EXPORT NO_NULL bool harbol_bilist_set(struct HarbolBiList *list, uindex_t index, void *val);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_bilist_index_del(struct HarbolBiList *list, uindex_t index, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_bilist_node_del(struct HarbolBiList *list, struct HarbolBiNode **noderef, void dtor(void**));
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_BILIST_INCLUDED */
