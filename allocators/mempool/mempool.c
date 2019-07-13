#include "mempool.h"
#include <assert.h>

#ifdef OS_WINDOWS
#	define HARBOL_LIB
#endif


static inline NO_NULL void __remove_node(struct HarbolMemNode **const node)
{
	((*node)->Prev != NULL)? ((*node)->Prev->Next = (*node)->Next) : (*node = (*node)->Next);
	((*node)->Next != NULL)? ((*node)->Next->Prev = (*node)->Prev) : (*node = (*node)->Prev);
}


HARBOL_EXPORT struct HarbolMemPool harbol_mempool_create(const size_t size)
{
	struct HarbolMemPool mempool = { {NULL,NULL,0,0,false}, {NULL,NULL,0} };
	if( size==0 )
		return mempool;
	else {
		// align the mempool size to at least the size of an alloc node.
		mempool.Stack.Size = size;
		mempool.Stack.Mem = malloc(1 + mempool.Stack.Size * sizeof *mempool.Stack.Mem);
		if( mempool.Stack.Mem==NULL ) {
			mempool.Stack.Size = 0UL;
			return mempool;
		} else {
			mempool.Stack.Base = mempool.Stack.Mem + mempool.Stack.Size;
			return mempool;
		}
	}
}

HARBOL_EXPORT struct HarbolMemPool harbol_mempool_from_buffer(void *const buf, const size_t size)
{
	struct HarbolMemPool mempool = {0};
	if( size==0UL || size<=sizeof(struct HarbolMemNode) )
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
		*mempool = (struct HarbolMemPool){ {NULL,NULL,0,0,false}, {NULL,NULL,0} };
		return true;
	}
}

static NO_NULL struct HarbolMemNode *__iterate_freelist(struct HarbolMemPool *const mempool, const size_t bytes)
{
	if( mempool->FreeList.Head != NULL ) {
		const size_t mem_split_threshold = sizeof(intptr_t);
		// if the freelist is valid, let's allocate FROM the freelist then!
		for( struct HarbolMemNode **inode = &mempool->FreeList.Head; *inode != NULL; inode = &(*inode)->Next ) {
			if( (*inode)->Size < bytes )
				continue;
			else if( (*inode)->Size <= bytes + mem_split_threshold ) {
				// close in size - reduce fragmentation by not splitting.
				struct HarbolMemNode *new_mem = *inode;
				__remove_node(inode);
				mempool->FreeList.Len--;
				new_mem->Next = new_mem->Prev = NULL;
				return new_mem;
			} else {
				// split the memory chunk.
				struct HarbolMemNode *new_mem = (struct HarbolMemNode *)( (uint8_t *)*inode + ((*inode)->Size - bytes));
				(*inode)->Size -= bytes;
				new_mem->Size = bytes;
				new_mem->Next = new_mem->Prev = NULL;
				return new_mem;
			}
		}
		return NULL;
	}
	else return NULL;
}

