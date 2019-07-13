#ifndef HARBOL_GRAPH_INCLUDED
#	define HARBOL_GRAPH_INCLUDED

#include "../harbol_common_defines.h"
#include "../harbol_common_includes.h"
#include "../vector/vector.h"

#ifdef __cplusplus
extern "C" {
#endif


struct HarbolEdge {
	uint8_t *Weight;
	index_t Link;
};

HARBOL_EXPORT NO_NULL struct HarbolEdge harbol_edge_create(void *data, size_t datasize, index_t link);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_edge_clear(struct HarbolEdge *edge, void dtor(void**));

HARBOL_EXPORT NO_NULL void *harbol_edge_get(const struct HarbolEdge *edge);
HARBOL_EXPORT NO_NULL bool harbol_edge_set(struct HarbolEdge *edge, void *data, size_t datasize);


struct HarbolVertex {
	struct HarbolVector Edges;
	uint8_t *Data;
};

HARBOL_EXPORT NO_NULL struct HarbolVertex harbol_vertex_create(void *data, size_t datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_vertex_clear(struct HarbolVertex *vert, void vert_dtor(void**), void edge_dtor(void**));

HARBOL_EXPORT NO_NULL void *harbol_vertex_get(const struct HarbolVertex *vert);
HARBOL_EXPORT NO_NULL bool harbol_vertex_set(struct HarbolVertex *vert, void *data, size_t datasize);

HARBOL_EXPORT NO_NULL struct HarbolVector harbol_vertex_get_edges(const struct HarbolVertex *vert);
HARBOL_EXPORT NO_NULL struct HarbolEdge *harbol_vertex_get_edge(const struct HarbolVertex *vert, uindex_t index);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_vertex_del_index(struct HarbolVertex *vert, uindex_t index, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_vertex_del_edge(struct HarbolVertex *vert, struct HarbolEdge *edge, void dtor(void**));


// Adjacency List Graph Implementation.
struct HarbolGraph {
	struct HarbolVector Vertices;
	size_t VertexDataSize, EdgeDataSize;
};

HARBOL_EXPORT struct HarbolGraph *harbol_graph_new(size_t vert_datasize, size_t edge_datasize);
HARBOL_EXPORT struct HarbolGraph harbol_graph_create(size_t vert_datasize, size_t edge_datasize);
HARBOL_EXPORT NEVER_NULL(1) bool harbol_graph_clear(struct HarbolGraph *graph, void vert_dtor(void**), void edge_dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_graph_free(struct HarbolGraph **graphref, void vert_dtor(void**), void edge_dtor(void**));

HARBOL_EXPORT NO_NULL size_t harbol_graph_vertices(const struct HarbolGraph *graph);
HARBOL_EXPORT NO_NULL size_t harbol_graph_edges(const struct HarbolGraph *graph);

HARBOL_EXPORT NO_NULL bool harbol_graph_add_vert(struct HarbolGraph *graph, void *val);
HARBOL_EXPORT NO_NULL struct HarbolVertex *harbol_graph_get_vert(const struct HarbolGraph *graph, uindex_t index);

HARBOL_EXPORT NEVER_NULL(1, 2) bool harbol_graph_del_vert(struct HarbolGraph *graph, struct HarbolVertex *vert, void vert_dtor(void**), void edge_dtor(void**));
HARBOL_EXPORT NEVER_NULL(1) bool harbol_graph_del_index(struct HarbolGraph *graph, uindex_t index, void vert_dtor(void**), void edge_dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_graph_set_vert(struct HarbolGraph *graph, struct HarbolVertex *vert, void *val);
HARBOL_EXPORT NO_NULL bool harbol_graph_set_index(struct HarbolGraph *graph, uindex_t index, void *val);

HARBOL_EXPORT NO_NULL bool harbol_graph_vert_add_edge(struct HarbolGraph *graph, struct HarbolVertex *vert, index_t link, void *val);
HARBOL_EXPORT NO_NULL bool harbol_graph_index_add_edge(struct HarbolGraph *graph, uindex_t index, index_t link, void *val);

HARBOL_EXPORT NO_NULL struct HarbolEdge *harbol_graph_index_get_edge(const struct HarbolGraph *graph, uindex_t vert_index, uindex_t edge_index);

HARBOL_EXPORT NEVER_NULL(1) bool harbol_graph_index_del_edge(struct HarbolGraph *graph, uindex_t vert_index1, uindex_t vert_index2, void dtor(void**));
HARBOL_EXPORT NEVER_NULL(1,2,3) bool harbol_graph_vert_del_edge(struct HarbolGraph *graph, struct HarbolVertex *vert1, struct HarbolVertex *vert2, void dtor(void**));

HARBOL_EXPORT NO_NULL bool harbol_graph_indices_adjacent(struct HarbolGraph *graph, uindex_t index1, uindex_t index2);
HARBOL_EXPORT NO_NULL bool harbol_graph_verts_adjacent(struct HarbolGraph *graph, struct HarbolVertex *vert1, struct HarbolVertex *vert2);
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* HARBOL_GRAPH_INCLUDED */
