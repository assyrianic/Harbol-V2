#ifndef HARBOL_TREE_INCLUDED
#	define HARBOL_TREE_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../vector/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HarbolTree {
	struct HarbolVector Children;
	uint8_t *Data;
};

#define EMPTY_HARBOL_TREE    { EMPTY_HARBOL_VECTOR, NULL }

HARBOL_EXPORT struct HarbolTree *harbol_tree_new(void *val, size_t datasize);
HARBOL_EXPORT struct HarbolTree harbol_tree_create(void *val, size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_tree_clear(struct HarbolTree *tree, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_tree_free(struct HarbolTree **treeref, void dtor(void**));

HARBOL_EXPORT NO_NULL void *harbol_tree_get(const struct HarbolTree *tree);
HARBOL_EXPORT NO_NULL bool harbol_tree_set(struct HarbolTree *tree, void *val, size_t datasize);

HARBOL_EXPORT NO_NULL bool harbol_tree_insert_child_val(struct HarbolTree *tree, void *val, size_t datasize);
HARBOL_EXPORT NO_NULL bool harbol_tree_insert_child_node(struct HarbolTree *tree, struct HarbolTree *child);

HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_tree_del_child_node(struct HarbolTree *tree, struct HarbolTree *childref, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_tree_del_child_index(struct HarbolTree *tree, uindex_t index, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_tree_del_child_val(struct HarbolTree *tree, void *val, size_t datasize, void dtor(void**));

HARBOL_EXPORT NO_NULL struct HarbolTree *harbol_tree_index_get_child(const struct HarbolTree *tree, uindex_t index);
HARBOL_EXPORT NO_NULL struct HarbolTree *harbol_tree_val_get_child(const struct HarbolTree *tree, void *val, size_t datasize);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_TREE_INCLUDED */