HARBOL_EXPORT void *harbol_mempool_alloc(struct HarbolMemPool *const mempool, const size_t size)
{
	if( size==0UL || size > mempool->Stack.Size )
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
		struct HarbolMemNode *new_mem = __iterate_freelist(mempool, alloc_bytes);
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
				new_mem->Next = new_mem->Prev = NULL;
			}
		}
		
		uint8_t *const final_mem = (uint8_t *)new_mem + sizeof *new_mem;
		assert( "final_mem is MISALIGNED" && is_aligned(final_mem, sizeof(uintptr_t)) );
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
	else if( (uintptr_t)ptr <= (uintptr_t)mempool->Stack.Mem )
		return NULL;
	else {
		struct HarbolMemNode *node = (struct HarbolMemNode *)((uint8_t *)ptr - sizeof *node);
		//const bool increasing = node->Size < size;
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
	if( ptr==NULL || (uintptr_t)ptr <= (uintptr_t)mempool->Stack.Mem )
		return false;
	else {
		// behind the actual pointer data is the allocation info.
		struct HarbolMemNode *mem_node = (struct HarbolMemNode *)((uint8_t *)ptr - sizeof *mem_node);
		// make sure the pointer data is valid.
		if( (uintptr_t)mem_node < (uintptr_t)mempool->Stack.Base || ((uintptr_t)mem_node - (uintptr_t)mempool->Stack.Mem) > mempool->Stack.Size || mem_node->Size==0UL || mem_node->Size > mempool->Stack.Size )
			return false;
		// if the mem_node is right at the Stack Base ptr, then add it to the Stack.
		else if( (uintptr_t)mem_node==(uintptr_t)mempool->Stack.Base ) {
			mempool->Stack.Base += mem_node->Size;
		}
		// otherwise, we add it to the free list.
		// We also check if the freelist already has the pointer so we can prevent double frees.
		else if( mempool->FreeList.Len==0UL || ((uintptr_t)mempool->FreeList.Head >= (uintptr_t)mempool->Stack.Mem && (uintptr_t)mempool->FreeList.Head - (uintptr_t)mempool->Stack.Mem < mempool->Stack.Size) ) {
			for( struct HarbolMemNode *n = mempool->FreeList.Head; n != NULL; n = n->Next )
				if( n==mem_node )
					return false;
			
			// this code insertion sorts where largest Size is first.
			if( mempool->FreeList.Head==NULL ) {
				mempool->FreeList.Head = mempool->FreeList.Tail = mem_node;
				mempool->FreeList.Len++;
			} else if( mempool->FreeList.Head->Size <= mem_node->Size ) {
				mem_node->Next = mempool->FreeList.Head;
				mem_node->Next->Prev = mem_node;
				mempool->FreeList.Head = mem_node;
				mempool->FreeList.Len++;
			} else if( mempool->FreeList.Tail->Size > mem_node->Size ) {
				mem_node->Prev = mempool->FreeList.Tail;
				mempool->FreeList.Tail->Next = mem_node;
				mempool->FreeList.Tail = mem_node;
				mempool->FreeList.Len++;
			} else {
				struct HarbolMemNode *n = mempool->FreeList.Head;
				while( n->Next != NULL && n->Next->Size > mem_node->Size )
					n = n->Next;
				
				mem_node->Next = n->Next;
				if( n->Next != NULL )
					mem_node->Next->Prev = mem_node;
				
				n->Next = mem_node;
				mem_node->Prev = n;
				mempool->FreeList.Len++;
			}
			
			if( mempool->FreeList.AutoDefrag && mempool->FreeList.MaxNodes != 0UL && mempool->FreeList.Len > mempool->FreeList.MaxNodes )
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
	return total_remaining;
}


HARBOL_EXPORT bool harbol_mempool_defrag(struct HarbolMemPool *const mempool)
{
	// if the memory pool has been entirely released, fully defrag it.
	if( mempool->Stack.Size == harbol_mempool_mem_remaining(mempool) ) {
		memset(&mempool->FreeList, 0, sizeof mempool->FreeList);
		mempool->Stack.Base = mempool->Stack.Mem + mempool->Stack.Size;
		return true;
	} else {
		const size_t predefrag_len = mempool->FreeList.Len;
		struct HarbolMemNode **node = &mempool->FreeList.Head;
		while( *node != NULL ) {
			if( (uintptr_t)*node == (uintptr_t)mempool->Stack.Base ) {
				// if node is right at the Stack, merge it back into the Stack.
				mempool->Stack.Base += (*node)->Size;
				(*node)->Size = 0UL;
				__remove_node(node);
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (uintptr_t)*node + (*node)->Size == (uintptr_t)(*node)->Next ) {
				// Next node is at a higher address.
				(*node)->Size += (*node)->Next->Size;
				(*node)->Next->Size = 0UL;
				
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
				(*node)->Prev->Size = 0UL;
				
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
				(*node)->Next->Size += (*node)->Size;
				
				(*node)->Size = 0UL;
				(*node)->Next->Prev = (*node)->Prev;
				(*node)->Prev->Next = (*node)->Next;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else if( (*node)->Prev != NULL && (*node)->Next != NULL && (uintptr_t)*node - (*node)->Prev->Size == (uintptr_t)(*node)->Prev ) {
				// Prev node is at a lower address.
				(*node)->Prev->Size += (*node)->Size;
				
				(*node)->Size = 0UL;
				(*node)->Next->Prev = (*node)->Prev;
				(*node)->Prev->Next = (*node)->Next;
				
				mempool->FreeList.Len--;
				node = &mempool->FreeList.Head;
			} else {
				node = &(*node)->Next;
			}
		}
		return predefrag_len > mempool->FreeList.Len;
	}
}


HARBOL_EXPORT void harbol_mempool_toggle_auto_defrag(struct HarbolMemPool *const mempool)
{
	mempool->FreeList.AutoDefrag ^= true;
}
