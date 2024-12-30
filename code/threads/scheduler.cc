// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------


static int compareL2(Thread *t1, Thread *t2){

    if(t1->getPriority()> t2->getPriority()){
        return -1;
    }
    else if(t1->getPriority() < t2->getPriority()){
        return 1;
    } 
    else{
        return t1->getID() < t2->getID() ? -1:1;
    }
    return 0;
}
static int compareL1(Thread *t1, Thread *t2){

    if(t1->getBurstTime()> t2->getBurstTime()){
        return  1;
    }
    else if(t1->getBurstTime() < t2->getBurstTime()){
        return -1;
    }
    else{
        return t1->getID() < t2->getID() ? -1:1;
    }
    return 0;
}
void Scheduler::updatePriority()
{
    ListIterator<Thread *> *iter1 = new ListIterator<Thread*>(L1ReadyList);
    ListIterator<Thread *> *iter2 = new ListIterator<Thread*>(L2ReadyList);
    ListIterator<Thread *> *iter3 = new ListIterator<Thread*>(L3ReadyList);
    Statistics *stats = kernel->stats;

    int oldPriority;
    int newPriority; 

    for(;!iter1->IsDone();iter1->Next())
    {
        ASSERT(iter1->Item()->getStatus()==READY);

        iter1->Item()->setWaitingTime(iter1->Item()->getWaitingTime()+TimerTicks);
        if(iter1->Item()->getWaitingTime() >=1500 && iter1->Item()->getID()>0)
        {
            oldPriority = iter1->Item()->getPriority();
            newPriority = oldPriority + 10;
            DEBUG('z',"[C] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iter1->Item()->getID()<< "] changes its priority from [" << oldPriority << "] to ["<< newPriority << "]");

            if(newPriority>149)
            {
                newPriority = 149;
            }
            iter1->Item()->aging_flag = 1;
            iter1->Item()->setPriority(newPriority);
            iter1->Item()->setWaitingTime(0);
        }
    }
    for(;!iter2->IsDone();iter2->Next())
    {
        ASSERT(iter2->Item()->getStatus()==READY);

        iter2->Item()->setWaitingTime(iter2->Item()->getWaitingTime()+TimerTicks);
        if(iter2->Item()->getWaitingTime() >=3000 && iter2->Item()->getID()>0)
        {
            oldPriority = iter2->Item()->getPriority();
            newPriority = oldPriority + 10;
            DEBUG('z',"[C] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iter2->Item()->getID()<< "] changes its priority from [" << oldPriority << "] to ["<< newPriority << "]");
            if(newPriority>149)
            {
                newPriority = 149;
            }
            iter2->Item()->setPriority(newPriority);
            iter2->Item()->aging_flag = 1;
            iter2->Item()->setWaitingTime(0);
            L2ReadyList->Remove(iter2->Item());
            ReadyToRun(iter2->Item());
        }
    }
    for(;!iter3->IsDone();iter3->Next())
    {
        ASSERT(iter3->Item()->getStatus()==READY);
        iter3->Item()->setWaitingTime(iter3->Item()->getWaitingTime()+TimerTicks);
        if(iter3->Item()->getWaitingTime() >=1500 && iter3->Item()->getID()>0)
        {
            oldPriority = iter3->Item()->getPriority();
            newPriority = oldPriority + 10;
            DEBUG('z',"[C] Tick [" << kernel->stats->totalTicks << "]: Thread [" << iter3->Item()->getID()<< "] changes its priority from [" << oldPriority << "] to ["<< newPriority << "]");            
            if(newPriority>149)
            {
                newPriority = 149;
            }
            iter3->Item()->setPriority(newPriority);
            iter3->Item()->aging_flag = 1;
            iter3->Item()->setWaitingTime(0);
            Thread* tmpThread = iter3->Item();
            L3ReadyList->Remove(tmpThread);
            ReadyToRun(tmpThread); 
        }
    }
}



