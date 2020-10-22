//============================================================================
// Version     : October, 2020
// Description : A solution for ENSC 351 Project Part 2
// Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <stdlib.h> // EXIT_SUCCESS
#include <sys/socket.h>
#include <pthread.h>
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
enum  {TermSkt, MediumSkt};

//static int daSktPr[2];	  //Socket Pair between term1 and term2
static int daSktPrT1M[2];	  //Socket Pair between term1 and medium
static int daSktPrMT2[2];	  //Socket Pair between medium and term2

void testReceiverX(const char* iFileName, int mediumD)
{
#define LIM 400
    char receiverFileName[LIM + 1];    /* +1 for terminating null byte */

  // For Checksum
    receiverFileName[0] = '\0';
    strcat(receiverFileName, iFileName);
    strcat(receiverFileName, "-cs-recd");

    COUT << "Will try to receive with Checksum to file:  " << receiverFileName << endl;
    ReceiverX xReceiverCS(mediumD, receiverFileName, false);
    xReceiverCS.receiveFile();
    COUT << "Receiver was receiving " << iFileName << " and had result: " << xReceiverCS.result << endl;

    receiverFileName[0] = '\0';
    strcat(receiverFileName, iFileName);
    strcat(receiverFileName, "-CRC-recd");

    COUT << "Will try to receive with CRC to file:  " << receiverFileName << endl;
    ReceiverX xReceiverCRC(mediumD, receiverFileName, true);
    xReceiverCRC.receiveFile();
    COUT << "Receiver was receiving " << iFileName << " and had result: " << xReceiverCRC.result << endl;

//    this_thread::sleep_for (chrono::milliseconds(5));
}

void testSenderX(const char* iFileName, int mediumD)
{
    // For checksum
    SenderX xSender(iFileName, mediumD);
    COUT << "Will try to send file:  " << iFileName << endl;
    xSender.sendFile();
    COUT << "Sender stopped sending " << iFileName << " with result: " << xSender.result << endl << endl;

    SenderX xSender2(iFileName, mediumD);
    COUT << "Will try to send file:  " << iFileName << endl;
    xSender2.sendFile();
    COUT << "Sender stopped sending " << iFileName << " with result: " << xSender2.result << endl << endl;
}

void termFunc(int termNum)
{
	// ***** modify this function to communicate with the "Kind Medium" *****

	if (termNum == Term1) {
        testReceiverX("hs_err_pid11506.log", daSktPrT1M[TermSkt]);         // normal text file
        testReceiverX("sudo_as_admin_successful", daSktPrT1M[TermSkt]);  // empty file
        testReceiverX("doesNotExist.txt", daSktPrT1M[TermSkt]);          // file does not exist

        this_thread::sleep_for (chrono::milliseconds(5)); // 10?
        PE(myClose(daSktPrT1M[TermSkt]));
	}
	else { // Term2
		PE_0(pthread_setname_np(pthread_self(), "T2")); // give the thread (terminal 2) a name
	    // PE_0(pthread_setname_np("T2")); // Mac OS X

	    testSenderX("/home/osboxes/hs_err_pid11506.log", daSktPrMT2[TermSkt]);        // normal text file
        testSenderX("/home/osboxes/.sudo_as_admin_successful", daSktPrMT2[TermSkt]);  // empty file
        testSenderX("/doesNotExist.txt", daSktPrMT2[TermSkt]);                        // file does not exist

//        this_thread::sleep_for (chrono::milliseconds(10));
        PE(myClose(daSktPrMT2[TermSkt]));
	}
//    std::this_thread::sleep_for (std::chrono::milliseconds(10));
//    PE(myClose(daSktPr[termNum]));
}

int Ensc351Part2()
{
	// ***** Modify this function to create the "Kind Medium" thread and communicate with it *****

	PE_0(pthread_setname_np(pthread_self(), "P-T1")); // give the primary thread (terminal 1) a name
    // PE_0(pthread_setname_np("P-T1")); // Mac OS X

	// ***** switch from having one socketpair for direct connection to having two socketpairs
	//			for connection through medium thread *****
	// PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr));
	//daSktPr[Term1] =  PE(/*myO*/open("/dev/ser2", O_RDWR));
    PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPrT1M));
    PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPrMT2));

    posixThread term2Thrd(SCHED_FIFO, 70, termFunc, Term2);

    // ***** create thread with SCHED_FIFO priority 40 for medium *****
    //     have the thread run the function found in Medium.cpp:
    //          void mediumFunc(int T1d, int T2d, const char *fname)
    //          where T1d is the descriptor for the socket to Term1
    //          and T2d is the descriptor for the socket to Term2
    //          and fname is the name of the binary medium "log" file
    //          ("xmodemData.dat").
    //      Make sure that thread is created at SCHED_FIFO priority 40

    posixThread mediumThrd(SCHED_FIFO, 40, mediumFunc, daSktPrT1M[MediumSkt],daSktPrMT2[MediumSkt], "xmodemData.dat"); // lower priority

	termFunc(Term1);

    term2Thrd.join();
    mediumThrd.join(); // ***** join with thread for medium *****

	return EXIT_SUCCESS;
}
