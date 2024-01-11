/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"
bool is_change=0;
struct BlockMetaData *last_bloc;
struct BlockMetaData *first_free;
uint32 last_size=0;

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
struct MemBlock_LIST my_list;
bool is_initialized;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{

	//=========================================
	//DON'T CHANGE THESE LINES=================
	if (initSizeOfAllocatedSpace == 0)
		return ;
	is_initialized=1;

	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
	//panic("initialize_dynamic_allocator is not implemented yet");

	LIST_INIT(&my_list);
	my_list.lh_first = (void*)daStart;
	my_list.lh_last = (void*)daStart;
	my_list.lh_first->is_free = 1;
	my_list.lh_first->size = initSizeOfAllocatedSpace;


}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
	//panic("alloc_block_FF is not implemented yet");

	if(size == 0){
		return NULL;
	}

	if (!is_initialized)
	{
	uint32 required_size = size + sizeOfMetaData();
	uint32 da_start = (uint32)sbrk(required_size);

	//get new break since it's page aligned! thus, the size can be more than the required one
	uint32 da_break = (uint32)sbrk(0);
	initialize_dynamic_allocator(da_start, da_break - da_start);
	}
	struct BlockMetaData *iterator=NULL;
	   bool found=0;
	   uint32 number=my_list.size;

		if(is_change==0&&size>=last_size&&(void*)last_bloc!=NULL)
		{
			iterator=last_bloc;
		}
		else
		{
			if(first_free==NULL)
				iterator=LIST_FIRST(&my_list);
			else
				iterator=first_free;
		}
		bool first_free_bool=0;
			while((void*)iterator!=NULL)
			{

				if(iterator->is_free == 1)
				{

				   if(iterator->size - sizeOfMetaData() == size||iterator->size-(size+sizeOfMetaData())<=sizeOfMetaData()){

					   iterator->is_free=0;
					   found=1;
					   last_bloc=iterator;
					   last_size=size;
					   is_change=0;
					   return (void *) iterator +sizeOfMetaData();


				   }
				   else if(iterator->size  > (size+sizeOfMetaData()))
					{

						struct BlockMetaData *newelement=(void *) iterator+size+sizeOfMetaData();
						uint32 x=iterator->size-(size+sizeOfMetaData());

						newelement->size=x;
						iterator->size=size+sizeOfMetaData();
						newelement ->is_free=1;

						iterator->is_free=0;
						if (x > sizeOfMetaData())
						{
							LIST_INSERT_AFTER(&my_list,iterator,newelement);
						}
						else
						{
							iterator->size += x;
						}


						found=1;
						last_bloc=iterator;
						last_size=size;
						is_change=0;
						return (void*)iterator +sizeOfMetaData();

					}
				}
				if(first_free_bool==0)
				{
					if(iterator->is_free == 1)
						{
						first_free_bool=1;
							first_free=iterator;
						}
				}
				iterator=iterator->prev_next_info.le_next;
			}



		if(!found){
//			iterator=sbrk(0);
			uint32 oldbreak = (uint32)sbrk(size+sizeOfMetaData());

			if(oldbreak==-1){
				return NULL;
			}
			else
			{
//				uint32 da_break = (uint32)sbrk(0);
//
//				my_list.lh_last->size += (da_break - copyCat);


				struct BlockMetaData *newelement =  (struct BlockMetaData *)oldbreak;
				newelement->is_free = 0;
				newelement->size = size+sizeOfMetaData();

				LIST_INSERT_TAIL(&my_list, newelement);

				struct BlockMetaData *newelement2 =  (struct BlockMetaData *)( (uint32)oldbreak + newelement->size );
				newelement2->is_free = 1;
				newelement2->size = PAGE_SIZE - newelement->size;

				LIST_INSERT_AFTER(&my_list, newelement, newelement2);
				   last_bloc=newelement;

				   last_size=size;
				   is_change=0;
				   if(first_free_bool==0)
				   {
					   first_free=newelement2;
					   first_free_bool=1;
				   }
				return (void*)(newelement + 1);


			}
		}


	return NULL;


}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//panic("alloc_block_BF is not implemented yet");
	struct BlockMetaData *iterator1 = LIST_FIRST(&my_list);
			struct BlockMetaData *iterator2 = NULL;



			if(size==0){
				return NULL;
			}
			LIST_FOREACH(iterator1,&my_list)
			{
				if(iterator1->is_free && iterator1->size == size + sizeOfMetaData()){

					iterator1->is_free=0;
					//iterator2=iterator1; not needed in this case

					return (void*) iterator1+sizeOfMetaData();
				}
				else if(iterator1->is_free && (iterator1->size) > size + sizeOfMetaData() )
				{

					if(iterator2==NULL){
						iterator2 = iterator1;
					}
					else
					{
						uint32 sizeold = iterator2->size - size + sizeOfMetaData();
						uint32 sizenew = iterator1->size - size + sizeOfMetaData();

						if(sizeold > sizenew)
						{
							iterator2 = iterator1;
						}
					}
				}

			}

			if(iterator2 == NULL){

				if(sbrk(size+sizeOfMetaData()) == (void*)-1){
					return NULL;
				}
				else{
					return sbrk(size+sizeOfMetaData()) + sizeOfMetaData();
				}
			}
			else{


				if(iterator2->size - (size + sizeOfMetaData()) <=  sizeOfMetaData()){

					iterator2->is_free = 0;
					return (void*)iterator2 + sizeOfMetaData();
				}


				struct BlockMetaData *newelement=(void*) ((unsigned char*)iterator2 + size + sizeOfMetaData());

				newelement->size = iterator2->size - (size + sizeOfMetaData());

				iterator2->size= size + sizeOfMetaData();
				newelement ->is_free=1;

			   // newelement->prev_next_info.le_prev=iterator;
			   // newelement->prev_next_info.le_next = iterator->prev_next_info.le_next;
			   // iterator->prev_next_info.le_next=newelement;
			   // heapsize-=newelement.size;


				iterator2->is_free = 0;

				//cprintf("im at else\n");

				LIST_INSERT_AFTER(&my_list,iterator2,newelement);

				//cprintf("im after insert\n");

				return (void *)iterator2+sizeOfMetaData();
			}

			return NULL;
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//panic("free_block is not implemented yet");

		if (va == NULL)
			return;
		struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);
		uint32 size = curBlkMetaData->size;
		curBlkMetaData->is_free = 1;
		struct BlockMetaData *prev = curBlkMetaData->prev_next_info.le_prev;
		struct BlockMetaData *next = curBlkMetaData->prev_next_info.le_next;
		if(next > 0 && prev > 0 && next < my_list.lh_last + 1 && prev > my_list.lh_first - 1 && next->is_free == 1 && prev->is_free == 1)
		{
			prev->size += size + next->size;
			prev->is_free = 1;

			next->is_free = 0;
			next->size = 0;

			curBlkMetaData->is_free = 0;
			curBlkMetaData->size = 0;
			LIST_REMOVE(&my_list,next);
			LIST_REMOVE(&my_list,curBlkMetaData);
			is_change=1;
			if(first_free>prev)
				first_free=prev;
			return;
		}
		else if(next > 0 && next->is_free == 1){
			curBlkMetaData->size += next->size;
			curBlkMetaData->is_free = 1;
			next->is_free = 0;
			next->size = 0;

			LIST_REMOVE(&my_list,next);
			if(first_free>curBlkMetaData)
					first_free=curBlkMetaData;
			is_change=1;
			return;

		}
		else if(prev > 0 && prev->is_free == 1){
			prev->size += size;
			prev->is_free = 1;
			curBlkMetaData->is_free = 0;
			curBlkMetaData->size = 0;

			LIST_REMOVE(&my_list,curBlkMetaData);
			if(first_free>prev)
				first_free=prev;
			is_change=1;

			return;
		}
		if(first_free>curBlkMetaData)
			first_free=curBlkMetaData;
}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	//panic("realloc_block_FF is not implemented yet");
		if (va == NULL)
		{
				return alloc_block_FF(new_size);
		}
		if (new_size == 0)
		{
			free_block(va);
			return NULL;
		}
		struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1);
		uint32 size = curBlkMetaData->size;
		struct BlockMetaData *next = LIST_NEXT(curBlkMetaData);


		//cprintf("%d------------%d\n",next->size,new_size - curBlkMetaData->size);
		//cprintf("-----%d------\n",is_free_block(next + 1));
		//print_blocks_list(my_list);
		//cprintf("current size:%d,newSize:%d, nextBlockSize:%d\n",curBlkMetaData->size,new_size,next->size);
		//if (next > 0 && next->is_free == 1 && next->size == new_size - curBlkMetaData->size - sizeOfMetaData())
			if (curBlkMetaData->size - sizeOfMetaData() == new_size){
				return va;
			}

			else if (next > 0 && next->is_free == 1 && next->size > new_size - (curBlkMetaData->size - sizeOfMetaData()))
			{

			struct BlockMetaData *new = (void*)curBlkMetaData + new_size + sizeOfMetaData();
			//cprintf(">>>>>>>>>>>>>>>>>>>");
			//cprintf("%d\n",sizeOfMetaData());
			//cprintf("current size:%d,newSize:%d, nextBlockSize:%d",curBlkMetaData->size,new_size,next->size);
			curBlkMetaData->is_free = 1;
			int nextSize = next->size - (new_size - curBlkMetaData->size) - sizeOfMetaData();
			free_block(next + 1);
			curBlkMetaData->is_free = 0;
			curBlkMetaData->size = new_size + sizeOfMetaData();
			//cprintf("-------%d------",is_free_block(next + 1));
			//LIST_REMOVE(&my_list,next);
			//cprintf("aaaaaaaaaaaaaaaaa");
			//curBlkMetaData->size = new_size + sizeOfMetaData();

			new->size = nextSize;
			new->is_free = 1;
			LIST_INSERT_AFTER(&my_list,curBlkMetaData,new);
			//cprintf("current size:%d,newSize:%d, nextBlockSize:%d",curBlkMetaData->size,new_size,new->size);
			//uint32 nextBlockAddress = (uint32)va + curBlkMetaData->size - sizeOfMetaData();
			//struct BlockMetaData* nextBlock = (struct BlockMetaData*)nextBlockAddress;
			//nextBlock->is_free = 0;
			//nextBlock->size = 0;

			return va;


		//	struct BlockMetaData *new = &x;
		//	new->is_free = 1;
		//	new->size = nextSize + sizeOfMetaData();
		//	LIST_INSERT_AFTER(&my_list,curBlkMetaData,new);


			}
			else if (next > 0 && next->is_free == 1 && next->size == new_size - (curBlkMetaData->size - sizeOfMetaData()))
			{
				free_block(next + 1);
				curBlkMetaData->size = new_size + sizeOfMetaData();
				LIST_REMOVE(&my_list,next);
				return va;

			}
			else if (new_size < curBlkMetaData->size - sizeOfMetaData())
			{
				int nextSize = curBlkMetaData->size - new_size;
				curBlkMetaData->size = new_size + sizeOfMetaData();
				struct BlockMetaData *new = (void*)curBlkMetaData + new_size + sizeOfMetaData();
				//cprintf("+++++++%d++++++\n",nextSize);
				new->size = nextSize - sizeOfMetaData();
				new->is_free = 1;
				LIST_INSERT_AFTER(&my_list,curBlkMetaData,new);
				if (new->prev_next_info.le_next->is_free == 1)
				{
					free_block(new);
				}

				return va;
			}

		else
		{
			free_block(va);
			return alloc_block_FF(new_size);
		}

	return NULL;
}