Scheduler::Scheduler()
{
    L2ReadyList = new SortedList<Thread *>(compareL2);
    L1ReadyList = new SortedList<Thread *>(compareL1);
    L3ReadyList = new List<Thread *>;
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete L2ReadyList;
    delete L1ReadyList;
    delete L3ReadyList;

} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread){

    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getID()); 
    thread->setStatus(READY);
    if(thread->getPriority() >= 100 && thread->getPriority() <= 149){

        if(!L1ReadyList->IsInList(thread)){

            DEBUG('z' , "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID()<< "] is inserted into queue L[1]");            
            L1ReadyList->Insert(thread);
        }
    }
    else if(thread->getPriority() >= 50 && thread->getPriority() <= 99){
        if(!L2ReadyList->IsInList(thread)){
            DEBUG('z' , "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID()<< "] is inserted into queue L[2]");
            L2ReadyList->Insert(thread);
        }
    }
    else
    {
        if(!L3ReadyList->IsInList(thread))
        {
            DEBUG('z' , "[A] Tick [" << kernel->stats->totalTicks << "]:Thread [" << thread->getID()<< "] is inserted into queue L[3]");
            L3ReadyList->Append(thread);
        }
    }
}

// 12/26
void Scheduler::ApproximateBurstTime(Thread *thread){

    if(kernel->stats->mainTicks != 0){  // main function 完成才開始進行

        if(thread->first_exe==0)
            thread->first_exe = 1;

        double prevBurstTime = thread->getBurstTime();
        thread->setExecutionTime( (kernel->stats->totalTicks) - (thread->getStartexcution()));
        double newBurstTime = 0.5 * prevBurstTime + 0.5 * thread->getExecutionTime();
        thread->setBurstTime(newBurstTime);
        DEBUG('z',"[D] Tick [" << kernel->stats->totalTicks << "]: Thread ["<< thread->getID() 
            << "] update approximate burst time, from: [" << prevBurstTime 
            <<  "], add [" << thread->getExecutionTime() <<"], to ["<< newBurstTime <<"]" );
        //////
        // cout << "startTime = " << thread->getStartexcution() << endl;
        // cout << "New execution Time = " << thread->getExecutionTime() << endl;
        // cout << "---" << endl;
        // cout << "prevBurstTime" << prevBurstTime << endl;
        // cout << "newBurstTime" << newBurstTime << endl;
    } 

}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun (){

    ASSERT(kernel->interrupt->getLevel() == IntOff);
    if (!L1ReadyList->IsEmpty()){
        Thread *a = L1ReadyList->RemoveFront();
        if(a->aging_flag == 1){
            a->setPriority(a->initial_priority);
        }
        a->setStartexcution(kernel->stats->totalTicks);
        DEBUG('z',"[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << a->getID()<< "] is removed from queue L[1]");
        return a;
    } 
    else if(!L2ReadyList->IsEmpty()){
        Thread *a = L2ReadyList->RemoveFront();
        if(a->aging_flag == 1){
            a->setPriority(a->initial_priority);
        }
        a->setStartexcution(kernel->stats->totalTicks);
        DEBUG('z',"[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << a->getID()<< "] is removed from queue L[2]");
        return a;
    }
    else if(!L3ReadyList->IsEmpty()){

        Thread *a = L3ReadyList->RemoveFront();

        if(a->aging_flag == 1){
            a->setPriority(a->initial_priority);
        }
        a->setStartexcution(kernel->stats->totalTicks);
        DEBUG('z',"[B] Tick [" << kernel->stats->totalTicks << "]:Thread [" << a->getID()<< "] is removed from queue L[3]");
        return a;
    }
    else{
        return NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing){

    Thread *oldThread = kernel->currentThread;
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    if (finishing) {	// mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
	    toBeDestroyed = oldThread;
    }
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow
    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".


    // 12/25
    DEBUG('z', "[E] Tick [" << kernel->stats->totalTicks << "]:Thread [" << nextThread->getID()<< "] is now selected for execution, thread [" << oldThread->getID() << "] is replaced, and it has executed ["<< oldThread->getExecutionTime() << "] ticks");
    
    SWITCH(oldThread, nextThread);
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    /**********************************************/
    CheckToBeDestroyed();		// check if thread we were running
					            // before this one has finished
					            // and needs to be cleaned up
    // 若 ToBeDestroyed 有指到東西則 delete 他
    /**********************************************/
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        // cout << "call Destoryed , Thread[" << toBeDestroyed->getID() << "]" << endl;  
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print(){
    
    cout << "Ready list contents : ";
    L1ReadyList->Apply(ThreadPrint);
    cout << "\n";
}



