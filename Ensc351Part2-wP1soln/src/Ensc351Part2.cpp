//============================================================================
//
//% Student Name 1: student1
//% Student 1 #: 123456781
//% Student 1 userid (email): stu1 (stu1@sfu.ca)
//
//% Student Name 2: student2
//% Student 2 #: 123456782
//% Student 2 userid (email): stu2 (stu2@sfu.ca)
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: _everybody helped us/me with the assignment (list names or put 'None')__
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  ___________
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P2_<userid1>_<userid2>" (eg. P2_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// Version     : September 28, 2020
// Copyright   : Copyright 2020, Craig Scratchley
// Description : Starting point for ENSC 351 Project Part 2
//============================================================================

#include <stdlib.h> // EXIT_SUCCESS
#include <sys/socket.h>
#include <pthread.h>
//#include <thread>
#include <chrono>         // std::chrono::

#include "myIO.h"
#include "SenderX.h"
#include "ReceiverX.h"
#include "Medium.h"

#include "VNPE.h"
#include "AtomicCOUT.h"
#include "posixThread.hpp"

using namespace std;
using namespace pthreadSupport;

enum  {Term1, Term2};
//enum  {TermSkt, MediumSkt};

static int daSktPr[2];	  //Socket Pair between term1 and term2
//static int daSktPrT1M[2];	  //Socket Pair between term1 and medium
//static int daSktPrMT2[2];	  //Socket Pair between medium and term2

void testReceiverX(const char* iFileName, int mediumD)
{
#define LIM 400
    char receiverFileName[LIM + 1];    /* +1 for terminating null byte */

/*  // For Checksum
    receiverFileName[0] = '\0';
    strcat(receiverFileName, iFileName);
    strcat(receiverFileName, "-cs-recd");

    COUT << "Will try to receive with Checksum to file:  " << receiverFileName << endl;
    ReceiverX xReceiverCS(mediumD, receiverFileName, false);
    xReceiverCS.receiveFile();
    COUT << "xReceiver result was: " << xReceiverCS.result << endl << endl;
*/
    receiverFileName[0] = '\0';
    strcat(receiverFileName, iFileName);
    strcat(receiverFileName, "-CRC-recd");

    COUT << "Will try to receive with CRC to file:  " << receiverFileName << endl;
    ReceiverX xReceiverCRC(mediumD, receiverFileName, true);
    xReceiverCRC.receiveFile();
    COUT << "xReceiver result was: " << xReceiverCRC.result << endl  << endl;

    this_thread::sleep_for (chrono::milliseconds(5));
}

void testSenderX(const char* iFileName, int mediumD)
{
    /* // For checksum
    SenderX xSender(iFileName, mediumD);
    COUT << "test sending" << endl;
    xSender.sendFile();
    COUT << "Sender finished with result: " << xSender.result << endl;
*/
    SenderX xSender2(iFileName, mediumD);
    COUT << "Will try to send file:  " << iFileName << endl;
    xSender2.sendFile();
    COUT << "Sender finished with result: " << xSender2.result << endl;
}

void termFunc(int termNum)
{
	// ***** modify this function to communicate with the "Kind Medium" *****

	if (termNum == Term1) {
        testReceiverX("hs_err_pid11506.log", daSktPr[Term1]);        // normal text file
//        testReceiverX("sudo_as_admin_successful", daSktPr[Term1]);  // empty file
//        testReceiverX("doesNotExist.txt", daSktPr[Term1]);                        // file does not exist
	}
	else { // Term2
		PE_0(pthread_setname_np(pthread_self(), "T2")); // give the thread (terminal 2) a name
	    // PE_0(pthread_setname_np("T2")); // Mac OS X

	    testSenderX("/home/osboxes/hs_err_pid11506.log", daSktPr[Term2]);        // normal text file
//        testSenderX("/home/osboxes/.sudo_as_admin_successful", daSktPr[Term2]);  // empty file
//        testSenderX("/doesNotExist.txt", daSktPr[Term2]);                        // file does not exist
	}
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
	PE(myClose(daSktPr[termNum]));
}

int Ensc351Part2()
{
	// ***** Modify this function to create the "Kind Medium" thread and communicate with it *****

	PE_0(pthread_setname_np(pthread_self(), "P-T1")); // give the primary thread (terminal 1) a name
    // PE_0(pthread_setname_np("P-T1")); // Mac OS X

	// ***** switch from having one socketpair for direct connection to having two socketpairs
	//			for connection through medium thread *****
	PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr));
	//daSktPr[Term1] =  PE(/*myO*/open("/dev/ser2", O_RDWR));

    posixThread term2Thrd(SCHED_FIFO, 70, termFunc, Term2);

    // ***** create thread with SCHED_FIFO priority 40 for medium *****
    //     have the thread run the function found in Medium.cpp:
    //          void mediumFunc(int T1d, int T2d, const char *fname)
    //          where T1d is the descriptor for the socket to Term1
    //          and T2d is the descriptor for the socket to Term2
    //          and fname is the name of the binary medium "log" file
    //          ("xmodemData.dat").
    //      Make sure that thread is created at SCHED_FIFO priority 40

	termFunc(Term1);

    term2Thrd.join();
    // ***** join with thread for medium *****

	return EXIT_SUCCESS;
}
