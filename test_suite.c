#include <assert.h>
#include <stdalign.h>
#include <time.h>
#include "harbol.h"

void test_harbol_string(void);
void test_harbol_vector(void);
void test_harbol_unilist(void);
void test_harbol_bilist(void);
void test_harbol_tuple(void);
void test_harbol_bytebuffer(void);
void test_harbol_map(void);
void test_harbol_mempool(void);
void test_harbol_objpool(void);
void test_harbol_cache(void);
void test_harbol_graph(void);
void test_harbol_tree(void);
void test_harbol_linkmap(void);
void test_harbol_cfg(void);
void test_harbol_plugins(void);
void test_harbol_veque(void);
void test_harbol_lex(void);

FILE *g_harbol_debug_stream = NULL;

#ifdef HARBOL_USE_MEMPOOL
struct HarbolMemPool *g_pool;
#endif

union Value {
	int64_t int64;
};

int main()
{
	g_harbol_debug_stream = fopen("harbol_debug_output.txt", "wa+");
	if( !g_harbol_debug_stream )
		return -1;
	
	fprintf(g_harbol_debug_stream, "The Harbol Test Suite is using Harbol version %s | major: %u, minor: %u, patch: %u, phase: %s\n\n", HARBOL_VERSION_STRING, HARBOL_VERSION_MAJOR, HARBOL_VERSION_MINOR, HARBOL_VERSION_PATCH, HARBOL_VERSION_PHASE);
	
	fprintf(
		g_harbol_debug_stream,
		"float64 == double? %u\nfloat64_t == float? %u\nfloat32_t == float? %u | long double size: %zu\nfloatmax_t == long double? %u\nfloatmax_t == double? %u\nfloatmax_t == float? %u\n\n",
		sizeof(float64_t)==sizeof(double),
		sizeof(float64_t)==sizeof(float),
		sizeof(float32_t)==sizeof(float),
		sizeof(long double),
		sizeof(floatmax_t)==sizeof(long double),
		sizeof(floatmax_t)==sizeof(double),
		sizeof(floatmax_t)==sizeof(float)
	);
#ifdef HARBOL_USE_MEMPOOL
	struct HarbolMemPool m = harbol_mempool_create(1000000);
	g_pool = &m;
#endif
	
	test_harbol_string();
	test_harbol_vector();
	test_harbol_unilist();
	test_harbol_bilist();
	test_harbol_tuple();
	test_harbol_bytebuffer();
	test_harbol_map();
	test_harbol_graph();
	test_harbol_tree();
	test_harbol_linkmap();
	test_harbol_cfg();
	test_harbol_plugins();
	
	test_harbol_objpool();
	test_harbol_cache();
	test_harbol_mempool();
	test_harbol_veque();
	
	test_harbol_lex();
	
	fclose(g_harbol_debug_stream), g_harbol_debug_stream=NULL;
#ifdef HARBOL_USE_MEMPOOL
	harbol_mempool_clear(g_pool);
#endif
}

