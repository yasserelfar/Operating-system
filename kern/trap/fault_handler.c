
/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
//	cprintf("address is %u\n",fault_va);
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif
		fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);
	if(isPageReplacmentAlgorithmFIFO())
	{
		if(wsSize < (curenv->page_WS_max_size))
			{
			//placment
			fifo_placment(curenv,fault_va);
	}
		else
		{
			//replacment
			fifo_replacment(curenv,fault_va);
		}
	}

		if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
		{


	unsigned int sizes=curenv->ActiveList.size+curenv->SecondList.size;

		if(curenv->page_WS_max_size > sizes)
		{
			lru_placment(curenv,fault_va);
		}

	//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement

		else
		{



					lru_replacment(curenv,fault_va);
			}

		}

}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}
//void lru_placment(struct Env * curenv, uint32 fault_va,uint8 found,struct WorkingSetElement *delelem)
//{
//	cprintf(" LRU place:!\n");
//	env_page_ws_print(curenv);
//
//
//	struct WorkingSetElement *p=NULL;
//
//
//	if(curenv->ActiveList.size >= curenv->ActiveListSize)
//	{
//		  p+=(sizeof(struct WS_List )*curenv->ActiveList.size);
//		LIST_FOREACH (p, &curenv->ActiveList)
//		{
//			if(p->prev_next_info.le_next==NULL)
//			{
//				 LIST_REMOVE(&curenv->ActiveList, p);
//				 LIST_INSERT_HEAD(&curenv->SecondList, p);
//				 pt_set_page_permissions(curenv->env_page_directory,p->virtual_address,0,PERM_PRESENT);
//			}
//		}
//	}
//
//
//
//		if(found)
//		{
//			uint32 *y=NULL;
//			uint32 page=get_page_table(curenv->env_page_directory,delelem->virtual_address,&y);
//			 pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT|PERM_WRITEABLE | PERM_USER,0);
//			 LIST_INSERT_HEAD(&curenv->ActiveList, delelem);
//
//		}
//
//		else if(!found)
//		{
//			uint32 *y=NULL;
//			uint32 page=get_page_table(curenv->env_page_directory,fault_va,&y);
//			struct FrameInfo *  f=get_frame_info(curenv->env_page_directory,fault_va,&y);
//
//			struct FrameInfo *ptr_frame;
//		allocate_frame(&ptr_frame);
//			map_frame(curenv->env_page_directory, ptr_frame, fault_va, PERM_PRESENT|PERM_WRITEABLE | PERM_USER);
//			uint32 check= pf_read_env_page(curenv, (void *)fault_va);//read from disk
//			if(check!=  E_PAGE_NOT_EXIST_IN_PF)
//			{
//				struct FrameInfo *ptr_frame_info;
//
//				 struct WorkingSetElement *newelem=env_page_ws_list_create_element(curenv, fault_va);
//				 pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT|PERM_WRITEABLE | PERM_USER,0);
//				 LIST_INSERT_HEAD(&curenv->ActiveList, newelem);
//			}
//
//			else
//			{
//				if(
//					(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX)||
//					(fault_va<USTACKTOP&&fault_va>=USTACKBOTTOM));
//
//			else
//			{
//				unmap_frame(curenv->env_page_directory, fault_va);
//				pt_set_page_permissions(curenv->env_page_directory,fault_va,0,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);
//				sched_kill_env(curenv->env_id);
//				return;
//			}
//				if (f == NULL)
//				{
//					struct FrameInfo *ptr_frame_info;
//					allocate_frame(&ptr_frame_info);
//
//					map_frame(curenv->env_page_directory, ptr_frame_info, fault_va, PERM_PRESENT|PERM_WRITEABLE | PERM_USER);
//				}
//				struct WorkingSetElement *elem=env_page_ws_list_create_element(curenv, fault_va);
//				 LIST_INSERT_HEAD(&curenv->ActiveList, elem);
//				 pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT|PERM_WRITEABLE | PERM_USER,0);
//				 env_page_ws_print(curenv);
//			}
//
//					}
//	//					cprintf("lru placement  DONE!\n");
//}
void fifo_placment(struct Env * curenv, uint32 fault_va)
{
	//cprintf("in placement\n");
	//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
	//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
	// Writ e your code here, remove the panic and write your code
//	panic("page_fault_handler().PLACEMENT is not implemented yet...!!");
	struct FrameInfo *ptr_frame_info;
	allocate_frame(&ptr_frame_info);

	map_frame(curenv->env_page_directory, ptr_frame_info, fault_va, PERM_PRESENT|PERM_WRITEABLE | PERM_USER);
	int read = pf_read_env_page(curenv, (void*)fault_va);




	if (read == E_PAGE_NOT_EXIST_IN_PF)
	{
		if ((fault_va >= USER_HEAP_START && fault_va < USER_HEAP_MAX)
				|| (fault_va < USTACKTOP && fault_va >= USTACKBOTTOM))
		{

		}
		else{

			sched_kill_env(curenv->env_id);
		}

	}

	struct WorkingSetElement* workingSet = env_page_ws_list_create_element(curenv, fault_va);
	LIST_INSERT_TAIL(&curenv->page_WS_list, workingSet);


	if (curenv->page_WS_max_size == LIST_SIZE(&curenv->page_WS_list))
	{
		curenv->page_last_WS_element = curenv->page_WS_list.lh_first;
	}
	else
	{
		curenv->page_last_WS_element = NULL;

	}
}
void fifo_replacment(struct Env * curenv, uint32 fault_va)
{

	int perm = pt_get_page_permissions(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address);
if (perm & PERM_MODIFIED) // MODIFIED
{
	uint32 *ptr_page_table = NULL;
	struct FrameInfo *modified_frame = get_frame_info(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address, &ptr_page_table);
	int zz = pf_update_env_page(curenv, curenv->page_last_WS_element->virtual_address, modified_frame);
}

unmap_frame(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address);
env_page_ws_invalidate(curenv, curenv->page_last_WS_element->virtual_address);

struct FrameInfo *ptr_frame_info;
allocate_frame(&ptr_frame_info);
map_frame(curenv->env_page_directory, ptr_frame_info, fault_va, PERM_PRESENT | PERM_WRITEABLE | PERM_USER);
uint32 check = pf_read_env_page(curenv, (void *)fault_va);
if(check==E_PAGE_NOT_EXIST_IN_PF)
{
	if(
			(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX)||
			(fault_va<USTACKTOP&&fault_va>=USTACKBOTTOM)
			);
	else
	{
		unmap_frame(curenv->env_page_directory, curenv->page_last_WS_element->virtual_address);
		pt_set_page_permissions(curenv->env_page_directory,fault_va,0,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);
		sched_kill_env(curenv->env_id);
		return;
	}
}
struct WorkingSetElement *newelem = env_page_ws_list_create_element(curenv, fault_va);

if (curenv->page_last_WS_element == NULL)
{
	LIST_INSERT_TAIL(&curenv->page_WS_list, newelem);
	curenv->page_last_WS_element = curenv->page_WS_list.lh_first;
}
else
{
	LIST_INSERT_BEFORE(&curenv->page_WS_list, curenv->page_last_WS_element, newelem);
}
}
void lru_placment(struct Env * curenv, uint32 fault_va)
{
	fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);
	if(search_in_second(curenv,fault_va))
		return;

	struct FrameInfo *ptr_frame;
	allocate_frame(&ptr_frame);
	map_frame(curenv->env_page_directory, ptr_frame, fault_va, PERM_PRESENT|PERM_WRITEABLE | PERM_USER|PERM_MARKED);

	uint32 check= pf_read_env_page(curenv, (void *)fault_va);//read from disk

	if(check==  E_PAGE_NOT_EXIST_IN_PF)
	{
		if(
					(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX)||
					(fault_va<USTACKTOP&&fault_va>=USTACKBOTTOM));

		else
			{
				unmap_frame(curenv->env_page_directory, fault_va);
				pt_set_page_permissions(curenv->env_page_directory,fault_va,0,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);
				sched_kill_env(curenv->env_id);
				return;
			}


	}


		struct WorkingSetElement *newelem=env_page_ws_list_create_element(curenv, fault_va);

		int active_size=LIST_SIZE(&curenv->ActiveList);
		if(active_size<curenv->ActiveListSize)
		{
			LIST_INSERT_HEAD(&curenv->ActiveList,newelem);
		}
		else
		{
			remove_from_active(curenv);
			LIST_INSERT_HEAD(&curenv->ActiveList,newelem);
		}



}

