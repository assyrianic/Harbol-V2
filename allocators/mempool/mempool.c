#include "mempool.h"

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif

static NO_NULL void __ll_remove(struct HarbolMemPool *const mempool, struct HarbolMemNode **const node)
{
	if( (*node)->Next != NULL )
		(*node)->Next->Prev = (*node)->Prev;
	else {
		mempool->FreeList.Tail = (*node)->Prev;
		if( mempool->FreeList.Tail != NULL )
			mempool->FreeList.Tail->Next = NULL;
	}
	
	if( (*node)->Prev != NULL )
		(*node)->Prev->Next = (*node)->Next;
	else {
		mempool->FreeList.Head = (*node)->Next;
		if( mempool->FreeList.Head != NULL )
			mempool->FreeList.Head->Prev = NULL;
	}
}

HARBOL_EXPORT struct HarbolMemPool harbol_mempool_create(const size_t size)
{
	struct HarbolMemPool mempool = EMPTY_HARBOL_MEMPOOL;
	if( size==0 )
		return mempool;
	else {
		mempool.Stack.Size = size;
		mempool.Stack.Mem = malloc(mempool.Stack.Size * sizeof *mempool.Stack.Mem);
		if( mempool.Stack.Mem==NULL ) {
			mempool.Stack.Size = 0;
			return mempool;
		} else {
			mempool.Stack.Base = mempool.Stack.Mem + mempool.Stack.Size;
			return mempool;
		}
	}
}

HARBOL_EXPORT struct HarbolMemPool harbol_mempool_from_buffer(void *const buf, const size_t size)
{
	struct HarbolMemPool mempool = EMPTY_HARBOL_MEMPOOL;
	if( size==0 || size<=sizeof(struct HarbolMemNode) )
		return mempool;
	else {
		mempool.Stack.Size = size;
		mempool.Stack.Mem = buf;
		mempool.Stack.Base = mempool.Stack.Mem + mempool.Stack.Size;
		return mempool;
	}
}

HARBOL_EXPORT bool harbol_mempool_clear(struct HarbolMemPool *const mempool)
{
	if( mempool->Stack.Mem==NULL )
		return false;
	else {
		free(mempool->Stack.Mem);
		*mempool = (struct HarbolMemPool)EMPTY_HARBOL_MEMPOOL;
		return true;
	}
}

static NO_NULL struct HarbolMemNode *__iterate_freenodes(struct HarbolMemPool *const mempool, const size_t bytes)
{
	const uindex_t b = (bytes >> HARBOL_BUCKET_BITS) - 1;
	// check if we have a good sized node from the buckets.
	if( b < HARBOL_BUCKET_SIZE && mempool->Buckets[b] != NULL && mempool->Buckets[b]->Size >= bytes ) {
		struct HarbolMemNode *new_mem = mempool->Buckets[b];
		mempool->Buckets[b] = mempool->Buckets[b]->Next;
		if( mempool->Buckets[b] != NULL )
			mempool->Buckets[b]->Prev = NULL;
		return new_mem;
	} else if( mempool->FreeList.Head != NULL ) {
		const size_t mem_split_threshold = 24;
		// if the freelist is valid, let's allocate FROM the freelist then!
		for( struct HarbolMemNode **inode = &mempool->FreeList.Head; *inode != NULL; inode = &(*inode)->Next ) {
			if( (*inode)->Size < bytes )
				continue;
			else if( (*inode)->Size <= bytes + mem_split_threshold ) {
				// close in size - reduce fragmentation by not splitting.
				struct HarbolMemNode *new_mem = *inode;
				__ll_remove(mempool, inode);
				mempool->FreeList.Len--;
				return new_mem;
			} else {
				// split the memory chunk.
				struct HarbolMemNode *new_mem = (struct HarbolMemNode *)( (uint8_t *)*inode + ((*inode)->Size - bytes) );
				(*inode)->Size -= bytes;
				new_mem->Size = bytes;
				return new_mem;
			}
		}
		return NULL;
	}
	else return NULL;
}

