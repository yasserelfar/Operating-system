#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

struct MemBlock_LIST kernal_list;

	int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
	//panic("not implemented yet");
	start_heap = daStart;
	segment_break = daStart + initSizeToAllocate;

	if (segment_break > daLimit)
		return E_NO_MEM;

	hard_limit = daLimit;
	struct FrameInfo *ptr_frame_info ;
	int totalFrameSize = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
	int entries = totalFrameSize / PAGE_SIZE;
	uint32 va = daStart;

	for (int i = 0; i < entries; i++)
	{
		int ret1 = allocate_frame(&ptr_frame_info);
		if (ret1 == E_NO_MEM)
			return E_NO_MEM;
		int ret2 = map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
		ptr_frame_info->va=va;
		if (ret2 == E_NO_MEM)
			return E_NO_MEM;
		va += PAGE_SIZE;
	}

	initialize_dynamic_allocator(daStart, initSizeToAllocate);
	return 0;
}
void* sbrk(int increment)
{
		//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
		/* increment > 0: move the segment break of the kernel to increase the size of its heap,
		 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
		 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
		 * increment = 0: just return the current position of the segment break
		 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
		 * 				you should deallocate pages that no longer contain part of the heap as necessary.
		 * 				and returns the address of the new break (i.e. the end of the current heap space).
		 *
		 * NOTES:
		 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
		 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
		 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
		 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
		 */

		//MS2: COMMENT THIS LINE BEFORE START CODING====
		//return (void*)-1 ;
		//panic("not implemented yet");
		if (increment > 0)
		{
			uint32 oldBreak = segment_break;
			segment_break += increment;
			segment_break = ROUNDUP(segment_break, PAGE_SIZE);
			if (segment_break > hard_limit)
				panic("Exceeded dynamic allocator");
			int numberOfPages = (segment_break - oldBreak) / PAGE_SIZE;
			struct FrameInfo *ptr_frame_info;
			uint32 va = ROUNDUP(oldBreak, PAGE_SIZE);
			for (int i = 0; i < numberOfPages; i++)
			{
				allocate_frame(&ptr_frame_info);
				map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
				ptr_frame_info->va=va;

				va += PAGE_SIZE;
			}

			return (void*)oldBreak;
		}
		else if (increment == 0)
		{
			return (void*)segment_break;
		}
		else
		{
			uint32 oldBreak = segment_break;
			segment_break -= increment;
			int numberOfPages = (oldBreak - segment_break) / PAGE_SIZE;
			uint32 va = oldBreak - PAGE_SIZE;
			uint32 * ptr_page_table;


				for (int i = 0; i < numberOfPages; i++)
				{
					struct FrameInfo * frame = get_frame_info(ptr_page_directory, va,&ptr_page_table);
					free_frame(frame);
					va -= PAGE_SIZE;
				}

			return (void*)segment_break;
		}
		return NULL;
}
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	uint32 start = hard_limit + PAGE_SIZE;

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
	{
		//cprintf("kmalloc block\n");
		return alloc_block_FF(size);
	}
	if(isKHeapPlacementStrategyFIRSTFIT())
	{
		if(size > KERNEL_HEAP_MAX - start)
				return NULL;

		int numbereOfFreeFrames = 0;
		uint32 va = start;
		size = ROUNDUP(size,PAGE_SIZE);
		int numberOfPages = size / PAGE_SIZE;



		for(uint32 startingPage = start; startingPage < KERNEL_HEAP_MAX; startingPage += PAGE_SIZE)
		{
			va = startingPage;
			numbereOfFreeFrames = 0;
			for (int i = 0; i < numberOfPages; i++)
			{
				if (size > KERNEL_HEAP_MAX - va)
					return NULL;

				uint32* ptr_page_table = NULL;
				struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, va, &ptr_page_table);
				if(ptr_frame_info == NULL)
				{
					numbereOfFreeFrames++;
					va += PAGE_SIZE;
				}
				else
				{
					startingPage += numbereOfFreeFrames * (PAGE_SIZE); //check this again
					numbereOfFreeFrames = 0;
					break;
				}

				if(numbereOfFreeFrames == numberOfPages)
				{
					va = startingPage;
					for (int j = 0; j < numberOfPages; j++)
					{
						struct FrameInfo *ptr_frame_info = NULL;
						allocate_frame(&ptr_frame_info);
						map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
						ptr_frame_info->va=va;
						if(va==startingPage){
							ptr_frame_info->size=numberOfPages;
						}
						va += PAGE_SIZE;
					}

					return (void*)startingPage;
				}
			}
		}


	}
	return NULL;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	if(virtual_address<(void *)hard_limit&&virtual_address>(void *)start_heap)
		free_block(virtual_address);


	else if(virtual_address>(void *)(hard_limit+PAGE_SIZE) &&virtual_address<(void *)KERNEL_HEAP_MAX)
	{
		uint32 *new_page=NULL;

		int page=get_page_table(ptr_page_directory,(uint32 )virtual_address,&new_page);
		if(new_page==NULL)
			return;
		struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, (uint32 )virtual_address, &new_page);
		int num_frams=ptr_frame_info->size;
		for(int i=0;i<num_frams;i++)
		{

			unmap_frame(ptr_page_directory,(uint32)virtual_address);

			virtual_address+=PAGE_SIZE;
		}

	}


	else
	{
		panic("wrong virtual address\n");
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	struct FrameInfo *ptr_frame_info ;
	ptr_frame_info = to_frame_info(physical_address) ;
	if(ptr_frame_info!=NULL){
			if(ptr_frame_info->references==1){
				uint32 pageno=physical_address/PAGE_SIZE,offset=physical_address%PAGE_SIZE;
				return (ptr_frame_info->va)+offset;
			}
			else{
				return 0;
			}
	}
	else{
		return 0;
	}
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	//change this "return" according to your answer

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//change this "return" according to your answer
//	uint32  dir=PDX(virtual_address);
//	uint32  newpage=PTX(virtual_address),* base=NULL;
//	get_page_table((ptr_page_directory+dir),virtual_address,1,&base);
//
//	uint32  offest=(virtual_address,0,12);
//
//	uint32  * frame_number= base+newpage;
//
//	return to_physical_address(frame_number)+offest;
//	if(virtual_address>(hard_limit+PAGE_SIZE) && virtual_address<KERNEL_HEAP_MAX ){
		uint32 * ptr_pagetable=NULL;
		uint32 frame_num;
		uint32 offset = virtual_address%PAGE_SIZE;
		uint32 x=get_page_table(ptr_page_directory, virtual_address, &ptr_pagetable);

		if(x!=TABLE_NOT_EXIST){
			struct FrameInfo * ptr_frame_info=get_frame_info(ptr_page_directory, virtual_address, &ptr_pagetable);
			if(ptr_frame_info==NULL)
				return 0;
			frame_num= ptr_pagetable[PTX(virtual_address)];
			uint32	framee=frame_num & 0xfffff000;//start physical add
			uint32 pa=(framee+offset);
			return pa;
		}
		else{
			return 0;
		}
//		}
//		else if(virtual_address>KERNEL_HEAP_START&&virtual_address<hard_limit){
//			if (virtual_address == 0)
//			            return 0;
//
//			unsigned int physical_address;
//			physical_address = virtual_address-KERNEL_BASE;
//			return physical_address;
//		}
//		return 0;

}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void* krealloc(void* virtual_address, uint32 new_size)
{
    //TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
    // Write your code here, remove the panic and write your code
    if(new_size==0){
        kfree(virtual_address);
        return NULL;
    }
    if(virtual_address==NULL){
        kmalloc(new_size);
    }
    uint32 *new_page=NULL;
	int page=get_page_table(ptr_page_directory,(uint32 )virtual_address,&new_page);
	struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, (uint32 )virtual_address, &new_page);
	int num_frams=ptr_frame_info->size;

    kfree(virtual_address);
    if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE)
    {

    	return realloc_block_FF(virtual_address,new_size);
    }
    uint32 size=ROUNDUP(new_size,PAGE_SIZE);
    void * return_address=kmalloc(size);
    if(return_address!=NULL)
    	return return_address;
    else
    {
    	uint32 va=(uint32)virtual_address;
    	for (int j = 0; j < num_frams; j++)
		{
			struct FrameInfo *ptr_frame_info = NULL;
			allocate_frame(&ptr_frame_info);
			map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE);
			ptr_frame_info->va=va;
			if(j==0)
			ptr_frame_info->size=num_frams;
			va += PAGE_SIZE;
		}

    	return NULL;
    }
    return NULL;
    //panic("krealloc() is not implemented yet...!!");

}