int search_in_second(struct Env * curenv, uint32 fault_va)
{
	int is_in_second=0;
	 struct WorkingSetElement *ite=NULL;
	 struct WorkingSetElement *delelem=NULL;
	 LIST_FOREACH (ite,& curenv->SecondList)
	 {
		if((uint32)ite->virtual_address==fault_va)
		{
			is_in_second=1;
			break;
		}
	 }

	 if(!is_in_second)
		 return is_in_second;
	 LIST_REMOVE(&curenv->SecondList,ite);
	 remove_from_active(curenv);
	 pt_set_page_permissions(curenv->env_page_directory,ite->virtual_address,PERM_PRESENT,0);
	 LIST_INSERT_HEAD(&curenv->ActiveList,ite);
	 return is_in_second;
}

void update_env(struct Env * curenv,struct WorkingSetElement*second_last)
{

	uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory,second_last->virtual_address);
	if(page_permissions & PERM_MODIFIED)
	{
		uint32 *ptr_page_table = NULL ;
		struct FrameInfo *modified_frame=get_frame_info(curenv->env_page_directory,second_last->virtual_address,&ptr_page_table);
		int zz= pf_update_env_page(curenv,second_last->virtual_address,modified_frame);

	}
}

void remove_from_active(struct Env * curenv)
{
	struct WorkingSetElement * delelment=LIST_LAST(&curenv->ActiveList);
	LIST_REMOVE(&curenv->ActiveList, delelment);
	pt_set_page_permissions(curenv->env_page_directory,delelment->virtual_address,0,PERM_PRESENT);
	LIST_INSERT_HEAD(&curenv->SecondList, delelment);
}