HARBOL_EXPORT void *harbol_mempool_alloc(struct HarbolMemPool *const mempool, const size_t size)
{
	if( size==0 || size > mempool->Stack.Size )
		return NULL;
	else {
		// visual of the allocation block.
		// --------------
		// |  mem size  | lowest addr of block
		// |  next node | 12 bytes - 32 bit
		// |  prev node | 24 bytes - 64 bit
		// --------------
		// |   alloc'd  |
		// |   memory   |
		// |   space    | highest addr of block
		// --------------
		const size_t alloc_bytes = harbol_align_size(size + sizeof(struct HarbolMemNode), sizeof(intptr_t));
		struct HarbolMemNode *new_mem = __iterate_freenodes(mempool, alloc_bytes);
		if( new_mem==NULL ) {
			// not enough memory to support the size!
			if( mempool->Stack.Base - alloc_bytes < mempool->Stack.Mem )
				return NULL;
			else {
				// couldn't allocate from a freelist, allocate from available mempool.
				// subtract allocation size from the mempool.
				mempool->Stack.Base -= alloc_bytes;
				
				// use the available mempool space as the new node.
				new_mem = (struct HarbolMemNode *)mempool->Stack.Base;
				new_mem->Size = alloc_bytes;
			}
		}
		new_mem->Next = new_mem->Prev = NULL;
		uint8_t *const final_mem = (uint8_t *)new_mem + sizeof *new_mem;
		memset(final_mem, 0, new_mem->Size - sizeof *new_mem);
		return final_mem;
	}
}

HARBOL_EXPORT void *harbol_mempool_realloc(struct HarbolMemPool *const restrict mempool, void *ptr, const size_t size)
{
	if( size > mempool->Stack.Size )
		return NULL;
	// NULL ptr should make this work like regular alloc.
	else if( ptr==NULL )
		return harbol_mempool_alloc(mempool, size);
	else if( (uintptr_t)ptr - sizeof(struct HarbolMemNode) < (uintptr_t)mempool->Stack.Mem )
		return NULL;
	else {
		struct HarbolMemNode *node = (struct HarbolMemNode *)((uint8_t *)ptr - sizeof *node);
		uint8_t *resized_block = harbol_mempool_alloc(mempool, size);
		if( resized_block==NULL )
			return NULL;
		else {
			struct HarbolMemNode *resized = (struct HarbolMemNode *)(resized_block - sizeof *resized);
			memmove(resized_block, ptr, (node->Size > resized->Size)? (resized->Size - sizeof *node) : (node->Size - sizeof *node));
			harbol_mempool_free(mempool, ptr);
			return resized_block;
		}
	}
}

