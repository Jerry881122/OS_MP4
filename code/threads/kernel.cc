// kernel.cc 
//	Initialization and cleanup routines for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "main.h"
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"
#include "synchlist.h"
#include "libtest.h"
#include "string.h"
#include "synchdisk.h"
#include "post.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// Kernel::Kernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

Kernel::Kernel(int argc, char **argv)
{
    randomSlice = FALSE; 
    debugUserProg = FALSE;
    consoleIn = NULL;          // default is stdin
    consoleOut = NULL;         // default is stdout
#ifndef FILESYS_STUB
    formatFlag = FALSE;
#endif
    reliability = 1;            // network reliability, default is 1.0
    hostName = 0;               // machine id, also UNIX socket name
                                // 0 is the default machine id
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rs") == 0) {
 	    	ASSERT(i + 1 < argc);
	    	RandomInit(atoi(argv[i + 1]));// initialize pseudo-random
			// number generator
	    	randomSlice = TRUE;
	    	i++;
        } else if (strcmp(argv[i], "-s") == 0) {
            debugUserProg = TRUE;
		} else if (strcmp(argv[i], "-e") == 0) {
        	execfile[++execfileNum]= argv[++i];
			cout << execfile[execfileNum] << "\n";
            
		}
        /////////////////////////////////////////////////////////
        //11/29
        else if (strcmp(argv[i],"-ep") == 0) {
            // cout << "-ep statement !!" << endl; 
        	execfile[++execfileNum]= argv[++i];
            threadPriority[execfileNum] = atoi(argv[++i]);
            if(threadPriority[execfileNum]>149){
                threadPriority[execfileNum] = 149;
            }
            if(threadPriority[execfileNum]<0){
                threadPriority[execfileNum] = 0;
            }
            //11/29
            //filepriority[execfileNum] = atoi(argv[++i]);
        }
        /////////////////////////////////////////////////////////
        else if (strcmp(argv[i], "-ci") == 0) {
	    	ASSERT(i + 1 < argc);
	    	consoleIn = argv[i + 1];
	    	i++;
		} else if (strcmp(argv[i], "-co") == 0) {
	    	ASSERT(i + 1 < argc);
	    	consoleOut = argv[i + 1];
	    	i++;
#ifndef FILESYS_STUB
		} else if (strcmp(argv[i], "-f") == 0) {
	    	formatFlag = TRUE;
#endif
        } else if (strcmp(argv[i], "-n") == 0) {
            ASSERT(i + 1 < argc);   // next argument is float
            reliability = atof(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-m") == 0) {
            ASSERT(i + 1 < argc);   // next argument is int
            hostName = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-u") == 0) {
            cout << "Partial usage: nachos [-rs randomSeed]\n";
	   		cout << "Partial usage: nachos [-s]\n";
            cout << "Partial usage: nachos [-ci consoleIn] [-co consoleOut]\n";
#ifndef FILESYS_STUB
	    	cout << "Partial usage: nachos [-nf]\n";
#endif
            cout << "Partial usage: nachos [-n #] [-m #]\n";
		}
    }
}

//----------------------------------------------------------------------
// Kernel::Initialize
// 	Initialize Nachos global data structures.  Separate from the 
//	constructor because some of these refer to earlier initialized
//	data via the "kernel" global variable.
//----------------------------------------------------------------------

void
Kernel::Initialize()
{
    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 

	
    currentThread = new Thread("main", threadNum++);
    currentThread->setStatus(RUNNING);

    stats = new Statistics();		// collect statistics
    interrupt = new Interrupt;		// start up interrupt handling
    scheduler = new Scheduler();	// initialize the ready queue
    alarm = new Alarm(randomSlice);	// start up time slicing
    machine = new Machine(debugUserProg);
    synchConsoleIn = new SynchConsoleInput(consoleIn); // input from stdin
    synchConsoleOut = new SynchConsoleOutput(consoleOut); // output to stdout
    synchDisk = new SynchDisk();    //
#ifdef FILESYS_STUB
    fileSystem = new FileSystem();
#else
    fileSystem = new FileSystem(formatFlag);
#endif // FILESYS_STUB
    //12/20
    //postOfficeIn = new PostOfficeInput(10);
    //postOfficeOut = new PostOfficeOutput(reliability);

    /************************************/
    usedPhyPage = new UsedPhyPage();
    /************************************/

    interrupt->Enable();
}

//----------------------------------------------------------------------
// Kernel::~Kernel
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------

Kernel::~Kernel()
{
    delete stats;
    delete interrupt;
    delete scheduler;
    delete alarm;
    delete machine;
    delete synchConsoleIn;
    delete synchConsoleOut;
    delete synchDisk;
    delete fileSystem;
    //delete postOfficeIn;
    //delete postOfficeOut;

    /************************************/
    delete usedPhyPage;
    /************************************/
    
    Exit(0);
}

//----------------------------------------------------------------------
// Kernel::ThreadSelfTest
//      Test threads, semaphores, synchlists
//----------------------------------------------------------------------

void
Kernel::ThreadSelfTest() {
   Semaphore *semaphore;
   SynchList<int> *synchList;
   
   LibSelfTest();		// test library routines
   
   currentThread->SelfTest();	// test thread switching
   
   				// test semaphore operation
   semaphore = new Semaphore("test", 0);
   semaphore->SelfTest();
   delete semaphore;
   
   				// test locks, condition variables
				// using synchronized lists
   synchList = new SynchList<int>;
   synchList->SelfTest(9);
   delete synchList;

}