void remove_from_second(struct Env * curenv)
{
	struct WorkingSetElement * delelment=LIST_LAST(&curenv->SecondList);
	update_env(curenv,delelment);
	LIST_REMOVE(&curenv->SecondList, delelment);
	unmap_frame(curenv->env_page_directory, delelment->virtual_address);
	kfree((void*)delelment);

}
void lru_replacment(struct Env * curenv, uint32 fault_va)
{
	fault_va=ROUNDDOWN(fault_va,PAGE_SIZE);



			if(search_in_second(curenv,fault_va))
				return;
			remove_from_second(curenv);
			remove_from_active(curenv);
			struct FrameInfo *ptr_frame;
				allocate_frame(&ptr_frame);
				map_frame(curenv->env_page_directory, ptr_frame, fault_va, PERM_PRESENT|PERM_WRITEABLE | PERM_USER|PERM_MARKED);

				uint32 check= pf_read_env_page(curenv, (void *)fault_va);//read from disk

				if(check==  E_PAGE_NOT_EXIST_IN_PF)
				{
					if(
								(fault_va>=USER_HEAP_START&&fault_va<USER_HEAP_MAX)||
								(fault_va<USTACKTOP&&fault_va>=USTACKBOTTOM));

					else
						{

							unmap_frame(curenv->env_page_directory, fault_va);
							pt_set_page_permissions(curenv->env_page_directory,fault_va,0,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);
							sched_kill_env(curenv->env_id);
							return;
						}


				}


					struct WorkingSetElement *newelem=env_page_ws_list_create_element(curenv, fault_va);
					LIST_INSERT_HEAD(&curenv->ActiveList,newelem);



}





