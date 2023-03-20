// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void increasePC() {
    int countNum = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg,countNum);
    machine->WriteRegister(PCReg,countNum);
    machine->WriteRegister(NextPCReg,countNum+ 4);
}

char* User2System(int virtAddr,int limit) {
    int i;// index
    int oneChar;
    char* kernelBuf = NULL;
    kernelBuf = new char[limit +1];//need for terminal string
    if (kernelBuf == NULL)
    return kernelBuf;
    memset(kernelBuf,0,limit+1);

 //printf("\n Filename u2s:");
    for (i = 0 ; i < limit ;i++) {
        machine->ReadMem(virtAddr+i,1,&oneChar);
        kernelBuf[i] = (char)oneChar;
 //printf("%c",kernelBuf[i]);
        if (oneChar == 0)
        break;
    }
    return kernelBuf;
}
/*
Input: - User space address (int)
 - Limit of buffer (int)
 - Buffer (char[])
Output:- Number of bytes copied (int)
Purpose: Copy buffer from System memory space to User memory space
*/
int System2User(int virtAddr,int len,char* buffer) {
    if (len < 0) return -1;
    if (len == 0)return len;
    int i = 0;
    int oneChar = 0 ;
    do{
        oneChar= (int) buffer[i];
        machine->WriteMem(virtAddr+i,1,oneChar);
        i ++;
    }  while(i < len && oneChar != 0);
    return i;
}

/*
    thanh ghi r4,r5,r6,r7 dai dien cho lan luot cac tham so duoc truyen vao trong ham la tham so thu 1, 2, 3, 4
    thanh ghi r2 ban dau dung de chua gia tri cua type sau day thi dung de chua ket qua tra ve neu co


*/

int pow(int n){
    int kq = 1;
    for(int i=1;i<n;i++){
        kq = kq * 10;
    }
    return kq;
}


void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which) {
    case NoException:
    return; 

    case SyscallException:
        switch (type) {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_ReadInt:
                char intBuffer[10];
                int temp;
                int size = SynchConsole->read(intBuffer,10); //Doc tu terminal size 10
                if(size == -1) machine->WriteRegister(2,0); 
                else {
                    int digit = 1;
                    int sign = 1;
                    for(int i =size -1 ;i >= 0;i--){
                        if(intBuffer[i] == '-') {
                            sign = -1;
                        }else if(intBuffer[i] >= '0' && intBuffer[i] <= '9'){
                            temp = (intBuffer[i] - '0') * pow(digit);
                        }else{
                            machine->WriteRegister(2,0); 
                            break;
                        }
                        digit++;
                    }
                    temp = temp*sign;
                    machine->WriteRegister(2,temp); 
                   
                }
                increasePC();
                break;
            case SC_PrintInt:
                int number = machine->ReadRegister(4);
                int size = 0;
                char buffer[10];
                while(number > 0){
                    buffer = number%10 + '0';
                    number /= 10;
                    size++;
                }
                SynchConsole->write(buffer,size);
                increasePC();
                break;

        }

        default:
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
        break;
    }
}
