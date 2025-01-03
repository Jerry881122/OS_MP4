// synchconsole.cc 
//	Routines providing synchronized access to the keyboard 
//	and console display hardware devices.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// SynchConsoleInput::SynchConsoleInput
//      Initialize synchronized access to the keyboard
//
//      "inputFile" -- if NULL, use stdin as console device
//              otherwise, read from this file
//----------------------------------------------------------------------

SynchConsoleInput::SynchConsoleInput(char *inputFile)
{
    consoleInput = new ConsoleInput(inputFile, this);
    lock = new Lock("console in");
    waitFor = new Semaphore("console in", 0);
}

//----------------------------------------------------------------------
// SynchConsoleInput::~SynchConsoleInput
//      Deallocate data structures for synchronized access to the keyboard
//----------------------------------------------------------------------

SynchConsoleInput::~SynchConsoleInput()
{ 
    delete consoleInput; 
    delete lock; 
    delete waitFor;
}

//----------------------------------------------------------------------
// SynchConsoleInput::GetChar
//      Read a character typed at the keyboard, waiting if necessary.
//----------------------------------------------------------------------

char
SynchConsoleInput::GetChar()
{
    char ch;

    lock->Acquire();
    waitFor->P();	// wait for EOF or a char to be available.
    ch = consoleInput->GetChar();
    lock->Release();
    return ch;
}

//----------------------------------------------------------------------
// SynchConsoleInput::CallBack
//      Interrupt handler called when keystroke is hit; wake up
//	anyone waiting.
//----------------------------------------------------------------------

void
SynchConsoleInput::CallBack()
{
    waitFor->V();
}

//----------------------------------------------------------------------
// SynchConsoleOutput::SynchConsoleOutput
//      Initialize synchronized access to the console display
//
//      "outputFile" -- if NULL, use stdout as console device
//              otherwise, read from this file
//----------------------------------------------------------------------

SynchConsoleOutput::SynchConsoleOutput(char *outputFile)
{
    consoleOutput = new ConsoleOutput(outputFile, this);
    lock = new Lock("console out");
    waitFor = new Semaphore("console out", 0);
}

//----------------------------------------------------------------------
// SynchConsoleOutput::~SynchConsoleOutput
//      Deallocate data structures for synchronized access to the keyboard
//----------------------------------------------------------------------

SynchConsoleOutput::~SynchConsoleOutput()
{ 
    delete consoleOutput; 
    delete lock; 
    delete waitFor;
}

//----------------------------------------------------------------------
// SynchConsoleOutput::PutChar
//      Write a character to the console display, waiting if necessary.
//----------------------------------------------------------------------
////ori
void
SynchConsoleOutput::PutChar(char ch)
{
    lock->Acquire();
    consoleOutput->PutChar(ch);
    waitFor->P();
    lock->Release();
}
//11/1
// void
// SynchConsoleOutput::PutInt(int value)
// {
//     char str[15];
//     int idx=0;
//     sprintf(str, "%d\n\0", value);
//     //cout<<"\n";
//     lock->Acquire();
//     do{
//         //cout<<"idx:"<<idx<<"str:"<<str[idx];
//         consoleOutput->PutChar(str[idx]);
//         idx++;
//         waitFor->P();
//         kernel->stats->numConsoleCharsWritten--; 
//     }while(str[idx] != '\0');
//     // while (str[idx] != '\0') {
//     //     consoleOutput->PutChar(str[idx]);
//     //     idx++;
//     //     waitFor->P();
//     //     kernel->stats->numConsoleCharsWritten--; 
//     // }
//     //consoleOutput->PutChar('\n');
//     //waitFor->P();
//     lock->Release();
// }


//ori//gg
// void SynchConsoleOutput::PutInt(int val){
//     char str[20];
//     int idx = 0;
    
//     sprintf(str, "%d\n\0", val); //simply for trace code


//     lock->Acquire();
//     consoleOutput->PutString(str);
//     waitFor->P();
//     lock->Release();
// }
//10/31 for example
void SynchConsoleOutput::PutInt(int value){
    lock->Acquire();
    
    consoleOutput->PutInt(value);
    //cout << "\n****************** up" << endl;
    kernel->currentThread->Yield();
    //cout << "****************** bottom" << endl;
    waitFor->P();
    // cout << "call P() in PutInt" << endl;
    lock->Release();
}

//----------------------------------------------------------------------
// SynchConsoleOutput::CallBack
//      Interrupt handler called when it's safe to send the next 
//	character can be sent to the display.
//----------------------------------------------------------------------

void
SynchConsoleOutput::CallBack()
{
    waitFor->V();
}
