/*
 * Medium.cpp
 *
 *      Author: Craig Scratchley
 *      Copyright(c) 2020 Craig Scratchley
 */

#include <fcntl.h>
#include <unistd.h> // for write()
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "Medium.h"
#include "myIO.h"
#include "VNPE.h"
#include "AtomicCOUT.h"

#include "PeerX.h"

// Uncomment the line below to turn on debugging output from the medium
//#define REPORT_INFO

//#define SEND_EXTRA_ACKS

//This is the kind medium.

using namespace std;

Medium::Medium(int d1, int d2, const char *fname)
:Term1D(d1), Term2D(d2), logFileName(fname)
{
    byteCount = 0;
    //byteCount = 1;
	ACKforwarded = 0;
	ACKreceived = 0;
	sendExtraAck = false;

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	logFileD = PE2(myCreat(logFileName, mode), logFileName);
}

Medium::~Medium() {
}

// this function will return false when it detects that the Term2 (sender) socket has closed.
bool Medium::MsgFromTerm2()
{
	blkT bytesReceived; // ?
	int numOfByteReceived;
	int byteToCorrupt;

	if (!(numOfByteReceived = PE(myRead(Term2D, bytesReceived, CAN_LEN)))) {
		COUT << "Medium thread: TERM2's socket closed, Medium terminating" << endl;
		return false;
	}
    byteCount += numOfByteReceived;

	PE_NOT(myWrite(logFileD, bytesReceived, numOfByteReceived), numOfByteReceived);
	//Forward the bytes to Term1 (usually RECEIVER),
	PE_NOT(myWrite(Term1D, bytesReceived, numOfByteReceived), numOfByteReceived);

    if(bytesReceived[0]!=EOT && bytesReceived[0] !=CAN) { // if (bytesReceived[0]==SOH) {
        if (sendExtraAck) {
    #ifdef REPORT_INFO
                COUT << "{" << "+A" << "}" << flush;
    #endif
            uint8_t buffer = ACK;
            PE_NOT(myWrite(logFileD, &buffer, 1), 1);
            //Write the byte to term2,
            PE_NOT(myWrite(Term2D, &buffer, 1), 1);

            sendExtraAck = false;
        }

        numOfByteReceived = PE(myRead(Term2D, bytesReceived, (BLK_SZ_CRC - numOfByteReceived)));

        byteCount += numOfByteReceived;
        if (byteCount >= 392) {
            byteCount = byteCount - 392;
            byteToCorrupt = numOfByteReceived - byteCount;
            //byteToCorrupt = numOfByteReceived - byteCount - 1;
            if (byteToCorrupt < numOfByteReceived) {
                bytesReceived[byteToCorrupt] = (255 - bytesReceived[byteToCorrupt]);
        #ifdef REPORT_INFO
                COUT << "<" << byteToCorrupt << "x>" << flush;
        #endif
            }
        }

        PE_NOT(myWrite(logFileD, &bytesReceived, numOfByteReceived), numOfByteReceived);
        //Forward the bytes to Term1 (RECEIVER),
        PE_NOT(myWrite(Term1D, &bytesReceived, numOfByteReceived), numOfByteReceived);
    }
	return true;
}

bool Medium::MsgFromTerm1()
{
	uint8_t buffer[CAN_LEN];
	int numOfByte = PE(myRead(Term1D, buffer, CAN_LEN));
	if (numOfByte == 0) {
		COUT << "Medium thread: TERM1's socket closed, Medium terminating" << endl;
		return false;
	}

    /*note that we record the corrupted ACK in the log file so that we can for it*/
	if(buffer[0]==ACK)
	{
		ACKreceived++;

		if((ACKreceived%10)==0)
		{
			ACKreceived = 0;
			buffer[0]=NAK;
#ifdef REPORT_INFO
			COUT << "{" << "AxN" << "}" << flush;
#endif
		}
#ifdef SEND_EXTRA_ACKS
		else/*actually forwarded ACKs*/
		{
			ACKforwarded++;

			if((ACKforwarded%6)==0)/*Note that this extra ACK is not an ACK forwarded from receiver to the sender, so we don't increment ACKforwarded*/
			{
				ACKforwarded = 0;
				sendExtraAck = true;
			}
		}
#endif
	}

	PE_NOT(write(logFileD, buffer, numOfByte), numOfByte);

	//Forward the buffer to term2,
	PE_NOT(myWrite(Term2D, buffer, numOfByte), numOfByte);
	return true;
}

void Medium::run()
{
    do
        //transfer byte from Term1 (receiver)
        MsgFromTerm1();

        //transfer data from Term2 (sender)
    while (MsgFromTerm2());


	PE(myClose(logFileD));
	PE(myClose(Term1D));
	PE(myClose(Term2D));
}


void mediumFunc(int T1d, int T2d, const char *fname)
{
    PE_0(pthread_setname_np(pthread_self(), "M")); // give the thread (medium) a name
    // PE_0(pthread_setname_np("M")); // give the thread (medium) a name // Mac OS X
    Medium medium(T1d, T2d, fname);
    medium.run();
}

