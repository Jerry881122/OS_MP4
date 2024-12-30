// kernel.h
//	Global variables for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef KERNEL_H
#define KERNEL_H

#include "copyright.h"
#include "debug.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "alarm.h"
#include "filesys.h"
#include "machine.h"

class PostOfficeInput;
class PostOfficeOutput;
class SynchConsoleInput;
class SynchConsoleOutput;
class SynchDisk;


/**************************/
class UsedPhyPage {
public:
    UsedPhyPage(){
      pages = new int[NumPhysPages];
      memset(pages, 0, sizeof(int) * NumPhysPages);
    };

    ~UsedPhyPage(){
      delete[] pages;
    };

    int numUnused(){
        int count = 0;
        for(int i = 0; i < NumPhysPages; i++) {
            if(pages[i] == 0) 
                count++;
        }
        return count;
    };

    int setPhyAddr(){
       int pickPhyPage = -1;

       while(1){
         pickPhyPage = rand()%NumPhysPages;
          if(pages[pickPhyPage] == 0) {
                  break;
          }
       }
        pages[pickPhyPage] = 1;
        return pickPhyPage;
    };
    
    int *pages; /* 0 for unused, 1 for used */
};
/**************************/


class Kernel {
  public:
    Kernel(int argc, char **argv);
    				// Interpret command line arguments
    ~Kernel();		        // deallocate the kernel
    
    void Initialize(); 		// initialize the kernel -- separated
				// from constructor because 
				// refers to "kernel" as a global
	void ExecAll();
	int Exec(char* name,int priority);
    void ThreadSelfTest();	// self test of threads and synchronization
	
    void ConsoleTest();         // interactive console self test
    void NetworkTest();         // interactive 2-machine network test
	Thread* getThread(int threadID){return t[threadID];}    
	
	int CreateFile(char* filename); // fileSystem call

  //10/31 add for example 2
  void PrintInt(int number);
  ///
// These are public for notational convenience; really, 
// they're global variables used everywhere.

    Thread *currentThread;	// the thread holding the CPU
    Scheduler *scheduler;	// the ready list
    Interrupt *interrupt;	// interrupt status
    Statistics *stats;		// performance metrics
    Alarm *alarm;		// the software alarm clock    
    Machine *machine;           // the simulated CPU
    SynchConsoleInput *synchConsoleIn;
    SynchConsoleOutput *synchConsoleOut;
    SynchDisk *synchDisk;
    FileSystem *fileSystem;     
    PostOfficeInput *postOfficeIn;
    PostOfficeOutput *postOfficeOut;


    /**************************/
    UsedPhyPage* usedPhyPage;
    /**************************/

    int hostName;               // machine identifier
  /**************************/
  //11/29
	//Thread* t[10];
  int threadNum;
  /***************************/
  private:
    Thread* t[51];
    int threadPriority[51];
    char*  execfile[51];  

	//char*   execfile[10];
	int execfileNum;

    bool randomSlice;		// enable pseudo-random time slicing
    bool debugUserProg;         // single step user program
    double reliability;         // likelihood messages are dropped
    char *consoleIn;            // file to read console input from
    char *consoleOut;           // file to send console output to
  //11/29
    int filepriority[10];

  //11/29
  // Thread* t[51];
  // int threadPriority[51];
  // char*  execfile[51];  

#ifndef FILESYS_STUB
    bool formatFlag;          // format the disk if this is true
#endif
};


#endif // KERNEL_H


