//============================================================================
//
//% Student Name 1: Ryan Stolys
//% Student 1 #: 301303127
//% Student 1 userid (email): rstolys (rstolys@sfu.ca)
//
//% Student Name 2: Matthew Nesdoly
//% Student 2 #: 301328738
//% Student 2 userid (email): mnesdoly (mnesdoly@sfu.ca)
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: 
//          1. Worked with Jayden Cole on determing purpose of project files
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  None
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

#include <stdlib.h>         // EXIT_SUCCESS
#include <sys/socket.h>
#include <pthread.h>
//#include <thread>
#include <chrono>           // std::chrono::

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


////////////////////////////////////////////////////////////////
//
// Setup the reciever thread
//
////////////////////////////////////////////////////////////////
void testReceiverX(const char* iFileName, int mediumD)
    {
#define LIM 400

    char receiverFileName[LIM + 1];    /* +1 for terminating null byte */

/*  
    // For Checksum
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
    ReceiverX xReceiverCRC(mediumD, receiverFileName, false);
    xReceiverCRC.receiveFile();
    COUT << "xReceiver result was: " << xReceiverCRC.result << endl  << endl;

    this_thread::sleep_for (chrono::milliseconds(5));
    }


////////////////////////////////////////////////////////////////
//
// Setup the sender thread
//
////////////////////////////////////////////////////////////////
void testSenderX(const char* iFileName, int mediumD)
    {
/* 
    // For checksum
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


////////////////////////////////////////////////////////////////
//
// Starting point for the reciever and sender threads
//
////////////////////////////////////////////////////////////////
void termFunc(int termNum)
{   
	if(termNum == Term1) 
        {
        testReceiverX("hs_err_pid11506.log", daSktPrT1M[TermSkt]);          // normal text file
        //testReceiverX("sudo_as_admin_successful", daSktPrT1M[TermSkt]);        // empty file
        //testReceiverX("doesNotExist.txt", daSktPrT1M[TermSkt]);                // file does not exist
	    }
	else  // Term2
        { 
		PE_0(pthread_setname_np(pthread_self(), "T2"));                     // give the thread (terminal 2) a name

	    testSenderX("/home/osboxes/hs_err_pid11506.log", daSktPrMT2[TermSkt]);          // normal text file
        //testSenderX("/home/osboxes/.sudo_as_admin_successful", daSktPrT1M[TermSkt]);       // empty file
        //testSenderX("/doesNotExist.txt", daSktPrT1M[TermSkt]);                             // file does not exist
	    }

    //Have thread sleep for 10ms
    std::this_thread::sleep_for (std::chrono::milliseconds(10));

    //Close the file we were writing to -- is this required? -- last check it broke program
    if(termNum == Term1)
        PE(myClose(daSktPrT1M[TermSkt]));
    else 
        PE(myClose(daSktPrMT2[TermSkt]));
	

    return;
    }


////////////////////////////////////////////////////////////////
//
// Startup threads and begin program
//
////////////////////////////////////////////////////////////////
int Ensc351Part2()
    {

	PE_0(pthread_setname_np(pthread_self(), "P-T1"));       // give the primary thread (terminal 1) a name


	//Create 2 socket pairs. Between sender and medium and medium and recieved
	PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPrT1M));
	PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPrMT2));

	// This looks like it would be used to open the socket but I don't know if we need to do that
    //      if we do -- change the name it gets assigned to
	//daSktPr[Term1] =  PE(/*myO*/open("/dev/ser2", O_RDWR));


    //Set the thread priority to 70
    posixThread term2Thrd(SCHED_FIFO, 70, termFunc, Term2);

    
    //Create the medium thread to act as middle man between reciever and sender
    posixThread mediumThrd(SCHED_FIFO, 40, mediumFunc, daSktPrT1M[MediumSkt], daSktPrMT2[MediumSkt], "xmodemSenderData.dat" );
                // Set to prirotity 40
                // Begin Thread in mediumFunc -- void mediumFunc(int T1d, int T2d, const char *fname)
                // Pass Agrguements:
                //      T1d is descriptor of the socket to Term 1
                //      T2d is descriptor of the socket to Term 2
                //      fname is file name of the log file to be used


    //Send the main thread to termFunc as terminal 1
	termFunc(Term1);

    //Wait for terminal 2 thread to rejoin before exitting
    term2Thrd.join();

    //Wait for medium thread to rejoin before exiting
    mediumThrd.join();

	return EXIT_SUCCESS;
    }