HARBOL_EXPORT bool harbol_mempool_free(struct HarbolMemPool *const restrict mempool, void *const ptr)
{
	if( ptr==NULL || (uintptr_t)ptr - sizeof(struct HarbolMemNode) < (uintptr_t)mempool->Stack.Mem )
		return false;
	else {
		// behind the actual pointer data is the allocation info.
		struct HarbolMemNode *mem_node = (struct HarbolMemNode *)((uint8_t *)ptr - sizeof *mem_node);
		const uindex_t b = (mem_node->Size >> HARBOL_BUCKET_BITS) - 1;
		
		// make sure the pointer data is valid.
		if( (uintptr_t)mem_node < (uintptr_t)mempool->Stack.Base || ((uintptr_t)mem_node - (uintptr_t)mempool->Stack.Mem) > mempool->Stack.Size || mem_node->Size==0 || mem_node->Size > mempool->Stack.Size )
			return false;
		// if the mem_node is right at the Stack Base ptr, then add it to the Stack.
		else if( (uintptr_t)mem_node==(uintptr_t)mempool->Stack.Base ) {
			mempool->Stack.Base += mem_node->Size;
		}
		// try to place it into bucket.
		else if( b < HARBOL_BUCKET_SIZE ) {
			if( mempool->Buckets[b]==NULL ) {
				mempool->Buckets[b] = mem_node;
			} else {
				for( struct HarbolMemNode *n = mempool->Buckets[b]; n != NULL; n = n->Next )
					if( n==mem_node )
						return false;
				mempool->Buckets[b]->Prev = mem_node;
				mem_node->Next = mempool->Buckets[b];
				mempool->Buckets[b] = mem_node;
			}
		}
		// otherwise, we add it to the free list.
		// We also check if the freelist already has the pointer so we can prevent double frees.
		else if( mempool->FreeList.Len==0 || ((uintptr_t)mempool->FreeList.Head >= (uintptr_t)mempool->Stack.Mem && (uintptr_t)mempool->FreeList.Head - (uintptr_t)mempool->Stack.Mem < mempool->Stack.Size) ) {
			for( struct HarbolMemNode *n = mempool->FreeList.Head; n != NULL; n = n->Next )
				if( n==mem_node )
					return false;
			
			// this code insertion sorts where largest Size is first.
			if( mempool->FreeList.Head==NULL ) {
				mempool->FreeList.Head = mempool->FreeList.Tail = mem_node;
				mempool->FreeList.Len++;
			} else if( mempool->FreeList.Head->Size > mem_node->Size ) {
				mem_node->Next = mempool->FreeList.Head;
				mem_node->Next->Prev = mem_node;
				mempool->FreeList.Head = mem_node;
				mempool->FreeList.Len++;
			} else {
				mem_node->Prev = mempool->FreeList.Tail;
				mempool->FreeList.Tail->Next = mem_node;
				mempool->FreeList.Tail = mem_node;
				mempool->FreeList.Len++;
			}
			
			if( mempool->FreeList.AutoDefrag && mempool->FreeList.MaxNodes != 0 && mempool->FreeList.Len > mempool->FreeList.MaxNodes )
				harbol_mempool_defrag(mempool);
		}
		return true;
	}
}

HARBOL_EXPORT bool harbol_mempool_cleanup(struct HarbolMemPool *const restrict mempool, void **ptrref)
{
	if( *ptrref==NULL )
		return false;
	else {
		const bool free_result = harbol_mempool_free(mempool, *ptrref);
		*ptrref = NULL;
		return free_result;
	}
}

HARBOL_EXPORT size_t harbol_mempool_mem_remaining(const struct HarbolMemPool *mempool)
{
	size_t total_remaining = (uintptr_t)mempool->Stack.Base - (uintptr_t)mempool->Stack.Mem;
	for( struct HarbolMemNode *n = mempool->FreeList.Head; n != NULL; n = n->Next )
		total_remaining += n->Size;
	for( uindex_t i=0; i<HARBOL_BUCKET_SIZE; i++ )
		for( struct HarbolMemNode *n=mempool->Buckets[i]; n != NULL; n = n->Next )
			total_remaining += n->Size;
	return total_remaining;
}


