// Name        : PeerX.cpp
// ENSC 351 November 2020  Prepared by:
//      - Craig Scratchley, Simon Fraser University

// Version     : November
// Copyright   : Copyright 2020, Craig Scratchley
// Description :  ENSC 351 Project 
//============================================================================

//#include <fstream>
#include <sys/time.h>
#include <string.h>
#include "PeerX.h"
#include "VNPE.h"
#include "Linemax.h"
#include "myIO.h"
#include "AtomicCOUT.h"

using namespace std;

void checksum(uint8_t* sumP, blkT blkBuf)
{
	*sumP = blkBuf[DATA_POS];
	for( int ii=DATA_POS + 1; ii<PAST_CHUNK; ii++ )
		*sumP += blkBuf[ii];
}

PeerX::
PeerX(int d, const char *fname, char left, char right, const char *smLogN, int conInD, int conOutD, bool useCrc)
:result("ResultNotSet"),
 errCnt(0),
 Crcflg(useCrc),
 KbCan(false),
 mediumD(d),
 fileName(fname),
 transferringFileD(-1), 
 logLeft(left),
 logRight(right),
 smLogName(smLogN),
 consoleInId(conInD),
 consoleOutId(conOutD),
 reportInfo(false),
 absoluteTimeout(0),
 holdTimeout(0)
{
	struct timeval tvNow;
	PE(gettimeofday(&tvNow, NULL));
	sec_start = tvNow.tv_sec;
}

//Send a byte to the remote peer across the medium
void
PeerX::
sendByte(uint8_t byte)
{
	if (reportInfo) {
	//*** remove all but last of this block ***
		/*
		if (byte == NAK)
			COUT << "(N)" << flush;
		else if (byte == ACK)
			COUT << "(A)" << flush;
		else if (byte == EOT)
			COUT << "[E]" << flush;
		else */
			COUT << logLeft << byte << logRight << flush;
	}
	PE_NOT(myWrite(mediumD, &byte, sizeof(byte)), sizeof(byte));
}

// get rid of any characters that may have arrived from the medium.
void
PeerX::
dumpGlitches()
{
	const int dumpBufSz = 20;
	char buf[dumpBufSz];
	int bytesRead;
	while (dumpBufSz == (bytesRead = PE(myReadcond(mediumD, buf, dumpBufSz, 0, 0, 0))));
}

void
PeerX::
transferCommon(std::shared_ptr<StateMgr> mySM, bool reportInfoParam)
{
	reportInfo = reportInfoParam;
	/*
	// use this code to send stateChart logging information to a file.
	ofstream smLogFile; // need '#include <fstream>' above
	smLogFile.open(smLogName, ios::binary|ios::trunc);
	if(!smLogFile.is_open()) {
		CERR << "Error opening sender state chart log file named: " << smLogName << endl;
		exit(EXIT_FAILURE);
	}
	mySM->setDebugLog(&smLogFile);
	// */

	// comment out the line below if you want to see logging information which will,
	//	by default, go to cout.
	mySM->setDebugLog(nullptr); // this will affect both peers.  Is this okay?

	mySM->start();

	/* ******** You may need to add code here ******** */

	struct timeval tv;
	tv.tv_sec = 0;

	fd_set set;

	int  max_fd = max(mediumD, consoleInId) + 1;


	while(mySM->isRunning()) {

	    u_int32_t relTM_abs = absoluteTimeout - elapsed_usecs();
	    u_int32_t relTM_default = TM_CHAR * uSECS_PER_UNIT;

	    tv.tv_usec = min(relTM_default, relTM_abs);

	    FD_ZERO(&set);
	    FD_SET(consoleInId, &set);
	    FD_SET(mediumD, &set);
	    int readyDes = PE(select(max_fd, &set, NULL, NULL, &tv));


		uint32_t now = elapsed_usecs();
        if (now >= absoluteTimeout && !readyDes) {
            mySM->postEvent(TM);
        }
        else {
            if(FD_ISSET(consoleInId, &set)) {
                char bytesToReceive[128] = {0};       //Max buffer size from keyboard
                int numBytesReceived = PE(myReadcond(consoleInId, &bytesToReceive[0], 128, 1, 0, 0));         // data should be available right away

                if(bytesToReceive[0] == CANC_C[0] && bytesToReceive[1] == CANC_C[1])
                    mySM->postEvent(KB_C);
                //else {
                    //Pass along message to the medium
                    //PE(myWrite(mediumD, &bytesToReceive[0], numBytesReceived));
                //}
            }
            else if (FD_ISSET(mediumD, &set)) {
                //read character from medium
				char byteToReceive;
				PE_NOT(myReadcond(mediumD, &byteToReceive, 1, 1, 0, 0), 1);             // data should be available right away


				//We  should vary the result based on the type of byte recieved
				if (reportInfo)
					COUT << logLeft << 1.0*(absoluteTimeout - now)/MILLION << ":" << (int)(unsigned char) byteToReceive << ":" << byteToReceive << logRight << flush;

				mySM->postEvent(SER, byteToReceive);
			}
		}
	}
//		smLogFile.close();
}

// returns microseconds elapsed since this peer was constructed (within 1 second)
uint32_t PeerX::elapsed_usecs()
{
	struct timeval tvNow;
	PE(gettimeofday(&tvNow, NULL));
	/*_CSTD */ time_t	tv_sec = tvNow.tv_sec;
	return (tv_sec - sec_start) * MILLION + tvNow.tv_usec; // casting needed?
}

/*
set a timeout time at an absolute time timeoutUnits into
the future. That is, determine an absolute time to be used
for the next one or more XMODEM timeouts by adding
timeoutUnits to the elapsed time.
*/
void PeerX::tm(int timeoutUnits)
{
	absoluteTimeout = elapsed_usecs() + timeoutUnits * uSECS_PER_UNIT;
}

/* make the absolute timeout earlier by reductionUnits */
void PeerX::tmRed(int unitsToReduce)
{
	absoluteTimeout -= (unitsToReduce * uSECS_PER_UNIT);
}

/*
Store the current absolute timeout, and create a temporary
absolute timeout timeoutUnits into the future.
*/
void PeerX::tmPush(int timeoutUnits)
{
	holdTimeout = absoluteTimeout;
	absoluteTimeout = elapsed_usecs() + timeoutUnits * uSECS_PER_UNIT;
}

/*
Discard the temporary absolute timeout and revert to the
stored absolute timeout
*/
void PeerX::tmPop()
{
	absoluteTimeout = holdTimeout;
}

