#include <inc/lib.h>
struct size_info{
	uint32 va;
	uint32 size;
	uint32 location;
}s[NUM_OF_UHEAP_PAGES];
int count=0;

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================



void* malloc(uint32 size)
{
//	cprintf("in malloc \n");
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
//	uint32 env_page_directory =sys_current_env();
//	cprintf("after sys_current_env  \n");
	uint32 start=sys_hard_limit()+PAGE_SIZE;
//	cprintf("after sys_hard_limit  \n");
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
//			cprintf("in alloc_block_FF \n");
			return alloc_block_FF(size);
		}
	if(sys_isUHeapPlacementStrategyFIRSTFIT())
	{
//		cprintf("in page malloc\n");

		if(size>USER_HEAP_MAX-start)
		{
			return NULL;
		}

		uint32 numbereOfFreeFrames = 0;
		uint32 startindex = 0;

		size = ROUNDUP(size,PAGE_SIZE);

		uint32 *start_paging = NULL;
		uint32 numberOfPages = size / PAGE_SIZE;

//		cprintf("req # of frames: %d \n", numberOfPages);

		for(
				uint32 temp_va = start;
				temp_va < USER_HEAP_MAX;
				temp_va += PAGE_SIZE
			)
		{

			//cprintf("this frame >= user heap : %d \n", temp_va >= USER_HEAP_START);
			if(sys_is_marked(temp_va) == 0)
			{
				//cprintf("this frame is not marked \n");

				if(numbereOfFreeFrames == 0)
				{
					start_paging = (void*)temp_va;
				}

				numbereOfFreeFrames++;
				//cprintf("this frame: %d \n", numbereOfFreeFrames);

				if(numbereOfFreeFrames == numberOfPages)
				{
					//cprintf("alloc needed frames \n");

					sys_allocate_user_mem((uint32) start_paging,size);
					s[count].va=(uint32)start_paging;
					s[count].size=size;

					count++;
					return (void*)start_paging;
				}

			}
			else
			{
				numbereOfFreeFrames=0;
				start_paging=NULL;
			}

		}

	}
		return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	uint32 start=sys_hard_limit()+PAGE_SIZE;

	if((uint32)virtual_address>=USER_HEAP_START&& (uint32)virtual_address<start-PAGE_SIZE)
	{
		free_block(virtual_address);
		//cprintf("\n in block \n");
		return;
	}

	else if((uint32)virtual_address>=start&& (uint32)virtual_address<USER_HEAP_MAX)
	{
		if(sys_is_marked((uint32)virtual_address)){
			uint32 num_frames=0;
			for(uint32 i=count;i>=0;i--)
			{
				cprintf("s\n");
				if((uint32)virtual_address==s[i].va)
					{
					cprintf("found");
					if(s[i].size!=0){
					num_frames=s[i].size/PAGE_SIZE;
					s[i].size=0;
					break;
					}
					}
			}

		///cprintf("\n num of frames%d\n",num_frames);
		if(num_frames==0)
		{
		//	cprintf("\n num of %d\n",num_frames);
			return;
		}
//		int32 temp_va=(uint32)virtual_address;
//
//		for(int i=0;i<num_frames;i++)
//		{
//			sys_set_perm(temp_va);
//			temp_va+=PAGE_SIZE;
//		}
		//cprintf("before free mem\n");
		sys_free_user_mem((uint32)virtual_address,num_frames*PAGE_SIZE);
		//cprintf("\nafter free mem\n");

		}
		else
			{
			//cprintf("\n in else\n");
			return;
			}
	}

	else{
		panic("wrong address\n");
	}


}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
