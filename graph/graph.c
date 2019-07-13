#include "graph.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


/*****************************************************************
******************************************************************
* Edge Code. *
******************************************************************
*****************************************************************/
HARBOL_EXPORT struct HarbolEdge harbol_edge_create(void *const data, const size_t datasize, const index_t link)
{
	struct HarbolEdge edge = {NULL, 0};
	edge.Weight = calloc(datasize, sizeof *edge.Weight);
	if( edge.Weight != NULL ) {
		edge.Link = link;
		memcpy(edge.Weight, data, datasize);
	}
	return edge;
}

HARBOL_EXPORT bool harbol_edge_clear(struct HarbolEdge *const edge, void dtor(void**))
{
	if( dtor != NULL )
		dtor((void**)&edge->Weight);
	
	if( edge->Weight != NULL )
		free(edge->Weight), edge->Weight=NULL;
	return true;
}

HARBOL_EXPORT void *harbol_edge_get(const struct HarbolEdge *const edge)
{
	return edge->Weight;
}

HARBOL_EXPORT bool harbol_edge_set(struct HarbolEdge *const restrict edge, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( edge->Weight==NULL ) {
		edge->Weight = calloc(datasize, sizeof *edge->Weight);
		return( edge->Weight==NULL ) ? false : memcpy(edge->Weight, data, datasize) != NULL;
	} else {
		return memcpy(edge->Weight, data, datasize) != NULL;
	}
}
/*****************************************************************/
/*****************************************************************/


/*****************************************************************
******************************************************************
* Vertex Code. *
******************************************************************
*****************************************************************/
HARBOL_EXPORT struct HarbolVertex harbol_vertex_create(void *data, size_t datasize)
{
	struct HarbolVertex vert = { {NULL, 0, 0, 0}, NULL };
	vert.Data = calloc(datasize, sizeof *vert.Data);
	if( vert.Data != NULL ) {
		vert.Edges = harbol_vector_create(sizeof(struct HarbolEdge), VEC_DEFAULT_SIZE);
		memcpy(vert.Data, data, datasize);
	}
	return vert;
}

HARBOL_EXPORT bool harbol_vertex_clear(struct HarbolVertex *const vert, void vert_dtor(void**), void edge_dtor(void**))
{
	for( size_t i=0; i<vert->Edges.Count; i++ )
		harbol_edge_clear(harbol_vector_get(&vert->Edges, i), edge_dtor);
	harbol_vector_clear(&vert->Edges, NULL);
	
	if( vert_dtor != NULL )
		vert_dtor((void**)&vert->Data);
	
	if( vert->Data != NULL )
		free(vert->Data), vert->Data=NULL;
	return true;
}

HARBOL_EXPORT void *harbol_vertex_get(const struct HarbolVertex *const vert)
{
	return vert->Data;
}

HARBOL_EXPORT bool harbol_vertex_set(struct HarbolVertex *const restrict vert, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( vert->Data==NULL ) {
		vert->Data = calloc(datasize, sizeof *vert->Data);
		return( vert->Data==NULL ) ? false : memcpy(vert->Data, data, datasize) != NULL;
	} else {
		return memcpy(vert->Data, data, datasize) != NULL;
	}
}

HARBOL_EXPORT struct HarbolVector harbol_vertex_get_edges(const struct HarbolVertex *const vert)
{
	return vert->Edges;
}

HARBOL_EXPORT struct HarbolEdge *harbol_vertex_get_edge(const struct HarbolVertex *vert, const uindex_t index)
{
	return harbol_vector_get(&vert->Edges, index);
}