HARBOL_EXPORT bool harbol_mempool_defrag(struct HarbolMemPool *const mempool)
{
	// if the memory pool has been entirely released, fully defrag it.
	if( mempool->Stack.Size == harbol_mempool_mem_remaining(mempool) ) {
		mempool->FreeList.Head = mempool->FreeList.Tail = NULL;
		mempool->FreeList.Len = 0;
		for( uindex_t i=0; i<HARBOL_BUCKET_SIZE; i++ )
			mempool->Buckets[i] = NULL;
		mempool->Stack.Base = mempool->Stack.Mem + mempool->Stack.Size;
		return true;
	} else {
		for( uindex_t i=0; i<HARBOL_BUCKET_SIZE; i++ ) {
			while( mempool->Buckets[i] != NULL ) {
				if( (uintptr_t)mempool->Buckets[i] == (uintptr_t)mempool->Stack.Base ) {
					mempool->Stack.Base += mempool->Buckets[i]->Size;
					mempool->Buckets[i]->Size = 0;
					mempool->Buckets[i] = mempool->Buckets[i]->Next;
					if( mempool->Buckets[i] != NULL )
						mempool->Buckets[i]->Prev = NULL;
				}
				else break;
			}
		}
		
		const size_t predefrag_len = mempool->FreeList.Len;
		struct HarbolMemNode **node = &mempool->FreeList.Head;
		while( *node != NULL ) {
			if( (uintptr_t)*node == (uintptr_t)mempool->Stack.Base ) {
				// if node is right at the Stack, merge it back into the Stack.
				mempool->Stack.Base += (*node)->Size;
				(*node)->Size = 0;
				__ll_remove(mempool, node);
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (uintptr_t)*node + (*node)->Size == (uintptr_t)(*node)->Next ) {
				// Next node is at a higher address.
				(*node)->Size += (*node)->Next->Size;
				(*node)->Next->Size = 0;
				
				// <-[P Curr N]-> <-[P Next N]-> <-[P NextNext N]->
				// 
				//           |--------------------|
				// <-[P Curr N]-> <-[P Next N]-> [P NextNext N]->
				if( (*node)->Next->Next != NULL )
					(*node)->Next->Next->Prev = *node;
				
				// <-[P Curr N]-> <-[P NextNext N]->
				(*node)->Next = (*node)->Next->Next;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (uintptr_t)*node + (*node)->Size == (uintptr_t)(*node)->Prev && (*node)->Prev->Prev != NULL ) {
				// Prev node is at a higher address.
				(*node)->Size += (*node)->Prev->Size;
				(*node)->Prev->Size = 0;
				
				// <-[P PrevPrev N]-> <-[P Prev N]-> <-[P Curr N]->
				// 
				//               |--------------------|
				// <-[P PrevPrev N] <-[P Prev N]-> <-[P Curr N]->
				(*node)->Prev->Prev->Next = *node;
				
				// <-[P PrevPrev N]-> <-[P Curr N]->
				(*node)->Prev = (*node)->Prev->Prev;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (*node)->Prev != NULL && (*node)->Next != NULL && (uintptr_t)*node - (*node)->Next->Size == (uintptr_t)(*node)->Next ) {
				// Next node is at a lower address.
				// <-[P Prev N]-> <-[P Curr N]-> <-[P Next N]->
				(*node)->Next->Size += (*node)->Size;
				
				(*node)->Size = 0;
				
				//           |--------------------|
				// <-[P Prev N]-> <-[P Curr N]-> [P Next N]->
				(*node)->Next->Prev = (*node)->Prev;
				
				// <-[P Prev N]-> <-[P Next N]->
				(*node)->Prev->Next = (*node)->Next;
				
				// <-[P Prev N]-> <-[P Curr N]->
				*node = (*node)->Next;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (*node)->Prev != NULL && (*node)->Next != NULL && (uintptr_t)*node - (*node)->Prev->Size == (uintptr_t)(*node)->Prev ) {
				// Prev node is at a lower address.
				// <-[P Prev N]-> <-[P Curr N]-> <-[P Next N]->
				(*node)->Prev->Size += (*node)->Size;
				
				(*node)->Size = 0;
				
				//           |--------------------|
				// <-[P Prev N]-> <-[P Curr N]-> [P Next N]->
				(*node)->Next->Prev = (*node)->Prev;
				
				// <-[P Prev N]-> <-[P Next N]->
				(*node)->Prev->Next = (*node)->Next;
				
				// <-[P Curr N]-> <-[P Next N]->
				*node = (*node)->Prev;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			}
			else node = &(*node)->Next;
		}
		return predefrag_len > mempool->FreeList.Len;
	}
}

HARBOL_EXPORT NO_NULL void harbol_mempool_set_max_nodes(struct HarbolMemPool *const mempool, const size_t nodes)
{
	mempool->FreeList.MaxNodes = nodes;
}

HARBOL_EXPORT void harbol_mempool_toggle_auto_defrag(struct HarbolMemPool *const mempool)
{
	mempool->FreeList.AutoDefrag ^= true;
}
