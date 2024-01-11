#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>


uint32 isSchedMethodRR(){if(scheduler_method == SCH_RR) return 1; return 0;}
uint32 isSchedMethodMLFQ(){if(scheduler_method == SCH_MLFQ) return 1; return 0;}
uint32 isSchedMethodBSD(){if(scheduler_method == SCH_BSD) return 1; return 0;}

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
	scheduler_status = SCH_STOPPED;
}

//=========================
// [2] Main FOS Scheduler:
//=========================
void
fos_scheduler(void)
{
	//	cprintf("inside scheduler\n");

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR)
	{
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL)
		{
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	}
	else if (scheduler_method == SCH_MLFQ)
	{
		next_env = fos_scheduler_MLFQ();
	}
	else if (scheduler_method == SCH_BSD)
	{
		next_env = fos_scheduler_BSD();
	}
	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env ;
	chk2(next_env) ;
	curenv = old_curenv;

	//sched_print_all();

	if(next_env != NULL)
	{
		//		cprintf("\nScheduler select program '%s' [%d]... counter = %d\n", next_env->prog_name, next_env->env_id, kclock_read_cnt0());
		//		cprintf("Q0 = %d, Q1 = %d, Q2 = %d, Q3 = %d\n", queue_size(&(env_ready_queues[0])), queue_size(&(env_ready_queues[1])), queue_size(&(env_ready_queues[2])), queue_size(&(env_ready_queues[3])));
		env_run(next_env);
	}
	else
	{
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

//=============================
// [3] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum)
{

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel)
{
#if USE_KHEAP
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
#endif
}

//===============================
// [5] Initialize BSD Scheduler:
//===============================
void sched_init_BSD(uint8 numOfLevels, uint8 quantum)
{
	num_of_ready_queues=numOfLevels;
#if USE_KHEAP
	//TODO: [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");
	sched_delete_ready_queues();
	env_ready_queues =kmalloc(num_of_ready_queues*sizeof(struct Env_Queue));
	quantums=kmalloc(sizeof(uint8));
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	for(int i=0;i<=num_of_ready_queues;i++){
		init_queue(&env_ready_queues[i]);
	}
	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_BSD;
	//=========================================
	//=========================================

}
//=========================
// [6] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ()
{
	panic("not implemented");
	return NULL;
}

//=========================
// [7] BSD Scheduler:
//=========================
struct Env* fos_scheduler_BSD()
{
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");
//lazm a3ml loop on kol queues i have and then get the first queue have a high pirioty and from it get the first process
//	sched_print_all();

	if(curenv!=NULL){
		LIST_INSERT_TAIL(&env_ready_queues[curenv->priority],curenv);
		curenv=NULL;
	}
	for(int i=PRI_MAX;i>=PRI_MIN;i--){
		if(queue_size(&env_ready_queues[i])>0){
					kclock_set_quantum(quantums[0]);
					struct Env* next_env = env_ready_queues[i].lh_first;
					remove_from_queue(&env_ready_queues[i],next_env);
					return next_env;
				}

	}

	load_avg.f=0;
//	sched_print_all();

	return NULL;
}

//========================================
// [8] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
void clock_interrupt_handler()
{
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here
if(isSchedMethodBSD())
{
//	sched_print_all();

	curenv->recent_cpu=fix_add(curenv->recent_cpu,fix_int(1));
	if((timer_ticks()+1)*quantums[0]%1000==0){
		for(int i=0 ; i<num_of_ready_queues;i++){
			ready_process+=queue_size(&env_ready_queues[i]);

		}

		if (curenv!=NULL){
			ready_process++;
			fixed_point_t one = fix_int(1);
			fixed_point_t two = fix_int(2);
			fixed_point_t recent = curenv->recent_cpu;
			fixed_point_t fnice = fix_int(curenv->nice);
			fixed_point_t numerator = fix_mul(two,load_avg);
			fixed_point_t denominator = fix_add(numerator,one);
			fixed_point_t valdiv = fix_div(numerator,denominator);
			fixed_point_t valadd = fix_add(recent,fnice);
			curenv->recent_cpu=fix_mul(valdiv,valadd);

		}
		fixed_point_t fiftynine = fix_int(59);
		fixed_point_t sixty = fix_int(60);
		fixed_point_t onee = fix_int(1);
		fixed_point_t f1 = fix_div(fiftynine,sixty);
		fixed_point_t f2 = fix_div(onee,sixty);
		fixed_point_t readyprocess = fix_int(ready_process);
		fixed_point_t firstbracket = fix_mul(f1,load_avg);
		fixed_point_t secondbracket = fix_mul(f2,readyprocess);
		load_avg=fix_add(firstbracket,secondbracket);


		for(int i=0 ; i<num_of_ready_queues;i++){

			struct Env * it = env_ready_queues[i].lh_first;
			while (it!=NULL)
			{
				fixed_point_t one = fix_int(1);
				fixed_point_t two = fix_int(2);
				fixed_point_t recent = it->recent_cpu;
				fixed_point_t fnice = fix_int(it->nice);
				fixed_point_t numerator = fix_mul(two,load_avg);
				fixed_point_t denominator = fix_add(numerator,one);
				fixed_point_t valdiv = fix_div(numerator,denominator);
				fixed_point_t valadd = fix_add(recent,fnice);
				it->recent_cpu=fix_mul(valdiv,valadd);
				it=it->prev_next_info.le_next;
			}

		}


	}
	if((timer_ticks()+1)%4==0){
		for(int i=0 ; i<num_of_ready_queues;i++){
			struct Env * it = env_ready_queues[i].lh_first;
			while(it!=NULL)
			{
				int	mypriority=it->priority;
				fixed_point_t max = fix_int(PRI_MAX);
				fixed_point_t recentover = fix_unscale(it->recent_cpu,4);
				fixed_point_t niceby = fix_int(it->nice * 2);
				fixed_point_t muinus = fix_sub(max,recentover);
				fixed_point_t muinus2 = fix_sub(muinus,niceby);
				it->priority=fix_trunc(muinus2);
				if(it->priority<PRI_MIN)
				{
					it->priority=PRI_MIN;
				}
				else if(it->priority>PRI_MAX)
				{
					it->priority=PRI_MAX;
				}
				if (it->priority!=mypriority){
				remove_from_queue(&env_ready_queues[mypriority],it);
				LIST_INSERT_TAIL(&env_ready_queues[it->priority],it);
				}
				it=it->prev_next_info.le_next;
			}
		}
		if (curenv!=NULL){
			fixed_point_t max = fix_int(PRI_MAX);
			fixed_point_t recentover = fix_unscale(curenv->recent_cpu,4);
			fixed_point_t niceby = fix_int(curenv->nice * 2);
			fixed_point_t muinus = fix_sub(max,recentover);
			fixed_point_t muinus2 = fix_sub(muinus,niceby);
			curenv->priority=fix_trunc(muinus2);
			if(curenv->priority<PRI_MIN)
			{
				curenv->priority=PRI_MIN;
			}
			else if(curenv->priority>PRI_MAX)
			{
				curenv->priority=PRI_MAX;
			}
		}
	}
//sched_print_all();

 }

	/********DON'T CHANGE THIS LINE***********/
	ticks++ ;
	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
	{
		update_WS_time_stamps();
	}
	//cprintf("Clock Handler\n") ;
	fos_scheduler();
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		struct WorkingSetElement* wse ;
		{
			int i ;
#if USE_KHEAP
			LIST_FOREACH(wse, &(curr_env_ptr->page_WS_list))
			{
#else
			for (i = 0 ; i < (curr_env_ptr->page_WS_max_size); i++)
			{
				wse = &(curr_env_ptr->ptr_pageWorkingSet[i]);
				if( wse->empty == 1)
					continue;
#endif
				//update the time if the page was referenced
				uint32 page_va = wse->virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr->env_page_directory, page_va) ;
				uint32 oldTimeStamp = wse->time_stamp;

				if (perm & PERM_USED)
				{
					wse->time_stamp = (oldTimeStamp>>2) | 0x80000000;
					pt_set_page_permissions(curr_env_ptr->env_page_directory, page_va, 0 , PERM_USED) ;
				}
				else
				{
					wse->time_stamp = (oldTimeStamp>>2);
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < __TWS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->__ptr_tws[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr->env_page_directory, table_va))
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr->env_page_directory, table_va);
					}
					else
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}