HARBOL_EXPORT bool harbol_vertex_del_index(struct HarbolVertex *const vert, const uindex_t index, void dtor(void**))
{
	struct HarbolEdge *del_edge = harbol_vertex_get_edge(vert, index);
	if( del_edge==NULL )
		return false;
	else {
		harbol_edge_clear(del_edge, dtor);
		harbol_vector_del(&vert->Edges, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_vertex_del_edge(struct HarbolVertex *const vert, struct HarbolEdge *const edge, void dtor(void**))
{
	harbol_edge_clear(edge, dtor);
	const index_t i = harbol_vector_index_of(&vert->Edges, edge, 0);
	harbol_vector_del(&vert->Edges, i, NULL);
	return true;
}
/*****************************************************************/
/*****************************************************************/


HARBOL_EXPORT struct HarbolGraph *harbol_graph_new(const size_t vert_datasize, const size_t edge_datasize)
{
	struct HarbolGraph *g = calloc(1, sizeof *g);
	if( g != NULL )
		*g = harbol_graph_create(vert_datasize, edge_datasize);
	return g;
}

HARBOL_EXPORT struct HarbolGraph harbol_graph_create(const size_t vert_datasize, const size_t edge_datasize)
{
	struct HarbolGraph g = {{NULL, 0, 0, 0}, 0, 0};
	g.VertexDataSize = vert_datasize;
	g.EdgeDataSize = edge_datasize;
	g.Vertices = harbol_vector_create(sizeof(struct HarbolVertex), VEC_DEFAULT_SIZE);
	return g;
}

HARBOL_EXPORT bool harbol_graph_clear(struct HarbolGraph *const g, void vert_dtor(void**), void edge_dtor(void**))
{
	for( size_t i=0; i<g->Vertices.Count; i++ )
		harbol_vertex_clear(harbol_vector_get(&g->Vertices, i), vert_dtor, edge_dtor);
	harbol_vector_clear(&g->Vertices, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_graph_free(struct HarbolGraph **const gref, void vert_dtor(void**), void edge_dtor(void**))
{
	if( *gref==NULL )
		return false;
	else {
		const bool res = harbol_graph_clear(*gref, vert_dtor, edge_dtor);
		free(*gref), *gref=NULL;
		return res;
	}
}

HARBOL_EXPORT size_t harbol_graph_vertices(const struct HarbolGraph *const graph)
{
	return graph->Vertices.Count;
}


HARBOL_EXPORT NO_NULL size_t harbol_graph_edges(const struct HarbolGraph *graph)
{
	size_t total_edges = 0;
	const struct HarbolVertex *const end=harbol_vector_get_iter_end_count(&graph->Vertices);
	for( const struct HarbolVertex *vert=harbol_vector_get_iter(&graph->Vertices); vert != NULL && vert<end; vert++ )
		total_edges += vert->Edges.Count;
	return total_edges;
}


HARBOL_EXPORT bool harbol_graph_add_vert(struct HarbolGraph *const restrict graph, void *const restrict val)
{
	if( graph->VertexDataSize==0 )
		return false;
	else {
		struct HarbolVertex vert = harbol_vertex_create(val, graph->VertexDataSize);
		return harbol_vector_insert(&graph->Vertices, &vert);
	}
}

HARBOL_EXPORT struct HarbolVertex *harbol_graph_get_vert(const struct HarbolGraph *const graph, const uindex_t index)
{
	return harbol_vector_get(&graph->Vertices, index);
}

HARBOL_EXPORT bool harbol_graph_del_vert(struct HarbolGraph *const graph, struct HarbolVertex *const del_vert, void vert_dtor(void**), void edge_dtor(void**))
{
	const index_t index = harbol_vector_index_of(&graph->Vertices, del_vert, 0);
	if( index<0 )
		return false;
	else {
		for( uindex_t i=0; i<graph->Vertices.Count; i++ ) {
			if( i==(uindex_t)index )
				continue;
			else {
				struct HarbolVertex *restrict vert = harbol_vector_get(&graph->Vertices, i);
				for( uindex_t n=0; n<vert->Edges.Count; n++ ) {
					struct HarbolEdge *restrict edge = harbol_vector_get(&vert->Edges, n);
					if( edge->Link==index )
						edge->Link = -1;
				}
			}
		}
		harbol_vertex_clear(del_vert, vert_dtor, edge_dtor);
		harbol_vector_del(&graph->Vertices, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_graph_del_index(struct HarbolGraph *const graph, const uindex_t index, void vert_dtor(void**), void edge_dtor(void**))
{
	struct HarbolVertex *const del_vert = harbol_vector_get(&graph->Vertices, index);
	if( del_vert==NULL )
		return false;
	else {
		for( uindex_t i=0; i<graph->Vertices.Count; i++ ) {
			if( i==index )
				continue;
			else {
				struct HarbolVertex *vert = harbol_vector_get(&graph->Vertices, i);
				for( uindex_t n=0; n<vert->Edges.Count; n++ ) {
					struct HarbolEdge *edge = harbol_vector_get(&vert->Edges, n);
					if( edge->Link==(index_t)index )
						edge->Link = -1;
				}
			}
		}
		harbol_vertex_clear(del_vert, vert_dtor, edge_dtor);
		harbol_vector_del(&graph->Vertices, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_graph_set_vert(struct HarbolGraph *const restrict graph, struct HarbolVertex *const restrict vert, void *const restrict val)
{
	return harbol_vertex_set(vert, val, graph->VertexDataSize);
}

HARBOL_EXPORT bool harbol_graph_set_index(struct HarbolGraph *const restrict graph, const uindex_t index, void *const restrict val)
{
	struct HarbolVertex *const restrict vert = harbol_vector_get(&graph->Vertices, index);
	return( vert==NULL ) ? false : harbol_graph_set_vert(graph, vert, val);
}

HARBOL_EXPORT bool harbol_graph_vert_add_edge(struct HarbolGraph *const restrict graph, struct HarbolVertex *const restrict vert, const index_t link, void *const restrict val)
{
	struct HarbolEdge edge = harbol_edge_create(val, graph->EdgeDataSize, link);
	return harbol_vector_insert(&vert->Edges, &edge);
}

HARBOL_EXPORT bool harbol_graph_index_add_edge(struct HarbolGraph *const restrict graph, const uindex_t index, const index_t link, void *const restrict val)
{
	struct HarbolVertex *const restrict vert = harbol_graph_get_vert(graph, index);
	if( vert==NULL )
		return false;
	else {
		struct HarbolEdge edge = harbol_edge_create(val, graph->EdgeDataSize, link);
		return harbol_vector_insert(&vert->Edges, &edge);
	}
}

HARBOL_EXPORT struct HarbolEdge *harbol_graph_index_get_edge(const struct HarbolGraph *const graph, const uindex_t vert_index, const uindex_t edge_index)
{
	return harbol_vertex_get_edge(harbol_graph_get_vert(graph, vert_index), edge_index);
}

HARBOL_EXPORT bool harbol_graph_index_del_edge(struct HarbolGraph *const graph, const uindex_t vert_index1, const uindex_t vert_index2, void dtor(void**))
{
	struct HarbolVertex *const vert1 = harbol_graph_get_vert(graph, vert_index1);
	struct HarbolVertex *const vert2 = harbol_graph_get_vert(graph, vert_index2);
	if( vert1==NULL || vert2==NULL )
		return false;
	else {
		// make sure the vertex #1 actually has a connection to vertex #2
		struct HarbolEdge *edge = NULL;
		for( uindex_t n=0; n<vert1->Edges.Count; n++ ) {
			struct HarbolEdge *e = harbol_vertex_get_edge(vert1, n);
			if( e->Link<0 )
				continue;
			else if( e->Link==(index_t)vert_index2 ) {
				edge = e;
				break;
			}
		}
		return( edge==NULL ) ? false : harbol_vertex_del_edge(vert1, edge, dtor);
	}
}

HARBOL_EXPORT bool harbol_graph_vert_del_edge(struct HarbolGraph *graph, struct HarbolVertex *vert1, struct HarbolVertex *vert2, void dtor(void**))
{
	struct HarbolEdge *edge = NULL;
	const index_t vert2_index = harbol_vector_index_of(&graph->Vertices, vert2, 0);
	for( uindex_t n=0; n<vert1->Edges.Count; n++ ) {
		struct HarbolEdge *e = harbol_vertex_get_edge(vert1, n);
		if( e->Link<0 )
			continue;
		else if( e->Link==vert2_index ) {
			edge = e;
			break;
		}
	}
	return( edge==NULL ) ? false : harbol_vertex_del_edge(vert1, edge, dtor);
}

HARBOL_EXPORT bool harbol_graph_indices_adjacent(struct HarbolGraph *const graph, const uindex_t index1, const uindex_t index2)
{
	struct HarbolVertex *const vert = harbol_graph_get_vert(graph, index1);
	if( vert==NULL )
		return false;
	else {
		for( uindex_t i=0; i<vert->Edges.Count; i++ ) {
			struct HarbolEdge *edge = harbol_vector_get(&vert->Edges, i);
			if( edge->Link==(index_t)index2 )
				return true;
		}
		return false;
	}
}

HARBOL_EXPORT NO_NULL bool harbol_graph_verts_adjacent(struct HarbolGraph *const graph, struct HarbolVertex *const vert1, struct HarbolVertex *const vert2)
{
	const index_t n = harbol_vector_index_of(&graph->Vertices, vert2, 0);
	for( uindex_t i=0; i<vert1->Edges.Count; i++ ) {
		struct HarbolEdge *edge = harbol_vector_get(&vert1->Edges, i);
		if( edge->Link<0 )
			continue;
		else if( edge->Link==n )
			return true;
	}
	return false;
}
