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
	struct HarbolEdge edge = EMPTY_HARBOL_EDGE;
	edge.weight = harbol_alloc(datasize, sizeof *edge.weight);
	if( edge.weight != NULL ) {
		edge.link = link;
		memcpy(edge.weight, data, datasize);
	}
	return edge;
}

HARBOL_EXPORT bool harbol_edge_clear(struct HarbolEdge *const edge, void dtor(void**))
{
	if( dtor != NULL )
		dtor((void**)&edge->weight);
	
	if( edge->weight != NULL )
		harbol_free(edge->weight), edge->weight=NULL;
	edge->link = -1;
	return true;
}

HARBOL_EXPORT void *harbol_edge_get(const struct HarbolEdge *const edge)
{
	return edge->weight;
}

HARBOL_EXPORT bool harbol_edge_set(struct HarbolEdge *const restrict edge, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( edge->weight==NULL ) {
		edge->weight = harbol_alloc(datasize, sizeof *edge->weight);
		return( edge->weight==NULL ) ? false : memcpy(edge->weight, data, datasize) != NULL;
	} else {
		return memcpy(edge->weight, data, datasize) != NULL;
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
	struct HarbolVertex vert = EMPTY_HARBOL_VERT;
	vert.data = harbol_alloc(datasize, sizeof *vert.data);
	if( vert.data != NULL ) {
		vert.edges = harbol_vector_create(sizeof(struct HarbolEdge), VEC_DEFAULT_SIZE);
		memcpy(vert.data, data, datasize);
	}
	return vert;
}

HARBOL_EXPORT bool harbol_vertex_clear(struct HarbolVertex *const vert, void vert_dtor(void**), void edge_dtor(void**))
{
	for( size_t i=0; i<vert->edges.count; i++ )
		harbol_edge_clear(harbol_vector_get(&vert->edges, i), edge_dtor);
	harbol_vector_clear(&vert->edges, NULL);
	
	if( vert_dtor != NULL )
		vert_dtor((void**)&vert->data);
	
	if( vert->data != NULL )
		harbol_free(vert->data), vert->data=NULL;
	return true;
}

HARBOL_EXPORT void *harbol_vertex_get(const struct HarbolVertex *const vert)
{
	return vert->data;
}

HARBOL_EXPORT bool harbol_vertex_set(struct HarbolVertex *const restrict vert, void *const restrict data, const size_t datasize)
{
	if( datasize==0 )
		return false;
	else if( vert->data==NULL ) {
		vert->data = harbol_alloc(datasize, sizeof *vert->data);
		return( vert->data==NULL ) ? false : memcpy(vert->data, data, datasize) != NULL;
	} else {
		return memcpy(vert->data, data, datasize) != NULL;
	}
}

HARBOL_EXPORT struct HarbolVector harbol_vertex_get_edges(const struct HarbolVertex *const vert)
{
	return vert->edges;
}

HARBOL_EXPORT struct HarbolEdge *harbol_vertex_get_edge(const struct HarbolVertex *vert, const uindex_t index)
{
	return harbol_vector_get(&vert->edges, index);
}

HARBOL_EXPORT bool harbol_vertex_del_index(struct HarbolVertex *const vert, const uindex_t index, void dtor(void**))
{
	struct HarbolEdge *del_edge = harbol_vertex_get_edge(vert, index);
	if( del_edge==NULL )
		return false;
	else {
		harbol_edge_clear(del_edge, dtor);
		harbol_vector_del(&vert->edges, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_vertex_del_edge(struct HarbolVertex *const vert, struct HarbolEdge *const edge, void dtor(void**))
{
	harbol_edge_clear(edge, dtor);
	const index_t i = harbol_vector_index_of(&vert->edges, edge, 0);
	harbol_vector_del(&vert->edges, i, NULL);
	return true;
}
/*****************************************************************/
/*****************************************************************/


HARBOL_EXPORT struct HarbolGraph *harbol_graph_new(const size_t vert_datasize, const size_t edge_datasize)
{
	struct HarbolGraph *g = harbol_alloc(1, sizeof *g);
	if( g != NULL )
		*g = harbol_graph_create(vert_datasize, edge_datasize);
	return g;
}

HARBOL_EXPORT struct HarbolGraph harbol_graph_create(const size_t vert_datasize, const size_t edge_datasize)
{
	struct HarbolGraph g = EMPTY_HARBOL_GRAPH;
	g.vert_datasize = vert_datasize;
	g.edge_datasize = edge_datasize;
	g.vertices = harbol_vector_create(sizeof(struct HarbolVertex), VEC_DEFAULT_SIZE);
	return g;
}

HARBOL_EXPORT bool harbol_graph_clear(struct HarbolGraph *const g, void vert_dtor(void**), void edge_dtor(void**))
{
	for( size_t i=0; i<g->vertices.count; i++ )
		harbol_vertex_clear(harbol_vector_get(&g->vertices, i), vert_dtor, edge_dtor);
	harbol_vector_clear(&g->vertices, NULL);
	return true;
}

HARBOL_EXPORT bool harbol_graph_free(struct HarbolGraph **const gref, void vert_dtor(void**), void edge_dtor(void**))
{
	if( *gref==NULL )
		return false;
	else {
		const bool res = harbol_graph_clear(*gref, vert_dtor, edge_dtor);
		harbol_free(*gref), *gref=NULL;
		return res;
	}
}

HARBOL_EXPORT size_t harbol_graph_vertices(const struct HarbolGraph *const graph)
{
	return graph->vertices.count;
}


HARBOL_EXPORT NO_NULL size_t harbol_graph_edges(const struct HarbolGraph *graph)
{
	size_t total_edges = 0;
	const struct HarbolVertex *const end=harbol_vector_get_iter_end_count(&graph->vertices);
	for( const struct HarbolVertex *vert=harbol_vector_get_iter(&graph->vertices); vert != NULL && vert<end; vert++ )
		total_edges += vert->edges.count;
	return total_edges;
}


HARBOL_EXPORT bool harbol_graph_add_vert(struct HarbolGraph *const restrict graph, void *const restrict val)
{
	if( graph->vert_datasize==0 )
		return false;
	else {
		struct HarbolVertex vert = harbol_vertex_create(val, graph->vert_datasize);
		return harbol_vector_insert(&graph->vertices, &vert);
	}
}

HARBOL_EXPORT struct HarbolVertex *harbol_graph_get_vert(const struct HarbolGraph *const graph, const uindex_t index)
{
	return harbol_vector_get(&graph->vertices, index);
}

HARBOL_EXPORT bool harbol_graph_del_vert(struct HarbolGraph *const graph, struct HarbolVertex *const del_vert, void vert_dtor(void**), void edge_dtor(void**))
{
	const index_t index = harbol_vector_index_of(&graph->vertices, del_vert, 0);
	if( index<0 )
		return false;
	else {
		for( uindex_t i=0; i<graph->vertices.count; i++ ) {
			if( i==(uindex_t)index )
				continue;
			else {
				struct HarbolVertex *restrict vert = harbol_vector_get(&graph->vertices, i);
				for( uindex_t n=0; n<vert->edges.count; n++ ) {
					struct HarbolEdge *restrict edge = harbol_vector_get(&vert->edges, n);
					if( edge->link==index )
						edge->link = -1;
				}
			}
		}
		harbol_vertex_clear(del_vert, vert_dtor, edge_dtor);
		harbol_vector_del(&graph->vertices, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_graph_del_index(struct HarbolGraph *const graph, const uindex_t index, void vert_dtor(void**), void edge_dtor(void**))
{
	struct HarbolVertex *const del_vert = harbol_vector_get(&graph->vertices, index);
	if( del_vert==NULL )
		return false;
	else {
		for( uindex_t i=0; i<graph->vertices.count; i++ ) {
			if( i==index )
				continue;
			else {
				struct HarbolVertex *vert = harbol_vector_get(&graph->vertices, i);
				for( uindex_t n=0; n<vert->edges.count; n++ ) {
					struct HarbolEdge *edge = harbol_vector_get(&vert->edges, n);
					if( edge->link==(index_t)index )
						edge->link = -1;
				}
			}
		}
		harbol_vertex_clear(del_vert, vert_dtor, edge_dtor);
		harbol_vector_del(&graph->vertices, index, NULL);
		return true;
	}
}

HARBOL_EXPORT bool harbol_graph_set_vert(struct HarbolGraph *const restrict graph, struct HarbolVertex *const restrict vert, void *const restrict val)
{
	return harbol_vertex_set(vert, val, graph->vert_datasize);
}

HARBOL_EXPORT bool harbol_graph_set_index(struct HarbolGraph *const restrict graph, const uindex_t index, void *const restrict val)
{
	struct HarbolVertex *const restrict vert = harbol_vector_get(&graph->vertices, index);
	return( vert==NULL ) ? false : harbol_graph_set_vert(graph, vert, val);
}

HARBOL_EXPORT bool harbol_graph_vert_add_edge(struct HarbolGraph *const restrict graph, struct HarbolVertex *const restrict vert, const index_t link, void *const restrict val)
{
	struct HarbolEdge edge = harbol_edge_create(val, graph->edge_datasize, link);
	return harbol_vector_insert(&vert->edges, &edge);
}

HARBOL_EXPORT bool harbol_graph_index_add_edge(struct HarbolGraph *const restrict graph, const uindex_t index, const index_t link, void *const restrict val)
{
	struct HarbolVertex *const restrict vert = harbol_graph_get_vert(graph, index);
	if( vert==NULL )
		return false;
	else {
		struct HarbolEdge edge = harbol_edge_create(val, graph->edge_datasize, link);
		return harbol_vector_insert(&vert->edges, &edge);
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
		for( uindex_t n=0; n<vert1->edges.count; n++ ) {
			struct HarbolEdge *e = harbol_vertex_get_edge(vert1, n);
			if( e->link<0 )
				continue;
			else if( e->link==(index_t)vert_index2 ) {
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
	const index_t vert2_index = harbol_vector_index_of(&graph->vertices, vert2, 0);
	for( uindex_t n=0; n<vert1->edges.count; n++ ) {
		struct HarbolEdge *e = harbol_vertex_get_edge(vert1, n);
		if( e->link<0 )
			continue;
		else if( e->link==vert2_index ) {
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
		for( uindex_t i=0; i<vert->edges.count; i++ ) {
			struct HarbolEdge *edge = harbol_vector_get(&vert->edges, i);
			if( edge->link==(index_t)index2 )
				return true;
		}
		return false;
	}
}

HARBOL_EXPORT NO_NULL bool harbol_graph_verts_adjacent(struct HarbolGraph *const graph, struct HarbolVertex *const vert1, struct HarbolVertex *const vert2)
{
	const index_t n = harbol_vector_index_of(&graph->vertices, vert2, 0);
	for( uindex_t i=0; i<vert1->edges.count; i++ ) {
		struct HarbolEdge *edge = harbol_vector_get(&vert1->edges, i);
		if( edge->link<0 )
			continue;
		else if( edge->link==n )
			return true;
	}
	return false;
}