static void __print_mempool_nodes(const struct HarbolMemPool *const mempool)
{
	fputs("\nmempool :: printing mempool free bucket.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<HARBOL_BUCKET_SIZE; i++ )
		for( struct HarbolMemNode *n=mempool->buckets[i]; n != NULL; n = n->next )
			fprintf(g_harbol_debug_stream, "mempool bucket[%zu] node :: n (%" PRIuPTR ") size == %zu.\n", i, (uintptr_t)n, n->size);
	
	fputs("\nmempool :: printing mempool free list.\n", g_harbol_debug_stream);
	for( struct HarbolMemNode *n = mempool->freelist.head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "mempool list node :: n (%" PRIuPTR ") size == %zu.\n", (uintptr_t)n, n->size);
	
	fprintf(g_harbol_debug_stream, "mempool memory remaining :: %zu | freenodes: %zu.\n", harbol_mempool_mem_remaining(mempool), mempool->freelist.len);
}

void test_harbol_string(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("string :: test allocation/initialization.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	struct HarbolString *p = harbol_string_new("test ptr with cstr!");
	assert( p );
	fputs(p->cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\np's string len '%zu' | strlen val '%zu'\n", p->len, strlen(p->cstr));
	fputs("\n", g_harbol_debug_stream);
	
	struct HarbolString i = harbol_string_create("test stk with cstr!");
	fputs(i.cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\ni's string len '%zu' | strlen val '%zu'\n", i.len, strlen(i.cstr));
	fputs("\n", g_harbol_debug_stream);
	
	// test appending individual chars.
	fputs("string :: test appending individual chars.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	// correct output: test ptr with cstr!6
	harbol_string_add_char(p, ' ');
	harbol_string_add_char(p, '6');
	fputs(p->cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	harbol_string_add_char(&i, ' ');
	harbol_string_add_char(&i, '6');
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// test appending strings.
	fputs("string :: test appending C strings.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_string_add_cstr(p, " \'new string!\'");
	fputs(p->cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	harbol_string_add_cstr(&i, " \'new string!\'");
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// test appending string objects.
	fputs("\nstring :: test appending string objects.\n", g_harbol_debug_stream);
	harbol_string_copy_cstr(p, "A");
	harbol_string_copy_cstr(&i, "B");
	harbol_string_add_str(p, &i);
	harbol_string_add_str(&i, p);
	
	// correct output: AB
	fputs(p->cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// correct output: BAB
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// test copying string objects.
	fputs("\nstring :: test copying string objects.\n", g_harbol_debug_stream);
	harbol_string_copy_cstr(p, "copied from ptr!");
	harbol_string_add_str(&i, p);
	fputs(p->cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// test string formatting.
	fputs("\nstring :: test string formatting.\n", g_harbol_debug_stream);
	harbol_string_clear(&i);
	//harbol_string_reserve(&i, 100);
	harbol_string_format(&i, "%i + %f + %i", 900, 4242.2, 10);
	fputs(i.cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\ni's string len '%zu' | strlen val '%zu'\n", i.len, strlen(i.cstr));
	fputs("\n", g_harbol_debug_stream);
	harbol_string_format(&i, "%i + %f", 900, 4242.2);
	fputs(i.cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\ni's string len '%zu' | strlen val '%zu'\n", i.len, strlen(i.cstr));
	fputs("\n", g_harbol_debug_stream);
	
	// test string concatenation formatting.
	fputs("\nstring :: test string concatenation formatting.\n", g_harbol_debug_stream);
	harbol_string_clear(&i);
	harbol_string_format(&i, "%i + %f + %i + ", 900, 4242.2, 10);
	fputs(i.cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\ni's string len '%zu' | strlen val '%zu'\n", i.len, strlen(i.cstr));
	
	harbol_string_add_format(&i, "%i + %f + %i", 900, 4242.2, 10);
	fputs(i.cstr, g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\ni's string len '%zu' | strlen val '%zu'\n", i.len, strlen(i.cstr));
	fputs("\n", g_harbol_debug_stream);
	
	// test reversing string.
	fputs("\nstring :: test reversing string.\n", g_harbol_debug_stream);
	harbol_string_clear(&i);
	harbol_string_clear(p);
	i = harbol_string_create("test");
	harbol_string_reverse(&i);
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	harbol_string_clear(&i);
	i = harbol_string_create("abcd");
	harbol_string_reverse(&i);
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	harbol_string_clear(&i);
	i = harbol_string_create("hello world!");
	harbol_string_reverse(&i);
	fputs(i.cstr, g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	// free data
	fputs("string :: test destruction.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_string_clear(&i);
	fprintf(g_harbol_debug_stream, "i's string is null? '%s'\n", i.cstr ? "no" : "yes");
	
	harbol_string_clear(p);
	fprintf(g_harbol_debug_stream, "p's string is null? '%s'\n", p->cstr ? "no" : "yes");
	harbol_string_free(&p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}


static inline void int64_swap(int64_t *restrict a, int64_t *restrict b)
{
	if( *a==*b )
		return;
	else {
		*a ^= *b;
		*b ^= *a;
		*a ^= *b;
	}
}

void test_harbol_vector(void)
{
	// Test allocation and initializations
	fputs("vector :: test allocation/initialization.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	struct HarbolVector *p = harbol_vector_new(sizeof(union Value), 4);
	assert( p );
	
	struct HarbolVector i = harbol_vector_create(sizeof(union Value), 4);
	
	// test data inserting
	fputs("vector :: test insertion.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_vector_insert(p, &(union Value){.int64=100});
	harbol_vector_insert(&i, &(union Value){.int64=100});
	
	// test data retrieval
	fputs("vector :: test retrieval.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "ptr[0] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(p, 0))->int64);
	fprintf(g_harbol_debug_stream, "stk[0] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(&i, 0))->int64);
	
	// test data setting
	fputs("vector :: test setting data.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_vector_set(p, 0, &(union Value){.int64=10});
	harbol_vector_set(&i, 0, &(union Value){.int64=9});
	
	fprintf(g_harbol_debug_stream, "ptr[0] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(p, 0))->int64);
	fprintf(g_harbol_debug_stream, "stk[0] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(&i, 0))->int64);
	
	// append the vectors
	fputs("vector :: test vector appending.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_vector_add(p, &i);
	fprintf(g_harbol_debug_stream, "ptr[1] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(p, 1))->int64);
	fprintf(g_harbol_debug_stream, "stk[1] == %p | count %zu\n", harbol_vector_get(&i, 1), i.count);
	
	// test vector copying.
	fputs("vector :: test vector copying.", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	harbol_vector_clear(&i, NULL);
	
	harbol_vector_insert(&i, &(union Value){.int64=100});
	harbol_vector_insert(&i, &(union Value){.int64=101});
	harbol_vector_insert(&i, &(union Value){.int64=102});
	
	harbol_vector_copy(p, &i);
	fprintf(g_harbol_debug_stream, "ptr[1] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(p, 1))->int64);
	fprintf(g_harbol_debug_stream, "ptr[2] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(p, 2))->int64);
	fprintf(g_harbol_debug_stream, "stk[1] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(&i, 1))->int64);
	fprintf(g_harbol_debug_stream, "stk[2] == %" PRIi64 "\n", ((union Value *)harbol_vector_get(&i, 2))->int64);
	
	// test vector deleting and truncating.
	fputs("\nvector :: test item deletion.\n", g_harbol_debug_stream);
	harbol_vector_clear(&i, NULL);
	harbol_vector_clear(p, NULL);
	
	harbol_vector_insert(p, &(union Value){.int64=100});
	harbol_vector_insert(p, &(union Value){.int64=101});
	harbol_vector_insert(p, &(union Value){.int64=102});
	harbol_vector_insert(p, &(union Value){.int64=103});
	harbol_vector_insert(p, &(union Value){.int64=104});
	harbol_vector_insert(p, &(union Value){.int64=105});
	
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	fputs("\n", g_harbol_debug_stream);
	
	harbol_vector_del(p, 0, NULL); // deletes 100
	harbol_vector_del(p, 1, NULL); // deletes 102 since 101 because new 0 index
	harbol_vector_del(p, 2, NULL); // deletes 104
	
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	
	fputs("\nvector :: test vector truncation.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "\nbefore truncating ptr[] len == %zu\n", p->len);
	
	harbol_vector_truncate(p);
	fprintf(g_harbol_debug_stream, "after truncating ptr[] len == %zu\n\n", p->len);
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	
	fputs("\nvector :: test vector popping.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "prepop ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	fputs("\n", g_harbol_debug_stream);
	const union Value *vec_item_2 = harbol_vector_pop(p);
	
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "postpop ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	fputs("\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "popped val == %" PRIi64 "\n", vec_item_2->int64);
	
	fputs("\nvector :: test counting.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "count of value '103' %zu\n", harbol_vector_count_item(p, &(union Value){.int64=103}));
	
	fputs("\nvector :: test index of value.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "index of value '101' %zu\n", harbol_vector_index_of(p, &(union Value){.int64=101}, 0));
	
	fputs("\nvector :: test reversing vector.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "pre-reversing ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	
	harbol_vector_reverse(p, (void(*)(void*,void*))&int64_swap);
	fputs("\n", g_harbol_debug_stream);
	
	for( uindex_t i=0; i<p->count; i++ )
		fprintf(g_harbol_debug_stream, "post-reversing ptr[%zu] == %" PRIi64 "\n", i, ((union Value *)harbol_vector_get(p, i))->int64);
	
	
	// free data
	fputs("\nvector :: test destruction.\n", g_harbol_debug_stream);
	
	harbol_vector_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's table is null? '%s'\n", i.table ? "no" : "yes");
	
	harbol_vector_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's table is null? '%s'\n", p->table ? "no" : "yes");
	harbol_vector_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_unilist(void)
{
	// Test allocation and initializations
	fputs("unilist :: test allocation / initialization.\n", g_harbol_debug_stream);
	struct HarbolUniList *p = harbol_unilist_new(sizeof(union Value));
	assert( p );
	
	struct HarbolUniList i = harbol_unilist_create(sizeof(union Value));
	
	// test insertion.
	// test insertion at tail.
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test tail insertion.\n", g_harbol_debug_stream);
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=1});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "pre-tail insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	fputs("\n", g_harbol_debug_stream);
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=2});
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=3});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-tail insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test insertion at head.
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test head insertion.\n", g_harbol_debug_stream);
	harbol_unilist_insert_at_head(p, &(union Value){.int64=0});
	harbol_unilist_insert_at_head(p, &(union Value){.int64=-1});
	harbol_unilist_insert_at_head(p, &(union Value){.int64=-2});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test retrieval.
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test node retrival by index.\n", g_harbol_debug_stream);
	{
		struct HarbolUniNode *n = harbol_unilist_index_get_node(p, 1);
		fprintf(g_harbol_debug_stream, "value of n: %" PRIi64 "\n", ((union Value *)n->data)->int64);
		
		// test item destruction.
		fputs("\nunilist :: test node removal & destruction by reference.\ndeleting n...\n", g_harbol_debug_stream);
		harbol_unilist_node_del(p, &n, NULL);
		for( struct HarbolUniNode *k=p->head; k; k = k->next )
			fprintf(g_harbol_debug_stream, "post-n-deletion k value : %" PRIi64 "\n", ((union Value *)k->data)->int64);
		
		fputs("\nunilist :: test node removal & destruction by index.\ndeleting index 1\n", g_harbol_debug_stream);
		harbol_unilist_index_del(p, 1, NULL);
		for( struct HarbolUniNode *k=p->head; k; k = k->next )
			fprintf(g_harbol_debug_stream, "post-index deletion k value : %" PRIi64 "\n", ((union Value *)k->data)->int64);
	}
	
	fputs("\n", g_harbol_debug_stream);
	// test setting data by index
	fputs("\nunilist :: test setting data by index (index of 0 is head node).\n", g_harbol_debug_stream);
	harbol_unilist_set(p, 0, &(union Value){.int64=222}); // 0 is head
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-setting n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test deleting head node after freeing.
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test deleting head node after freeing and new insertion.\n", g_harbol_debug_stream);
	harbol_unilist_clear(p, NULL);
	harbol_unilist_insert_at_head(p, &(union Value){.int64=100});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	harbol_unilist_index_del(p, 0, NULL);
	fprintf(g_harbol_debug_stream, "unilist head node ptr : %p, tail node ptr : %p\n", (void*)p->head, (void*)p->tail);
	
	
	fputs("\n", g_harbol_debug_stream);
	// test deleting items by index on a list size of 2.
	fputs("\nunilist :: test deleting items by index on a list size of 2.\n", g_harbol_debug_stream);
	harbol_unilist_clear(p, NULL);
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=100});
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=101});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "pre-deletion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	harbol_unilist_index_del(p, 0, NULL);
	fputs("\n", g_harbol_debug_stream);
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-deletion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test insertion in middle of list
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test middle insertion.\n", g_harbol_debug_stream);
	harbol_unilist_clear(p, NULL);
	harbol_unilist_insert_at_index(p, &(union Value){.int64=100}, 0); // have it insert at head by inserting at index 0.
	harbol_unilist_insert_at_index(p, &(union Value){.int64=101}, 10); // have it insert at tail by having the index exceed the size of list
	harbol_unilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_unilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_unilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_unilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_unilist_insert_at_index(p, &(union Value){.int64=103}, 1);
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test deleting the tail through an index.
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test deleting tail by index.\n", g_harbol_debug_stream);
	harbol_unilist_clear(p, NULL);
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=100});
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=101});
	harbol_unilist_insert_at_tail(p, &(union Value){.int64=102});
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "pre-deleting-index 2 n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	harbol_unilist_index_del(p, 2, NULL);
	fputs("\n", g_harbol_debug_stream);
	for( struct HarbolUniNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-deleting-index 2 n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test finding a node by value.
	
	// free data
	fputs("\n", g_harbol_debug_stream);
	fputs("\nunilist :: test destruction.\n", g_harbol_debug_stream);
	harbol_unilist_clear(&i, NULL);
	harbol_unilist_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's head is null? '%s'\n", i.head ? "no" : "yes");
	
	harbol_unilist_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's head is null? '%s', but is p null? '%s'\n", p->head ? "no" : "yes", p ? "no" : "yes");
	harbol_unilist_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_bilist(void)
{
	// Test allocation and initializations
	fputs("\nbilist :: test allocation / initialization.\n", g_harbol_debug_stream);
	struct HarbolBiList *p = harbol_bilist_new(sizeof(union Value));
	assert( p );
	
	struct HarbolBiList i = harbol_bilist_create(sizeof(union Value));
	
	// test insertion.
	// test insertion at tail.
	fputs("\n\nbilist :: test tail insertion.\n", g_harbol_debug_stream);
	harbol_bilist_insert_at_tail(p, &(union Value){.int64=1});
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post single tail insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	harbol_bilist_insert_at_tail(p, &(union Value){.int64=2});
	harbol_bilist_insert_at_tail(p, &(union Value){.int64=3});
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post double tail insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test insertion at head.
	fputs("\n\nbilist :: test head insertion.\n", g_harbol_debug_stream);
	harbol_bilist_insert_at_head(p, &(union Value){.int64=0});
	harbol_bilist_insert_at_head(p, &(union Value){.int64=-1});
	harbol_bilist_insert_at_head(p, &(union Value){.int64=-2});
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post triple head insertion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test retrieval.
	fputs("\n\nbilist :: test node retrival by index.\n", g_harbol_debug_stream);
	{
		struct HarbolBiNode *n = harbol_bilist_index_get_node(p, 1);
		fprintf(g_harbol_debug_stream, "list len: %zu\n", p->len); 
		fprintf(g_harbol_debug_stream, "n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
		
		// test item destruction.
		fputs("\n\nbilist :: test node removal & destruction by reference.\n", g_harbol_debug_stream);
		harbol_bilist_node_del(p, &n, NULL);
		for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
			fprintf(g_harbol_debug_stream, "post-del-by-ref n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
		
		fputs("\n\nbilist :: test node removal & destruction by index.", g_harbol_debug_stream);
		harbol_bilist_index_del(p, 1, NULL);
		for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
			fprintf(g_harbol_debug_stream, "post-del-by-index n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	}
	
	// test setting data by index
	fputs("\n\nbilist :: test setting data by index (index of 0 is head node).\n", g_harbol_debug_stream);
	harbol_bilist_set(p, 0, &(union Value){.int64=222}); // 0 is head
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-setting n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test deleting head node after freeing.
	fputs("\n\nbilist :: test deleting head node after freeing and new insertion.\n", g_harbol_debug_stream);
	harbol_bilist_clear(p, NULL);
	harbol_bilist_insert_at_head(p, &(union Value){.int64=100});
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	harbol_bilist_index_del(p, 0, NULL);
	fprintf(g_harbol_debug_stream, "bilist head node ptr : %p, tail node ptr : %p\n", (void*)p->head, (void*)p->tail);
	
	
	// test deleting items by index on a list size of 2.
	fputs("\n\nbilist :: test deleting items by index on a list size of 2.\n", g_harbol_debug_stream);
	harbol_bilist_clear(p, NULL);
	harbol_bilist_insert_at_tail(p, &(union Value){.int64=100});
	harbol_bilist_insert_at_tail(p, &(union Value){.int64=101});
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "pre-deletion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	harbol_bilist_index_del(p, 0, NULL);
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "post-deletion n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	// test insertion in middle of list
	fputs("\n\nbilist :: test middle insertion.\n", g_harbol_debug_stream);
	harbol_bilist_clear(p, NULL);
	harbol_bilist_insert_at_index(p, &(union Value){.int64=100}, 0); // have it insert at head by inserting at index 0.
	harbol_bilist_insert_at_index(p, &(union Value){.int64=101}, 10); // have it insert at tail by having the index exceed the size of list
	harbol_bilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_bilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_bilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_bilist_insert_at_index(p, &(union Value){.int64=102}, 1);
	harbol_bilist_insert_at_index(p, &(union Value){.int64=103}, 1);
	for( struct HarbolBiNode *n=p->head; n != NULL; n = n->next )
		fprintf(g_harbol_debug_stream, "n value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	
	// free data
	fputs("\n\nbilist :: test destruction.\n", g_harbol_debug_stream);
	harbol_bilist_clear(&i, NULL);
	harbol_bilist_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's head is null? '%s'\n", i.head ? "no" : "yes");
	
	harbol_bilist_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's head is null? '%s', but is p null? '%s'\n", p->head ? "no" : "yes", p ? "no" : "yes");
	harbol_bilist_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_tuple(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("tuple :: test allocation/initialization.\n", g_harbol_debug_stream);
	fputs("\n", g_harbol_debug_stream);
	
	const size_t struc[] = { sizeof(char), sizeof(int), sizeof(short) };
	
	struct HarbolTuple *p = harbol_tuple_new(1[&struc] - 0[&struc], struc, false);
	assert( p );
	fprintf(g_harbol_debug_stream, "p's size: '%zu'\n", p->len);
	
	fputs("\ntuple :: printing 3-tuple fields.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->fields.count; i++ ) {
		//fprintf(g_harbol_debug_stream, "offset: %zu -> p's ptr: '%" PRIuPTR "'\n", i, harbol_vector_get(&p->fields, i).UIntNative);
		struct {
			uint16_t size;
			uint16_t Offset;
		} *info = harbol_vector_get(&p->fields, i);
		fprintf(g_harbol_debug_stream, "index: %zu -> p's offset: '%u' | size: '%u'\n", i, info->Offset, info->size);
	}
	fputs("\n", g_harbol_debug_stream);
	
	harbol_tuple_free(&p);
	
	p = harbol_tuple_new(1[&struc] - 0[&struc], struc, true);
	assert( p );
	fprintf(g_harbol_debug_stream, "packed p's size: '%zu'\n", p->len);
	
	fputs("\ntuple :: printing packed 3-tuple fields.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->fields.count; i++ ) {
		//fprintf(g_harbol_debug_stream, "offset: %zu -> p's ptr: '%" PRIuPTR "'\n", i, harbol_vector_get(&p->fields, i).UIntNative);
		struct {
			uint16_t size;
			uint16_t Offset;
		} *info = harbol_vector_get(&p->fields, i);
		fprintf(g_harbol_debug_stream, "index: %zu -> p's offset: '%u' | size: '%u'\n", i, info->Offset, info->size);
	}
	fputs("\n", g_harbol_debug_stream);
	
	harbol_tuple_free(&p);
	
	p = harbol_tuple_new(4, (size_t[]){ sizeof(char), sizeof(char), sizeof(char), sizeof(char) }, false);
	assert( p );
	fprintf(g_harbol_debug_stream, "char p's size: '%zu'\n", p->len);
	
	fputs("\ntuple :: printing byte 4-tuple fields.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->fields.count; i++ ) {
		//fprintf(g_harbol_debug_stream, "offset: %zu -> p's ptr: '%" PRIuPTR "'\n", i, harbol_vector_get(&p->fields, i).UIntNative);
		struct {
			uint16_t size;
			uint16_t Offset;
		} *info = harbol_vector_get(&p->fields, i);
		fprintf(g_harbol_debug_stream, "index: %zu -> p's offset: '%u' | size: '%u'\n", i, info->Offset, info->size);
	}
	fputs("\n", g_harbol_debug_stream);
	
	harbol_tuple_free(&p);
	
	p = harbol_tuple_new(4, (const size_t[]){ sizeof(int64_t), sizeof(char), sizeof(int), sizeof(short) }, false);
	assert( p );
	fprintf(g_harbol_debug_stream, "p's size: '%zu'\n", p->len);
	
	fputs("\ntuple :: printing 4-tuple fields.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->fields.count; i++ ) {
		struct {
			uint16_t size;
			uint16_t Offset;
		} *info = harbol_vector_get(&p->fields, i);
		fprintf(g_harbol_debug_stream, "index: %zu -> p's offset: '%u' | size: '%u'\n", i, info->Offset, info->size);
	}
	fputs("\n", g_harbol_debug_stream);
	harbol_tuple_free(&p);
	
	p = harbol_tuple_new(3, (size_t[]){ sizeof(int), sizeof(char), sizeof(short) }, false);
	assert( p );
	fprintf(g_harbol_debug_stream, "p's size: '%zu'\n", p->len);
	
	fputs("\ntuple :: printing newly aligned 3-tuple fields.\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<p->fields.count; i++ ) {
		struct {
			uint16_t size;
			uint16_t Offset;
		} *info = harbol_vector_get(&p->fields, i);
		fprintf(g_harbol_debug_stream, "index: %zu -> p's offset: '%u' | size: '%u'\n", i, info->Offset, info->size);
	}
	fputs("\n", g_harbol_debug_stream);
	
	// free tuple
	harbol_tuple_clear(p);
	fprintf(g_harbol_debug_stream, "p's item is null? '%s'\n", p->datum ? "no" : "yes");
	harbol_tuple_free(&p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n\n", p ? "no" : "yes");
}

void test_harbol_bytebuffer(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("bytebuffer :: test allocation/initialization.\n", g_harbol_debug_stream);
	struct HarbolByteBuf *p = harbol_bytebuffer_new();
	assert( p );
	
	struct HarbolByteBuf i = harbol_bytebuffer_create();
	
	// test adding a byte.
	fputs("\nbytebuffer :: test byte appending.\n", g_harbol_debug_stream);
	harbol_bytebuffer_insert_byte(p, 5);
	harbol_bytebuffer_insert_byte(&i, 6);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "i[%zu]= %u\n", n, i.table[n]);
	
	fputs("\nbytebuffer :: test uint16 appending.\n", g_harbol_debug_stream);
	// test integer appending
	harbol_bytebuffer_clear(&i);
	harbol_bytebuffer_clear(p);
	uint16_t ush = 50;
	harbol_bytebuffer_insert_int16(p, ush);
	harbol_bytebuffer_insert_int16(&i, ush);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "i[%zu]= %u\n", n, i.table[n]);
	
	harbol_bytebuffer_clear(&i);
	harbol_bytebuffer_clear(p);
	
	fputs("\nbytebuffer :: test uint64 appending.\n", g_harbol_debug_stream);
	uint64_t ull = 0xabcdef;
	harbol_bytebuffer_insert_int64(p, ull);
	harbol_bytebuffer_insert_int64(&i, ull);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "i[%zu]= %u\n", n, i.table[n]);
	
	fputs("\nbytebuffer :: test string appending.\n", g_harbol_debug_stream);
	const char *s = "supercalifragilisticexpialidocius";
	harbol_bytebuffer_insert_cstr(p, s);
	harbol_bytebuffer_insert_cstr(&i, s);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "i[%zu]= %u\n", n, i.table[n]);
	
	
	fputs("\nbytebuffer :: test range deletion.\n", g_harbol_debug_stream);
	harbol_bytebuffer_clear(&i);
	harbol_bytebuffer_clear(p);
	ull = 0xABCDEF1234567890;
	harbol_bytebuffer_insert_int64(p, ull);
	harbol_bytebuffer_insert_int64(&i, ull);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "pre-deletion p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "pre-deletion i[%zu]= %u\n", n, i.table[n]);
		
	fputs("\n", g_harbol_debug_stream);
	harbol_bytebuffer_del(p, 0, 4);
	harbol_bytebuffer_del(&i, 0, 4);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "post-deletion p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( size_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "post-deletion i[%zu]= %u\n", n, i.table[n]);
		
		
	fputs("\nbytebuffer :: test full range deletion.\n", g_harbol_debug_stream);
	harbol_bytebuffer_clear(&i);
	harbol_bytebuffer_clear(p);
	ull = 0xABCDEF1234567890;
	harbol_bytebuffer_insert_int64(p, ull);
	harbol_bytebuffer_insert_int64(&i, ull);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "pre-deletion p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "pre-deletion i[%zu]= %u\n", n, i.table[n]);
		
	fputs("\n", g_harbol_debug_stream);
	harbol_bytebuffer_del(p, 2, 3);
	harbol_bytebuffer_del(&i, 2, 3);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "post-deletion p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "post-deletion i[%zu]= %u\n", n, i.table[n]);
	
	
	fputs("\nbytebuffer :: test buffer appending.\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "pre-appending p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "pre-appending i[%zu]= %u\n", n, i.table[n]);
		
	fputs("\n", g_harbol_debug_stream);
	harbol_bytebuffer_append(p, &i);
	for( uindex_t n=0; n<p->count; n++ )
		fprintf(g_harbol_debug_stream, "post-appending p[%zu]= %u\n", n, p->table[n]);
	
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.count; n++ )
		fprintf(g_harbol_debug_stream, "post-appending i[%zu]= %u\n", n, i.table[n]);
	
	
	// free data
	fputs("\nbytebuffer :: test destruction.\n", g_harbol_debug_stream);
	harbol_bytebuffer_clear(&i);
	fprintf(g_harbol_debug_stream, "i's table is null? '%s'\n", i.table ? "no" : "yes");
	
	harbol_bytebuffer_clear(p);
	fprintf(g_harbol_debug_stream, "p's table is null? '%s'\n", p->table ? "no" : "yes");
	harbol_bytebuffer_free(&p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_map(void)
{
	// Test allocation and initializations
	fputs("hashmap :: test allocation & initialization.\n", g_harbol_debug_stream);
	struct HarbolMap *p = harbol_map_new(sizeof(union Value));
	assert( p );
	
	struct HarbolMap i = harbol_map_create(sizeof(union Value));
	
	// test insertion
	fputs("\nhashmap :: test insertion.\n", g_harbol_debug_stream);
	for( size_t n=0; n<10; n++ ) {
		char key[5] = {0};
		sprintf(key, "%zu", n+1);
		harbol_map_insert(p, key, &(union Value){.int64=n+1});
		harbol_map_insert(&i, key, &(union Value){.int64=n+1});
	}
	
	// test retrieval.
	fputs("\nhashmap :: test data retrieval.\n", g_harbol_debug_stream);
	for( size_t n=0; n<10; n++ ) {
		char key[5] = {0};
		sprintf(key, "%zu", n+1);
		fprintf(g_harbol_debug_stream, "ptr[\"%s\"] == %" PRIi64 "\n", key, ((union Value *)harbol_map_get(p, key))->int64);
		fprintf(g_harbol_debug_stream, "stk[\"%s\"] == %" PRIi64 "\n\n", key, ((union Value *)harbol_map_get(&i, key))->int64);
	}
	
	// test setting.
	fputs("\nhashmap :: test data setting.\n", g_harbol_debug_stream);
	harbol_map_set(p, "2", &(union Value){.int64=20});
	harbol_map_set(&i, "2", &(union Value){.int64=200});
	for( size_t n=0; n<10; n++ ) {
		char key[5] = {0};
		sprintf(key, "%zu", n+1);
		fprintf(g_harbol_debug_stream, "ptr[\"%s\"] == %" PRIi64 "\n", key, ((union Value *)harbol_map_get(p, key))->int64);
		fprintf(g_harbol_debug_stream, "stk[\"%s\"] == %" PRIi64 "\n\n", key, ((union Value *)harbol_map_get(&i, key))->int64);
	}
	
	// test deletion
	fputs("\nhashmap :: test item deletion.\n", g_harbol_debug_stream);
	harbol_map_del(p, "2", NULL);
	fprintf(g_harbol_debug_stream, "ptr[\"1\"] == %" PRIi64 "\n", ((union Value *)harbol_map_get(p, "1"))->int64);
	fprintf(g_harbol_debug_stream, "ptr[\"2\"] == %p\n", harbol_map_get(p, "2"));
	
	// free data
	fputs("\nhashmap :: test destruction.\n", g_harbol_debug_stream);
	harbol_map_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's buckets are null? '%s'\n", i.buckets ? "no" : "yes");
	
	harbol_map_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's buckets are null? '%s'\n", p->buckets ? "no" : "yes");
	harbol_map_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}


void test_harbol_mempool(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("mempool :: test allocation/initialization.\n", g_harbol_debug_stream);
	
	struct HarbolMemPool i = harbol_mempool_create(1000);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	
	const clock_t start = clock();
	// test giving memory
	fputs("mempool :: test giving memory.\n", g_harbol_debug_stream);
	fputs("\nmempool :: allocating int ptr.\n", g_harbol_debug_stream);
	int *p = harbol_mempool_alloc(&i, sizeof *p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
	if( p ) {
		*p = 500;
		fprintf(g_harbol_debug_stream, "p's value: %i\n", *p);
	}
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	
	fputs("\nmempool :: allocating float ptr.\n", g_harbol_debug_stream);
	float *f = harbol_mempool_alloc(&i, sizeof *f);
	fprintf(g_harbol_debug_stream, "f is null? '%s'\n", f ? "no" : "yes");
	if( f ) {
		*f = 500.5f;
		fprintf(g_harbol_debug_stream, "f's value: %f\n", *f);
	}
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	
	// test releasing memory
	fputs("mempool :: test releasing memory.\n", g_harbol_debug_stream);
	harbol_mempool_free(&i, f), f=NULL;
	harbol_mempool_free(&i, p), p=NULL;
	
	// test re-giving memory
	fputs("mempool :: test regiving memory.\n", g_harbol_debug_stream);
	p = harbol_mempool_alloc(&i, sizeof *p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
	if( p ) {
		*p = 532;
		fprintf(g_harbol_debug_stream, "p's value: %i\n", *p);
	}
	f = harbol_mempool_alloc(&i, sizeof *f);
	fprintf(g_harbol_debug_stream, "f is null? '%s'\n", f ? "no" : "yes");
	if( f ) {
		*f = 466.5f;
		fprintf(g_harbol_debug_stream, "f's value: %f\n", *f);
	}
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	harbol_mempool_free(&i, p), p=NULL; // release memory that's from different region.
	harbol_mempool_free(&i, f), f=NULL;
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	
	// test giving array memory
	fputs("\nmempool :: test giving array memory.\n", g_harbol_debug_stream);
	const size_t arrsize = 100;
	p = harbol_mempool_alloc(&i, sizeof *p * arrsize);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
	if( p ) {
		for( uindex_t i=0; i<arrsize; i++ ) {
			p[i] = i+1;
			fprintf(g_harbol_debug_stream, "p[%zu] value: %i\n", i, p[i]);
		}
	}
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	f = harbol_mempool_alloc(&i, sizeof *f * arrsize);
	fprintf(g_harbol_debug_stream, "f is null? '%s'\n", f ? "no" : "yes");
	if( f ) {
		for( uindex_t i=0; i<arrsize; i++ ) {
			f[i] = i+1.15f;
			fprintf(g_harbol_debug_stream, "f[%zu] value: %f\n", i, f[i]);
		}
	}
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	harbol_mempool_free(&i, p), p=NULL;
	harbol_mempool_free(&i, f), f=NULL;
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
	harbol_mempool_defrag(&i);
	__print_mempool_nodes(&i);
	
	// test using heap to make a unilinked list!
	fputs("\nmempool :: test using mempool for unilist.\n", g_harbol_debug_stream);
	struct HarbolUniList *list = harbol_mempool_alloc(&i, sizeof *list);
	assert( list );
	struct HarbolMemNode *b = (struct HarbolMemNode *)((uint8_t *)list - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: list (%" PRIuPTR ") alloc node size: %zu, sizeof *list: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "; base mem: (%" PRIuPTR ")\n", (uintptr_t)list, b->size, sizeof *list, (uintptr_t)b, (uint8_t*)b - i.stack.mem, (uintptr_t)i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	*list = harbol_unilist_create(sizeof(union Value));
	
	struct HarbolUniNode *node1 = harbol_mempool_alloc(&i, sizeof *node1);
	assert( node1 );
	
	b = (struct HarbolMemNode *)((uint8_t *)node1 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node1 (%" PRIuPTR ") alloc node size: %zu, sizeof *node1: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node1, b->size, sizeof *node1, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node1->data = (uint8_t *)&(union Value){.int64 = 1};
	harbol_unilist_add_node_at_tail(list, node1);
	
	struct HarbolUniNode *node2 = harbol_mempool_alloc(&i, sizeof *node2);
	assert( node2 );
	b = (struct HarbolMemNode *)((uint8_t *)node2 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node2 (%" PRIuPTR ") alloc node size: %zu, sizeof *node2: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node2, b->size, sizeof *node2, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node2->data = (uint8_t *)&(union Value){.int64 = 2};
	harbol_unilist_add_node_at_tail(list, node2);
	
	struct HarbolUniNode *node3 = harbol_mempool_alloc(&i, sizeof *node3);
	assert( node3 );
	b = (struct HarbolMemNode *)((uint8_t *)node3 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node3 (%" PRIuPTR ") alloc node size: %zu, sizeof *node3: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node3, b->size, sizeof *node3, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node3->data = (uint8_t *)&(union Value){.int64 = 3};
	harbol_unilist_add_node_at_tail(list, node3);
	
	struct HarbolUniNode *node4 = harbol_mempool_alloc(&i, sizeof *node4);
	assert( node4 );
	b = (struct HarbolMemNode *)((uint8_t *)node4 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node4 (%" PRIuPTR ") alloc node size: %zu, sizeof *node4: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node4, b->size, sizeof *node4, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node4->data = (uint8_t *)&(union Value){.int64 = 4};
	harbol_unilist_add_node_at_tail(list, node4);
	
	struct HarbolUniNode *node5 = harbol_mempool_alloc(&i, sizeof *node5);
	assert( node5 );
	b = (struct HarbolMemNode *)((uint8_t *)node5 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node5 (%" PRIuPTR ") alloc node size: %zu, sizeof *node5: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node5, b->size, sizeof *node5, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node5->data = (uint8_t *)&(union Value){.int64 = 5};
	harbol_unilist_add_node_at_tail(list, node5);
	
	struct HarbolUniNode *node6 = harbol_mempool_alloc(&i, sizeof *node6);
	assert( node6 );
	b = (struct HarbolMemNode *)((uint8_t *)node6 - sizeof *b);
	fprintf(g_harbol_debug_stream, "mempool :: node6 (%" PRIuPTR ") alloc node size: %zu, sizeof *node6: %zu, b node: (%" PRIuPTR "), offset: %" PRIiPTR "\n", (uintptr_t)node6, b->size, sizeof *node6, (uintptr_t)b, (uint8_t*)b - i.stack.mem);
	fprintf(g_harbol_debug_stream, "remaining heap mem: '%zu'\n", harbol_mempool_mem_remaining(&i));
	node6->data = (uint8_t *)&(union Value){.int64 = 6};
	harbol_unilist_add_node_at_tail(list, node6);
	
	__print_mempool_nodes(&i);
	
	for( struct HarbolUniNode *n=list->head; n; n = n->next )
		fprintf(g_harbol_debug_stream, "uninode value : %" PRIi64 "\n", ((union Value *)n->data)->int64);
	
	harbol_mempool_free(&i, list), list=NULL;
	harbol_mempool_free(&i, node1), node1=NULL;
	harbol_mempool_free(&i, node2), node2=NULL;
	harbol_mempool_free(&i, node3), node3=NULL;
	harbol_mempool_free(&i, node4), node4=NULL;
	harbol_mempool_free(&i, node5), node5=NULL;
	harbol_mempool_free(&i, node6), node6=NULL;
	__print_mempool_nodes(&i);
	
	// test "double freeing"
	fputs("\nmempool :: test double freeing.\n", g_harbol_debug_stream);
	p = harbol_mempool_alloc(&i, sizeof *p);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
	if( p ) {
		*p = 500;
		fprintf(g_harbol_debug_stream, "p's value: %i\n", *p);
	}
	harbol_mempool_free(&i, p);
	harbol_mempool_free(&i, p);
	harbol_mempool_cleanup(&i, (void**)&p);
	
	fprintf(g_harbol_debug_stream, "\nmempool :: pool size == %zu.\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
	
	float *hk = harbol_mempool_alloc(&i, sizeof *hk * 99);
	double *fg = harbol_mempool_alloc(&i, sizeof *fg * 10);
	char *fff = harbol_mempool_alloc(&i, sizeof *fff * 50);
	float *f32 = harbol_mempool_alloc(&i, sizeof *f32 * 23);
	char *jj = harbol_mempool_alloc(&i, sizeof *jj * 100);
	struct HarbolMemNode *ac = harbol_mempool_alloc(&i, sizeof *ac * 31);
	harbol_mempool_free(&i, fff);
	harbol_mempool_free(&i, fg);
	harbol_mempool_free(&i, ac);
	harbol_mempool_free(&i, f32);
	fprintf(g_harbol_debug_stream, "\nmempool :: pool size == %zu.\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
	fprintf(g_harbol_debug_stream, "mempool :: heap bottom (%zu).\n", (uintptr_t)i.stack.base);
	
	harbol_mempool_free(&i, hk);
	fprintf(g_harbol_debug_stream, "\ncrazy mempool :: pool size == %zu.\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
		
	harbol_mempool_free(&i, jj);
	
	fprintf(g_harbol_debug_stream, "\nlast mempool :: pool size == %zu.\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
	//fprintf(g_harbol_debug_stream, "mempool :: heap bottom (%zu).\n", (uintptr_t)i.stack.base);
	
	
	fprintf(g_harbol_debug_stream, "\nmempool :: pool size == %zu.\n", harbol_mempool_mem_remaining(&i));
	__print_mempool_nodes(&i);
	harbol_mempool_defrag(&i);
	fputs("\n", g_harbol_debug_stream);
	__print_mempool_nodes(&i);
	
	fputs("\nmempool :: test reallocating jj to a single value.\n", g_harbol_debug_stream);
	jj = harbol_mempool_alloc(&i, sizeof *jj);
	*jj = 50;
	fprintf(g_harbol_debug_stream, "mempool :: jj == %i.\n", *jj);
	
	int *newer = harbol_mempool_realloc(&i, jj, sizeof *newer);
	fputs("\nmempool :: test reallocating jj to int ptr 'newer'.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "mempool :: newer == %i.\n", *newer);
	
	jj = harbol_mempool_realloc(&i, newer, sizeof *jj);
	fputs("\nmempool :: test reallocating newer back to jj.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "mempool :: jj == %i.\n", *jj);
	
	newer = harbol_mempool_realloc(&i, jj, sizeof *newer * 10);
	fputs("\nmempool :: test reallocating jj back to newer as an array of int[10].\n", g_harbol_debug_stream);
	for( uindex_t i=0; i<10; i++ ) {
		newer[i] = i+1;
		fprintf(g_harbol_debug_stream, "mempool :: newer[%zu] == %i.\n", i, newer[i]);
	}
	fputs("\n", g_harbol_debug_stream);
	newer = harbol_mempool_realloc(&i, newer, sizeof *newer * 5);
	for( uindex_t i=0; i<5; i++ )
		fprintf(g_harbol_debug_stream, "mempool :: reallocated newer[%zu] == %i.\n", i, newer[i]);
	harbol_mempool_free(&i, newer);
	
	const clock_t end = clock();
	printf("memory pool run time: %f\n", (end-start)/(double)CLOCKS_PER_SEC);
	// free data
	fputs("\nmempool :: test destruction.\n", g_harbol_debug_stream);
	harbol_mempool_clear(&i);
	fprintf(g_harbol_debug_stream, "i's heap is null? '%s'\n", i.stack.mem ? "no" : "yes");
	fprintf(g_harbol_debug_stream, "i's freelist is null? '%s'\n", i.freelist.head ? "no" : "yes");
}

void test_harbol_objpool(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("objpool :: test allocation/initialization.\n", g_harbol_debug_stream);
	
	struct HarbolObjPool i = harbol_objpool_create(sizeof(union Value), 5);
	fprintf(g_harbol_debug_stream, "remaining object pool mem: '%zu'\n", i.free_blocks);
	
	fputs("\nobjpool :: test allocing values.\n", g_harbol_debug_stream);
	union Value *valtable[5] = {NULL};
	for( uindex_t n=0; n<5; n++ ) {
		valtable[n] = harbol_objpool_alloc(&i);
		valtable[n]->int64 = n + 1;
		fprintf(g_harbol_debug_stream, "valtable[%zu]: %" PRIi64 "\n", n, valtable[n]->int64);
		fprintf(g_harbol_debug_stream, "post-allocation remaining object pool mem: '%zu'\n", i.free_blocks);
	}
	
	fputs("\nobjpool :: test freeing values.\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<5; n++ ) {
		harbol_objpool_free(&i, valtable[n]);
		fprintf(g_harbol_debug_stream, "post-free remaining object pool mem: '%zu'\n", i.free_blocks);
	}
	for( uindex_t n=0; n<5; n++ ) {
		valtable[n] = harbol_objpool_alloc(&i);
		valtable[n]->int64 = n + 1;
		fprintf(g_harbol_debug_stream, "valtable[%zu]: %" PRIi64 "\n", n, valtable[n]->int64);
		fprintf(g_harbol_debug_stream, "post-allocation remaining object pool mem: '%zu'\n", i.free_blocks);
	}
	
	// free data
	fputs("\nobjpool :: test destruction.\n", g_harbol_debug_stream);
	harbol_objpool_clear(&i);
	fprintf(g_harbol_debug_stream, "i's heap is null? '%s'\n", i.mem ? "no" : "yes");
	fprintf(g_harbol_debug_stream, "i's next is null? '%s'\n", i.next ? "no" : "yes");
}

void test_harbol_cache(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("cache :: test allocation/initialization.\n", g_harbol_debug_stream);
	
	struct HarbolCache i = harbol_cache_create(sizeof(union Value) * 10);
	fprintf(g_harbol_debug_stream, "remaining cache mem: '%zu'\n", harbol_cache_remaining(&i));
	
	fputs("\ncache :: test allocing values.\n", g_harbol_debug_stream);
	union Value *valtable[10] = {NULL};
	for( uindex_t n=0; n<10; n++ ) {
		valtable[n] = harbol_cache_alloc(&i, sizeof *valtable[0]);
		valtable[n]->int64 = n + 1;
		fprintf(g_harbol_debug_stream, "valtable[%zu]: %" PRIi64 "\nremaining cache mem: '%zu'\n", n, valtable[n]->int64, harbol_cache_remaining(&i));
	}
	
	fputs("\ncache :: test allocing differently sized values.\n", g_harbol_debug_stream);
	harbol_cache_clear(&i);
	i = harbol_cache_create(1000);
	
	// on creation, the offset ptr for the cache points to invalid memory.
	fprintf(g_harbol_debug_stream, "pre-alloc offset ptr: '%" PRIuPTR "'\n", (uintptr_t)i.offset);
	
	float32_t *f = harbol_cache_alloc(&i, sizeof *f);
	*f = 32.f;
	fprintf(g_harbol_debug_stream, "post-alloc offset ptr: '%" PRIuPTR "'\n", (uintptr_t)i.offset);
	
	
	float64_t (*v)[3] = harbol_cache_alloc(&i, sizeof *v);
	
	(*v)[0] = 3.;
	(*v)[1] = 5.;
	(*v)[2] = 10.;
	fprintf(g_harbol_debug_stream, "remaining cache mem: '%zu'\nf value: %" PRIf32 "\nvec values: { %" PRIf64 ", %" PRIf64 ", %" PRIf64 " } | is aligned? %u\n", harbol_cache_remaining(&i), *f, (*v)[0], (*v)[1], (*v)[2], is_aligned(v, sizeof(uintptr_t)));
	
	// free data
	fputs("\ncache :: test destruction.\n", g_harbol_debug_stream);
	harbol_cache_clear(&i);
	fprintf(g_harbol_debug_stream, "i's base is null? '%s'\n", i.base ? "no" : "yes");
	
}

void test_harbol_graph(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("graph :: test allocation/initialization.\n", g_harbol_debug_stream);
	
	struct HarbolGraph g = harbol_graph_create(sizeof(union Value), sizeof(union Value));
	struct HarbolGraph *p = harbol_graph_new(sizeof(union Value), sizeof(union Value));
	if( p )
		fputs("allocation/initialization of p is GOOD.\n", g_harbol_debug_stream);
	
	// Test adding vertices
	fputs("\ngraph :: test adding vertices.\n", g_harbol_debug_stream);
	
	for( size_t c=0; c<5; c++ ) {
		harbol_graph_add_vert(&g, &(union Value){.int64=c+1});
		harbol_graph_add_vert(p, &(union Value){.int64=c+1});
	}
	for( uindex_t i=0; i<g.vertices.count; i++ ) {
		struct HarbolVertex *vert = harbol_graph_get_vert(&g, i);
		fprintf(g_harbol_debug_stream, "Vertex data: '%" PRIi64 "'\n", ((union Value *)vert->data)->int64);
	}
	// test linking two vertices
	fputs("\ngraph :: test linking vertices.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "linking was success?: '%u'\n", harbol_graph_index_add_edge(&g, 0, 1, &(union Value){0}));
	fprintf(g_harbol_debug_stream, "Edge count in graph: '%zu'\n", harbol_graph_edges(&g));
	
	for( uindex_t i=0; i<g.vertices.count; i++ ) {
		struct HarbolVertex *vert = harbol_graph_get_vert(&g, i);
		fprintf(g_harbol_debug_stream, "Vertex data: '%" PRIi64 "'\n", ((union Value *)vert->data)->int64);
		for( uindex_t n=0; n<vert->edges.count; n++ ) {
			struct HarbolEdge *k = harbol_vertex_get_edge(vert, n);
			fprintf(g_harbol_debug_stream, "Edge data: '%" PRIi64 "'\n", ((union Value *)k->weight)->int64);
			if( k->link >= 0 )
				fprintf(g_harbol_debug_stream, "Vertex link data: '%" PRIi64 "'\n", ((union Value *)harbol_graph_get_vert(&g, k->link)->data)->int64);
		}
	}
	
	// test unlinking the two previous vertices
	fputs("\ngraph :: test unlinking the two previous vertices.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "unlinking was success?: '%u'\n", harbol_graph_index_del_edge(&g, 0, 1, NULL));
	fprintf(g_harbol_debug_stream, "Edge count in graph: '%zu'\n", harbol_graph_edges(&g));
	
	// test linking ALL available vertices!
	fputs("\ngraph :: test linking all vertices in graph.\n", g_harbol_debug_stream);
	// link 1 to 2
	harbol_graph_index_add_edge(&g, 0, 1, &(union Value){5});
	// link 2 to 3
	harbol_graph_index_add_edge(&g, 1, 2, &(union Value){2});
	// link 3 to 4
	harbol_graph_index_add_edge(&g, 2, 3, &(union Value){24});
	// link 4 to 5
	harbol_graph_index_add_edge(&g, 3, 4, &(union Value){62});
	// link 5 to 3
	harbol_graph_index_add_edge(&g, 4, 2, &(union Value){77});
	fprintf(g_harbol_debug_stream, "Edge count in graph: '%zu'\n", harbol_graph_edges(&g));
	
	for( uindex_t i=0; i<g.vertices.count; i++ ) {
		struct HarbolVertex *vert = harbol_graph_get_vert(&g, i);
		fprintf(g_harbol_debug_stream, "Vertex data: '%" PRIi64 "'\n", ((union Value *)vert->data)->int64);
		for( uindex_t n=0; n<vert->edges.count; n++ ) {
			struct HarbolEdge *k = harbol_vertex_get_edge(vert, n);
			fprintf(g_harbol_debug_stream, "Edge data: '%" PRIi64 "'\n", ((union Value *)k->weight)->int64);
			if( k->link >= 0 )
				fprintf(g_harbol_debug_stream, "Edge Vertex link data: '%" PRIi64 "'\n", ((union Value *)harbol_graph_get_vert(&g, k->link)->data)->int64);
		}
	}
	fprintf(g_harbol_debug_stream, "\nRemoving 5th value success?: '%u'\n", harbol_graph_del_index(&g, 4, NULL, NULL));
	for( uindex_t i=0; i<g.vertices.count; i++ ) {
		struct HarbolVertex *vert = harbol_graph_get_vert(&g, i);
		fprintf(g_harbol_debug_stream, "Vertex data: '%" PRIi64 "'\n", ((union Value *)vert->data)->int64);
		for( uindex_t n=0; n<vert->edges.count; n++ ) {
			struct HarbolEdge *k = harbol_vertex_get_edge(vert, n);
			fprintf(g_harbol_debug_stream, "Edge data: '%" PRIi64 "'\n", ((union Value *)k->weight)->int64);
			if( k->link >= 0 )
				fprintf(g_harbol_debug_stream, "Edge Vertex link data: '%" PRIi64 "'\n", ((union Value *)harbol_graph_get_vert(&g, k->link)->data)->int64);
		}
	}
	
	// test changing vertex data through an index!
	fprintf(g_harbol_debug_stream, "\nTest changing vertex data by index\n");
	harbol_graph_set_index(&g, 0, &(union Value){.int64 = 100});
	for( size_t i=0; i<g.vertices.count; i++ ) {
		struct HarbolVertex *vert = harbol_graph_get_vert(&g, i);
		fprintf(g_harbol_debug_stream, "Vertex data: '%" PRIi64 "'\n", ((union Value *)vert->data)->int64);
		for( size_t n=0; n<vert->edges.count; n++ ) {
			struct HarbolEdge *k = harbol_vertex_get_edge(vert, n);
			fprintf(g_harbol_debug_stream, "Edge data: '%" PRIi64 "'\n", ((union Value *)k->weight)->int64);
			if( k->link >= 0 )
				fprintf(g_harbol_debug_stream, "Vertex link data: '%" PRIi64 "'\n", ((union Value *)harbol_graph_get_vert(&g, k->link)->data)->int64);
		}
	}
	
	// test getting edge pointer
	fputs("\ngraph :: test getting edge pointer.\n", g_harbol_debug_stream);
	struct HarbolEdge *edge = harbol_graph_index_get_edge(&g, 0, 0);
	if( edge ) {
		fputs("edge ptr is VALID.\n", g_harbol_debug_stream);
		if( edge->link >= 0 )
			fprintf(g_harbol_debug_stream, "edge Vertex link data: '%" PRIi64 "'\n", ((union Value *)harbol_graph_get_vert(&g, edge->link)->data)->int64);
	}
	
	// test adjacency function
	fprintf(g_harbol_debug_stream, "\nindex 0 is adjacent to index 1?: '%u'\n", harbol_graph_indices_adjacent(&g, 0, 1));
	
	// free data
	fputs("\ngraph :: test destruction.\n", g_harbol_debug_stream);
	harbol_graph_clear(&g, NULL, NULL);
	fprintf(g_harbol_debug_stream, "i's vertices vector is null? '%s'\n", g.vertices.table ? "no" : "yes");
	
	harbol_graph_clear(p, NULL, NULL);
	fprintf(g_harbol_debug_stream, "p's vertices vector is null? '%s'\n", p->vertices.table ? "no" : "yes");
	harbol_graph_free(&p, NULL, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

static void __free_string(void **tree_data) {
	struct HarbolString **data = (struct HarbolString **)tree_data;
	harbol_string_free(data);
}

void test_harbol_tree(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("tree :: test allocation/initialization.\n", g_harbol_debug_stream);
	const size_t u_val_size = sizeof(union Value);
	struct HarbolTree i = harbol_tree_create(&(union Value){.int64=1}, u_val_size);
	struct HarbolTree *p = harbol_tree_new(&(union Value){.int64=1}, u_val_size);
	if( p )
		fputs("graph :: allocation/initialization of p is GOOD.\n", g_harbol_debug_stream);
	
	
	// Test insertions
	fputs("\ntree :: test insertions.\n", g_harbol_debug_stream);
	struct HarbolTree t = harbol_tree_create(&(union Value){.int64=20}, u_val_size);
	fprintf(g_harbol_debug_stream, "insertion by node success?: '%u'\n", harbol_tree_insert_child_node(p, &t));
	fprintf(g_harbol_debug_stream, "insertion by value success?: '%u'\n", harbol_tree_insert_child_val(p, &(union Value){.int64=10}, u_val_size));
	
	harbol_tree_clear(p, NULL);
	
	// Test deletions
	fputs("\ntree :: test deletions by first adding 5 children.\n", g_harbol_debug_stream);
	harbol_tree_insert_child_val(p, &(union Value){.int64=1}, u_val_size);
	harbol_tree_insert_child_val(p, &(union Value){.int64=2}, u_val_size);
	harbol_tree_insert_child_val(p, &(union Value){.int64=3}, u_val_size);
	harbol_tree_insert_child_val(p, &(union Value){.int64=4}, u_val_size);
	harbol_tree_insert_child_val(p, &(union Value){.int64=5}, u_val_size);
	for( uindex_t n=0; n<p->children.count; n++ ) {
		struct HarbolTree *child = harbol_tree_index_get_child(p, n);
		fprintf(g_harbol_debug_stream, "child #%zu value: '%" PRIi64 "'\n", n, ((union Value *)child->data)->int64);
	}
	fputs("\ndeleting index 1\n", g_harbol_debug_stream);
	harbol_tree_del_child_index(p, 1, NULL);
	for( uindex_t n=0; n<p->children.count; n++ ) {
		struct HarbolTree *child = harbol_tree_index_get_child(p, n);
		fprintf(g_harbol_debug_stream, "child #%zu value: '%" PRIi64 "'\n", n, ((union Value *)child->data)->int64);
	}
	// Test delete by node reference
	fputs("\ntree :: test deletion by node reference.\n", g_harbol_debug_stream);
	// delete first child!
	fputs("\ndeleting index 0\n", g_harbol_debug_stream);
	harbol_tree_del_child_node(p, harbol_tree_index_get_child(p, 0), NULL);
	for( uindex_t n=0; n<p->children.count; n++ ) {
		struct HarbolTree *child = harbol_tree_index_get_child(p, n);
		fprintf(g_harbol_debug_stream, "child #%zu value: '%" PRIi64 "'\n", n, ((union Value *)child->data)->int64);
	}
	// Test creating something of an abstract syntax tree.
	fputs("\ntree :: test creating something of an abstract syntax tree.\n", g_harbol_debug_stream);
	harbol_tree_clear(p, NULL);
	const size_t hstr_size = sizeof(struct HarbolString);
	
	struct HarbolString str = harbol_string_create("program");
	harbol_tree_set(p, &str, hstr_size);
	
	str = harbol_string_create("stmt");
	harbol_tree_insert_child_val(p, &str, hstr_size);
	
	str = harbol_string_create("if");
	harbol_tree_insert_child_val(harbol_tree_index_get_child(p, 0), &str, hstr_size);
	
	str = harbol_string_create("cond");
	harbol_tree_insert_child_val(harbol_tree_index_get_child(p, 0), &str, hstr_size);
	
	str = harbol_string_create("stmt");
	harbol_tree_insert_child_val(harbol_tree_index_get_child(p, 0), &str, hstr_size);
	
	str = harbol_string_create("else");
	harbol_tree_insert_child_val(harbol_tree_index_get_child(p, 0), &str, hstr_size);
	
	fprintf(g_harbol_debug_stream, "p's data: '%s'\n", ((struct HarbolString *)p->data)->cstr);
	struct HarbolTree *kid = harbol_tree_index_get_child(p, 0);
	fprintf(g_harbol_debug_stream, "p's child data: '%s'\n", ((struct HarbolString *)kid->data)->cstr);
	for( uindex_t n=0; n<kid->children.count; n++ ) {
		struct HarbolTree *child = harbol_tree_index_get_child(kid, n);
		fprintf(g_harbol_debug_stream, "p's child's children data: '%s'\n", ((struct HarbolString *)child->data)->cstr);
	}
	
	fprintf(g_harbol_debug_stream, "\nfreeing string data. %u\n", harbol_tree_clear(p, &__free_string)); //(void(*)(void**))harbol_string_free));
	
	// free data
	fputs("\ntree :: test destruction.\n", g_harbol_debug_stream);
	harbol_tree_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's children vector is null? '%s'\n", i.children.table ? "no" : "yes");
	
	harbol_tree_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's children vector is null? '%s'\n", p->children.table ? "no" : "yes");
	harbol_tree_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_linkmap(void)
{
	// Test allocation and initializations
	fputs("linkmap :: test allocation / initialization.\n", g_harbol_debug_stream);
	struct HarbolLinkMap i = harbol_linkmap_create(sizeof(union Value));
	struct HarbolLinkMap *p = harbol_linkmap_new(sizeof(union Value));
	assert( p );
	
	// test insertion
	fputs("\nlinkmap :: test insertion.\n", g_harbol_debug_stream);
	harbol_linkmap_insert(p, "1", &(union Value){.int64=1});
	harbol_linkmap_insert(p, "2", &(union Value){.int64=2});
	harbol_linkmap_insert(p, "3", &(union Value){.int64=3});
	harbol_linkmap_insert(p, "4", &(union Value){.int64=4});
	harbol_linkmap_insert(p, "5", &(union Value){.int64=5});
	harbol_linkmap_insert(p, "6", &(union Value){.int64=6});
	harbol_linkmap_insert(p, "7", &(union Value){.int64=7});
	harbol_linkmap_insert(p, "8", &(union Value){.int64=8});
	harbol_linkmap_insert(p, "9", &(union Value){.int64=9});
	harbol_linkmap_insert(p, "10", &(union Value){.int64=10});
	
	harbol_linkmap_insert(&i, "1", &(union Value){.int64=1});
	harbol_linkmap_insert(&i, "2", &(union Value){.int64=2});
	harbol_linkmap_insert(&i, "3", &(union Value){.int64=3});
	harbol_linkmap_insert(&i, "4", &(union Value){.int64=4});
	harbol_linkmap_insert(&i, "5", &(union Value){.int64=5});
	harbol_linkmap_insert(&i, "6", &(union Value){.int64=6});
	harbol_linkmap_insert(&i, "7", &(union Value){.int64=7});
	harbol_linkmap_insert(&i, "8", &(union Value){.int64=8});
	harbol_linkmap_insert(&i, "9", &(union Value){.int64=9});
	harbol_linkmap_insert(&i, "10", &(union Value){.int64=10});
	
	// test retrieval.
	fputs("\nlinkmap :: test data retrieval.\n", g_harbol_debug_stream);
	fprintf(g_harbol_debug_stream, "ptr[\"1\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(p, "1"))->int64);
	fprintf(g_harbol_debug_stream, "ptr[\"2\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(p, "2"))->int64);
	fprintf(g_harbol_debug_stream, "stk[\"1\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(&i, "1"))->int64);
	fprintf(g_harbol_debug_stream, "stk[\"2\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(&i, "2"))->int64);
	
	fputs("\nlinkmap :: looping through all data.\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(&i, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	// test setting.
	fputs("\nlinkmap :: test data setting.\n", g_harbol_debug_stream);
	harbol_linkmap_key_set(p, "2", &(union Value){.int64=20});
	harbol_linkmap_key_set(&i, "2", &(union Value){.int64=200});
	fprintf(g_harbol_debug_stream, "ptr[\"1\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(p, "1"))->int64);
	fprintf(g_harbol_debug_stream, "ptr[\"2\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(p, "2"))->int64);
	fprintf(g_harbol_debug_stream, "stk[\"1\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(&i, "1"))->int64);
	fprintf(g_harbol_debug_stream, "stk[\"2\"] == %" PRIi64 "\n", ((union Value *)harbol_linkmap_key_get(&i, "2"))->int64);
	fputs("\nlinkmap :: looping through all data.\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<i.vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(&i, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	fputs("\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<p->vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(p, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	
	// test deletion
	fputs("\nlinkmap :: test item deletion.\n", g_harbol_debug_stream);
	harbol_linkmap_key_del(p, "2", NULL);
	fprintf(g_harbol_debug_stream, "ptr[\"2\"] == %p\n", harbol_linkmap_key_get(p, "2"));
	fputs("\nlinkmap :: looping through all data.\n", g_harbol_debug_stream);
	for( uindex_t n=0; n<p->vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(p, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	fputs("\nlinkmap :: test item deletion by index.\n", g_harbol_debug_stream);
	harbol_linkmap_index_del(p, 2, NULL);
	for( uindex_t n=0; n<p->vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(p, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	// test setting by index
	fputs("\nlinkmap :: test item setting by index.\n", g_harbol_debug_stream);
	harbol_linkmap_index_set(p, 2, &(union Value){.int64=500});
	for( uindex_t n=0; n<p->vec.count; n++ ) {
		struct HarbolKeyVal *l = harbol_linkmap_index_get_kv(p, n);
		fprintf(g_harbol_debug_stream, "l's value == %" PRIi64 "\n", ((union Value *)l->data)->int64);
	}
	// free data
	fputs("\nlinkmap :: test destruction.\n", g_harbol_debug_stream);
	harbol_linkmap_clear(&i, NULL);
	fprintf(g_harbol_debug_stream, "i's buckets are null? '%s'\n", i.vec.table ? "no" : "yes");
	
	harbol_linkmap_clear(p, NULL);
	fprintf(g_harbol_debug_stream, "p's buckets are null? '%s'\n", p->vec.table ? "no" : "yes");
	harbol_linkmap_free(&p, NULL);
	fprintf(g_harbol_debug_stream, "p is null? '%s'\n", p ? "no" : "yes");
}

void test_harbol_cfg(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	// Test allocation and initializations
	fputs("cfg :: test allocation/initialization.\n", g_harbol_debug_stream);
	struct HarbolLinkMap *cfg = harbol_cfg_parse_cstr("'section': { 'lel': null }");
	fprintf(g_harbol_debug_stream, "cfg ptr valid?: '%s'\n", cfg ? "yes" : "no");
	
	if( cfg ) {
		fputs("\ncfg :: testing config to string conversion.\n", g_harbol_debug_stream);
		struct HarbolString stringcfg = harbol_cfg_to_str(cfg);
		fprintf(g_harbol_debug_stream, "\ncfg :: \n%s\n", stringcfg.cstr);
		harbol_string_clear(&stringcfg);
	}
	
	// Test realistic config file string
	fputs("cfg :: test realistic config file string.\n", g_harbol_debug_stream);
	const char *test_cfg = "'root': { \
		'firstName': 'John', \
		'lastName': 'Smith', \
		'isAlive': true, \
		'age': 0x18 , \
		'money': 35.42e4 \
		'address': { \
			'streetAddress': '21 2nd Street', \
			'city': 'New York', \
			'state': 'NY', \
			'postalCode': '10021-3100' \
		}, \
		'phoneNumbers.': { \
			'1' { \
				'type': 'home \\x5c', \
				'number': '212 555-1234' \
			}, \
			'2' { \
				'type': 'office', \
				'number': '646 555-4567' \
			}, \
			'3' { \
				'type': 'mobile', \
				'number': '123 456-7890' \
			} \
		}, \
		'colors': c[ 0xff, 0xff, 0xff, 0xaa ], \
		'origin': v[10.0f, 24.43, 25.0, 0xA.p+2], \
		'children': {}, \
		'spouse': null \
		'test_iota': { \
		    '1': iota \
		    '2': { \
		        'a': iota \
		        'b': Iota \
		        'c': iota \
		        'd': iota \
		        'e': iota \
		    } \
		    '3': iota \
		    '4': iota \
		    '5': iota \
		    '6': Iota \
		    '7': Iota \
		} \
		'test_iota_again': { \
		    '1': iota \
		    '2': iota \
		    '3': iota \
		    '4': iota \
		    '5': iota \
		    '6': Iota \
		    '7': Iota \
		} \
	}";
	const clock_t start = clock();
	struct HarbolLinkMap *larger_cfg = harbol_cfg_parse_cstr(test_cfg);
	const clock_t end = clock();
	printf("cfg parsing time: %f\n", (end-start)/(double)CLOCKS_PER_SEC);
	
	fprintf(g_harbol_debug_stream, "larger_cfg ptr valid?: '%s'\n", larger_cfg ? "yes" : "no");
	if( larger_cfg ) {
		fputs("\ncfg :: iterating realistic config.\n", g_harbol_debug_stream);
		struct HarbolString stringcfg = harbol_cfg_to_str(larger_cfg);
		fprintf(g_harbol_debug_stream, "\ncfg :: test config to string conversion:\n%s\n", stringcfg.cstr);
		harbol_string_clear(&stringcfg);
		
		fputs("\ncfg :: test retrieving sub section of realistic config.\n", g_harbol_debug_stream);
		struct HarbolLinkMap *phone_numbers1 = harbol_cfg_get_section(larger_cfg, "root.phoneNumbers/..1");
		stringcfg = harbol_cfg_to_str(phone_numbers1);
		fprintf(g_harbol_debug_stream, "\nphone_numbers to string conversion: \n%s\n", stringcfg.cstr);
		harbol_string_clear(&stringcfg);
		if( phone_numbers1 ) {
			fputs("\ncfg :: iterating phone_numbers1 subsection.\n", g_harbol_debug_stream);
			struct HarbolString stringcfg = harbol_cfg_to_str(phone_numbers1);
			fprintf(g_harbol_debug_stream, "\nphone_numbers1 to string conversion: \n%s\n", stringcfg.cstr);
			harbol_string_clear(&stringcfg);
		}
		
		fputs("\ncfg :: test retrieving string value from subsection of realistic config.\n", g_harbol_debug_stream);
		char *type = harbol_cfg_get_cstr(larger_cfg, "root.phoneNumbers/..1.type");
		fprintf(g_harbol_debug_stream, "root.phoneNumbers/..1.type type valid?: '%s'\n", type ? "yes" : "no");
		if( type ) {
			fprintf(g_harbol_debug_stream, "root.phoneNumbers/..1.type: %s\n", type);
		}
		type = harbol_cfg_get_cstr(larger_cfg, "root.age");
		fprintf(g_harbol_debug_stream, "root.age string type valid?: '%s'\n", type ? "yes" : "no");
		if( type ) {
			fprintf(g_harbol_debug_stream, "root.age: %s\n", type);
		}
		intmax_t *age = harbol_cfg_get_int(larger_cfg, "root.age");
		if( age )
			fprintf(g_harbol_debug_stream, "root.age int?: '%" PRIiMAX "'\n", *age);
		floatmax_t *money = harbol_cfg_get_float(larger_cfg, "root.money");
		if( money )
			fprintf(g_harbol_debug_stream, "root.money float?: '%" PRIfMAX "'\n", *money);
		
		union HarbolColor *color = harbol_cfg_get_color(larger_cfg, "root.colors");
		if( color )
			fprintf(g_harbol_debug_stream, "root.colors: '[%u, %u, %u, %u]'\n", color->bytes.r, color->bytes.g, color->bytes.b, color->bytes.a);
		
		fputs("\ncfg :: test override setting an existing key-value from null to a string type.\n", g_harbol_debug_stream);
		harbol_cfg_set_cstr(larger_cfg, "root.spouse", "Jane Smith", true);
		{
			struct HarbolString stringcfg = harbol_cfg_to_str(larger_cfg);
			fprintf(g_harbol_debug_stream, "\nadded spouse!: \n%s\n", stringcfg.cstr);
			harbol_string_clear(&stringcfg);
		}
		
		fputs("\ncfg :: test building cfg file!\n", g_harbol_debug_stream);
		fprintf(g_harbol_debug_stream, "\nconfig construction result: '%s'\n", harbol_cfg_build_file(larger_cfg, "large_cfg.ini", true) ? "success" : "failure");
		
		fputs("\ncfg :: test setting a key back to null\n", g_harbol_debug_stream);
		harbol_cfg_set_to_null(larger_cfg, "root.spouse");
		{
			struct HarbolString stringcfg = harbol_cfg_to_str(larger_cfg);
			fprintf(g_harbol_debug_stream, "\nremoved spouse!: \n%s\n", stringcfg.cstr);
			harbol_string_clear(&stringcfg);
		}
		
		fputs("\ncfg :: test getting the type of a key\n", g_harbol_debug_stream);
		fprintf(g_harbol_debug_stream, "Type of root.phoneNumbers/.: %i\n", harbol_cfg_get_type(larger_cfg, "root.phoneNumbers/."));
		fprintf(g_harbol_debug_stream, "Type of root.spouse: %i\n", harbol_cfg_get_type(larger_cfg, "root.spouse"));
		fprintf(g_harbol_debug_stream, "Type of root.money: %i\n", harbol_cfg_get_type(larger_cfg, "root.money"));
		fprintf(g_harbol_debug_stream, "Type of root.origin: %i\n", harbol_cfg_get_type(larger_cfg, "root.origin"));
		
		fputs("\ncfg :: test adding other cfg as a new section\n", g_harbol_debug_stream);
		{
			struct HarbolVariant var = harbol_variant_create(&cfg, sizeof(struct HarbolLinkMap *), HarbolCfgType_Linkmap);
			harbol_linkmap_insert(larger_cfg, "former lovers", &var);
			struct HarbolString stringcfg = harbol_cfg_to_str(larger_cfg);
			fprintf(g_harbol_debug_stream, "\nremoved spouse!: \n%s\n", stringcfg.cstr);
			harbol_string_clear(&stringcfg);
		}
		fputs("\ncfg :: test building newer cfg file!\n", g_harbol_debug_stream);
		fprintf(g_harbol_debug_stream, "\nconfig construction result: '%s'\n", harbol_cfg_build_file(larger_cfg, "large_cfg_new_sect.ini", true) ? "success" : "failure");
		harbol_cfg_free(&larger_cfg);
		fprintf(g_harbol_debug_stream, "cfg ptr valid?: '%s'\n", cfg ? "yes" : "no");
	}
	cfg = NULL;
	fputs("\ncfg :: test destruction.\n", g_harbol_debug_stream);
	harbol_cfg_free(&cfg);
	fprintf(g_harbol_debug_stream, "cfg ptr valid?: '%s'\n", cfg ? "yes" : "no");
}


typedef int32_t __Loader(void);
static void on_plugin_load(struct HarbolPluginMod *const mod, struct HarbolPlugin *const plugin)
{
	size_t *const restrict plugin_counts = harbol_plugin_mod_userdata(mod);
	++*plugin_counts;
	/* no way manager nor the plugin reference would be invalid... */
	struct HarbolString load_func_name = {NULL,0};
	harbol_string_format(&load_func_name, "%s_load", harbol_plugin_name(plugin));
	fprintf(g_harbol_debug_stream, "\nplugin mod :: on_plugin_load - plugin name :: '%s'\n", harbol_plugin_name(plugin));
	
	fprintf(g_harbol_debug_stream, "on_plugin_load :: getting function '%s'\n", load_func_name.cstr);
	__Loader *const onload = (__Loader *)(intptr_t)harbol_plugin_sym(plugin, load_func_name.cstr);
	fprintf(g_harbol_debug_stream, "on_plugin_load :: function ptr valid? '%s'\n", onload ? "yes" : "no");
	if( onload )
		fprintf(g_harbol_debug_stream, "on_plugin_load :: function return value: '%i'\n", (*onload)());
	
	harbol_string_clear(&load_func_name);
}

typedef void __Unloader(void);
static void on_plugin_unload(struct HarbolPluginMod *const mod, struct HarbolPlugin *const plugin)
{
	size_t *const restrict plugin_counts = harbol_plugin_mod_userdata(mod);
	--*plugin_counts;
	/* no way manager nor the plugin reference would be invalid... */
	fprintf(g_harbol_debug_stream, "\nplugin mod :: on_plugin_unload - plugin name :: '%s'\n", harbol_plugin_name(plugin));
	
	struct HarbolString unload_func_name = {NULL,0};
	harbol_string_format(&unload_func_name, "%s_unload", harbol_plugin_name(plugin));
	
	__Unloader *const onunload = (__Unloader *)(intptr_t)harbol_plugin_sym(plugin, unload_func_name.cstr);
	fprintf(g_harbol_debug_stream, "on_plugin_unload :: getting function '%s'\n", unload_func_name.cstr);
	fprintf(g_harbol_debug_stream, "on_plugin_unload :: function ptr valid? '%s'\n", onunload ? "yes" : "no");
	if( onunload )
		(*onunload)();
	
	harbol_string_clear(&unload_func_name);
}

void test_harbol_plugins(void)
{
	if( !g_harbol_debug_stream )
		return;
	fputs("plugin mod :: test init.\n", g_harbol_debug_stream);
	size_t plugin_counts = 0;
	struct HarbolPluginMod pm = harbol_plugin_mod_create("test_harbol_plugins/", &plugin_counts, true, on_plugin_load);
	fprintf(g_harbol_debug_stream, "\nplugin mod :: initialization good?: '%s' | plugins count: %zu\n", pm.plugins.vec.count>0 ? "yes" : "no", plugin_counts);
	
	fputs("\nplugin mod :: test deleting plugin by name.\n", g_harbol_debug_stream);
	harbol_plugin_mod_name_del_plugin(&pm, "test_plugin", on_plugin_unload);
	
	fputs("\nplugin mod :: test reloading all loaded plugins.\n", g_harbol_debug_stream);
	harbol_plugin_mod_reload_plugins(&pm, NULL, NULL);
	
	fputs("\nplugin mod :: test destruction.\n", g_harbol_debug_stream);
	harbol_plugin_mod_clear(&pm, on_plugin_unload);
	fprintf(g_harbol_debug_stream, "\nplugin mod :: plugins count: %zu\n", plugin_counts);
}


void test_harbol_veque(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	fputs("veque :: test init.\n", g_harbol_debug_stream);
	struct HarbolVeque i = harbol_veque_create(sizeof(union Value), 10);
	struct HarbolVeque *p = harbol_veque_new(sizeof(union Value), 10);
	
	fprintf(g_harbol_debug_stream, "\nveque :: initialization good?: '%s'\n", p->table != NULL ? "yes" : "no");
	
	fputs("\nveque :: test insertions to front and back.\n", g_harbol_debug_stream);
	harbol_veque_insert_front(&i, &(union Value){2});
	harbol_veque_insert_front(p, &(union Value){2});
	
	harbol_veque_insert_back(&i, &(union Value){1});
	harbol_veque_insert_back(p, &(union Value){1});
	
	union Value *v1=harbol_veque_get_front(&i);
	fprintf(g_harbol_debug_stream, "\nveque :: v1: '%" PRIi64 "'\n", v1->int64);
	
	union Value *v2=harbol_veque_get_back(&i);
	fprintf(g_harbol_debug_stream, "\nveque :: v2: '%" PRIi64 "'\n", v2->int64);
	
	v2--;
	fprintf(g_harbol_debug_stream, "\nveque :: *--v2: '%" PRIi64 "'\n", v2->int64);
	
	fprintf(g_harbol_debug_stream, "\nveque :: v1 (%" PRIuPTR ") , v2 (%" PRIuPTR ")\n", (uintptr_t)v1, (uintptr_t)++v2);
	fprintf(g_harbol_debug_stream, "\nveque :: count: %zu\n", harbol_veque_count(p));
	
	
	fputs("\nveque :: test destruction.\n", g_harbol_debug_stream);
	harbol_veque_clear(&i, NULL);
	harbol_veque_free(&p, NULL);
}

void test_harbol_lex(void)
{
	if( !g_harbol_debug_stream )
		return;
	
	fputs("\nlex tools :: test C hex.\n", g_harbol_debug_stream);
	const char *c_hexs[] = {
		"0X55", /// correct
		"0X5fl", /// correct
		"0x55LLU", /// correct
		"0X5full", /// correct
		"0x0.3p10", /// correct
		"0x1.2p3", /// correct
		"0x1p+1", /// correct
		"0x1.b7p-1", /// correct
		"0x3.3333333333334p-5", /// correct
		"0x", /// bad
		"0x.f", /// bad
		"0x1.f", /// bad
	};
	for( const char **i=&c_hexs[0]; i<1[&c_hexs]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		bool is_float = false;
		const bool res = lex_c_style_hex(*i, &end, &lexeme, &is_float);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s' | is float? %s\n", res ? "yes" : "no", lexeme.cstr, is_float ? "yes" : "no");
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test Go hex.\n", g_harbol_debug_stream);
	const char *go_hex[] = {
		"0xBadFace", /// correct
		"0xBad_Face", /// correct
		"0x_67_7a_2f_cc_40_c6", /// correct
		"0x1p-2",       /// == 0.25
		"0x2.p10",      /// == 2048.0
		"0x1.Fp+0",     /// == 1.9375
		"0X.8p-0",      /// == 0.5
		"0X_1FFFP-16",  /// == 0.1249847412109375
		"0x15e-2",      /// == 0x15e - 2 (integer subtraction)
		"0x3.3333333333334p-5", /// correct
		"0x.p1", /// bad
		"0x1.5e-2", /// bad
		"0x_67_7a_2f_cc_40_c6_", /// bad
		"0_xBadFace", /// bad
	};
	for( const char **i=&go_hex[0]; i<1[&go_hex]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		bool is_float = false;
		const bool res = lex_go_style_hex(*i, &end, &lexeme, &is_float);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s' | is float? %s\n", res ? "yes" : "no", lexeme.cstr, is_float ? "yes" : "no");
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test C decimal.\n", g_harbol_debug_stream);
	const char *c_dec[] = {
		"344ULL", /// correct
		"33", /// correct
		"33_45_34", /// correct
		".0", /// correct
		".344", /// correct
		".34e4", /// correct
		".34e4f", /// correct
		"0.f", /// correct
		"0.34e4f", /// correct
		"3.-3", /// good
		"3.", /// good
		"3.e-3", /// good
		"3.e--3", /// good
		"0f", /// bad
		".34ef", /// bad
		"0.34ef", /// bad
		"0.34e1ULL", /// bad
		"34e1ULL", /// bad
	};
	for( const char **i=&c_dec[0]; i<1[&c_dec]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		bool is_float = false;
		const bool res = lex_c_style_decimal(*i, &end, &lexeme, &is_float);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s' | is float? %s\n", res ? "yes" : "no", lexeme.cstr, is_float ? "yes" : "no");
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test Go decimal.\n", g_harbol_debug_stream);
	const char *go_dec[] = {
		"0.",
		"72.40",
		"072.40",
		"2.71828",
		"1.e+0",
		"6.67428e-11",
		"1E6",
		".25",
		".12345E+5",
		"1_5.",
		"0.15e+0_2",
		"1_.5", /// bad
		"1._5", /// bad
		"1.5_e1", /// bad
		"1.5e_1", /// bad
		"1.5e1_" /// bad
	};
	for( const char **i=&go_dec[0]; i<1[&go_dec]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		bool is_float = false;
		const bool res = lex_go_style_decimal(*i, &end, &lexeme, &is_float);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s' | is float? %s\n", res ? "yes" : "no", lexeme.cstr, is_float ? "yes" : "no");
		harbol_string_clear(&lexeme);
	}
	
	
	fputs("\nlex tools :: test Go String.\n", g_harbol_debug_stream);
	const char *go_strs[] = {
		"`raw\\nstring`",
		"\"foo\\x61 bar\"",
		"\"foo\\141bar\"",
		"\"foo\\u0710 bar\"",
		"\"日本語\"",
		"\"\\u65e5本\\U00008a9e\""
	};
	for( const char **i=&go_strs[0]; i<1[&go_strs]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_go_style_str(*i, &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test C octal.\n", g_harbol_debug_stream);
	const char *c_oct[] = {
		"0553", /// correct
		"0553ULL", /// correct
		"0553u", /// correct
		"0553llu", /// correct
		"0777", /// correct
		"0553LUL", /// bad
		"553", /// bad
		"078", /// bad
	};
	for( const char **i=&c_oct[0]; i<1[&c_oct]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_c_style_octal(*i, &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test Go octal.\n", g_harbol_debug_stream);
	const char *go_oct[] = {
		"0o553", /// correct
		"0o777", /// correct
		"0o78", /// bad
		"0o553ULL", /// bad
		"0o553LUL", /// bad
		"0o553ull", /// bad
		"553", /// bad
		"0553", /// bad
	};
	for( const char **i=&go_oct[0]; i<1[&go_oct]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_go_style_octal(*i, &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	
	fputs("\nlex tools :: test C binary.\n", g_harbol_debug_stream);
	const char *c_binary[] = {
		"0b11101", /// correct
		"0b101ULL", /// correct
		"0b1u", /// correct
		"0b101llu", /// correct
		"b1101010", /// bad
		"0b10002", /// bad
		"011101010", /// bad
	};
	for( const char **i=&c_binary[0]; i<1[&c_binary]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_c_style_binary(*i, &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test Go binary.\n", g_harbol_debug_stream);
	const char *go_binary[] = {
		"0b11101", /// correct
		"b1101010", /// bad
		"0b10002", /// bad
		"011101010", /// bad
	};
	for( const char **i=&go_binary[0]; i<1[&go_binary]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_go_style_binary(*i, &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test C various numbers.\n", g_harbol_debug_stream);
	const char *c_nums[] = {
		"0X55", /// correct
		"0X5fl", /// correct
		"0x55LLU", /// correct
		"0X5full", /// correct
		"0X_CAFE_BABE", /// correct
		"0x0.3p10", /// correct
		"0x1.2p3", /// correct
		"0x1p+1", /// correct
		"0x1.b7p-1", /// correct
		"0x0.3_p10", /// correct
		"0x3.3333333333334p-5", /// correct
		"0x1.f", /// bad
		"0553", /// correct
		"0553ULL", /// correct
		"0553u", /// correct
		"0553llu", /// correct
		"0777", /// correct
		"0553LUL", /// bad
		"553", /// bad
		"078", /// bad
		".llu", /// bad
	};
	for( const char **i=&c_nums[0]; i<1[&c_nums]; i++ ) {
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		bool is_float = false;
		const bool res = lex_c_style_number(*i, &end, &lexeme, &is_float);
		fprintf(g_harbol_debug_stream, "result: %s :: lexeme: '%s' | is float? %s\n", res ? "yes" : "no", lexeme.cstr, is_float ? "yes" : "no");
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test single-line comment lexing.\n", g_harbol_debug_stream);
	{
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_single_line_comment("// kektus \\      \n foobar  \\ \n bazbard", &end, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: comment: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
	
	fputs("\nlex tools :: test multi-line comment lexing.\n", g_harbol_debug_stream);
	{
		struct HarbolString lexeme = harbol_string_create(NULL);
		const char *end = NULL;
		const bool res = lex_multi_line_comment("/** kektus \n foobar  \n bazbard */", &end, "*/", sizeof "*/"-1, &lexeme);
		fprintf(g_harbol_debug_stream, "result: %s :: comment: '%s'\n", res ? "yes" : "no", lexeme.cstr);
		harbol_string_clear(&lexeme);
	}
}