//----------------------------------------------------------------------
// Kernel::ConsoleTest
//      Test the synchconsole
//----------------------------------------------------------------------

void
Kernel::ConsoleTest() {
    char ch;

    cout << "Testing the console device.\n" 
        << "Typed characters will be echoed, until ^D is typed.\n"
        << "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    do {
        ch = synchConsoleIn->GetChar();
        if(ch != EOF) synchConsoleOut->PutChar(ch);   // echo it!
    } while (ch != EOF);

    cout << "\n";

}

//----------------------------------------------------------------------
// Kernel::NetworkTest
//      Test whether the post office is working. On machines #0 and #1, do:
//
//      1. send a message to the other machine at mail box #0
//      2. wait for the other machine's message to arrive (in our mailbox #0)
//      3. send an acknowledgment for the other machine's message
//      4. wait for an acknowledgement from the other machine to our 
//          original message
//
//  This test works best if each Nachos machine has its own window
//----------------------------------------------------------------------

void
Kernel::NetworkTest() {

    if (hostName == 0 || hostName == 1) {
        // if we're machine 1, send to 0 and vice versa
        int farHost = (hostName == 0 ? 1 : 0); 
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "Hello there!";
        char *ack = "Got it!";
        char buffer[MaxMailSize];

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farHost;         
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOfficeOut->Send(outPktHdr, outMailHdr, data); 

        // Wait for the first message from the other machine
        postOfficeIn->Receive(0, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOfficeOut->Send(outPktHdr, outMailHdr, ack); 

        // Wait for the ack from the other machine to the first message we sent
	postOfficeIn->Receive(1, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box " 
                                                << inMailHdr.from << "\n";
        cout.flush();
    }

    // Then we're done!
}

void ForkExecute(Thread *t)
{
	if ( !t->space->Load(t->getName()) ) {
    	return;             // executable not found
    }
	
    t->space->Execute(t->getName());

}

void Kernel::ExecAll()
{
    /*****************************************************************************/
    // cout << "In kernel::ExecAll" << endl; 
     //cout << "\texecfileNum = " << execfileNum << endl;
    // execfileNum 會在 command line 時就決定，記錄要跑幾個程式
    /*****************************************************************************/

	for (int i=1;i<=execfileNum;i++) {
        /*****************************************************************/
        // cout << "In kernel::ExecAll" << endl;
        // cout << "\tfile " << i << endl;
        /*****************************************************************/
        // cout << "execute : " << execfile[i] << endl;
		//int a = Exec(execfile[i]);
        int a = Exec(execfile[i],threadPriority[i]);
	}
    // cout << "currectThread = " << currentThread->getName() << endl;
	currentThread->Finish();
    //Kernel::Exec();	
}


int Kernel::Exec(char* name,int priority)//11/29
{   
    /************************************************/
    // t 為 Thread*

	t[threadNum] = new Thread(name, threadNum);     
    // 新增 Thread
    // 此時 status 為 JUST_CREATED

    //11/29
	t[threadNum]->setBurstTime(0);
    t[threadNum]->setWaitingTime(0);
    t[threadNum]->setExecutionTime(0);
    t[threadNum]->setPriority(priority);     
    //
    //11/29
    DEBUG('z',"[C] Tick [" << kernel->stats->totalTicks << "]:Thread [" << threadNum << "] changes its priority from [0] to ["<< priority << "]");

    // 12/27
    t[threadNum]->initial_priority = priority;
    t[threadNum]->aging_flag = 0;

	t[threadNum]->space = new AddrSpace();
    // space 為 AddrSpace*
    // 這邊的 new 原本會製造 pagetable，但因為我們要做 runtime address binding 故這邊先什麼都不做
    
	t[threadNum]->Fork((VoidFunctionPtr) &ForkExecute, (void *)t[threadNum]);

    //11/29
    //t[threadNum]->setPriority(filepriority[threadNum]);
    //cout<<"filepriority: "<< filepriority[threadNum]<<endl;
	threadNum++;

	return threadNum-1;
/*
    cout << "Total threads number is " << execfileNum << endl;
    for (int n=1;n<=execfileNum;n++) {
		t[n] = new Thread(execfile[n]);
		t[n]->space = new AddrSpace();
		t[n]->Fork((VoidFunctionPtr) &ForkExecute, (void *)t[n]);
		cout << "Thread " << execfile[n] << " is executing." << endl;
	}
	cout << "debug Kernel::Run finished.\n";	
*/
//  Thread *t1 = new Thread(execfile[1]);
//  Thread *t1 = new Thread("../test/test1");
//  Thread *t2 = new Thread("../test/test2");

//    AddrSpace *halt = new AddrSpace();
//  t1->space = new AddrSpace();
//  t2->space = new AddrSpace();

//    halt->Execute("../test/halt");
//  t1->Fork((VoidFunctionPtr) &ForkExecute, (void *)t1);
//  t2->Fork((VoidFunctionPtr) &ForkExecute, (void *)t2);

//	currentThread->Finish();
//    Kernel::Run();
//  cout << "after ThreadedKernel:Run();" << endl;  // unreachable
}

int Kernel::CreateFile(char *filename)
{
	return fileSystem->Create(filename);
}
//10/31 example 2
void Kernel::PrintInt(int number)
{
return synchConsoleOut->PutInt(number);
}

