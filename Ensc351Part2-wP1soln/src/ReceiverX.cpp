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
// File Name   : ReceiverX.cpp
// Version     : September 24th, 2020
// Description : Starting point for ENSC 351 Project Part 2
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <string.h> // for memset()
#include <fcntl.h>
#include <stdint.h>
//#include <iostream>
#include "myIO.h"
#include "ReceiverX.h"
#include "VNPE.h"

//using namespace std;

ReceiverX::
ReceiverX(int d, const char *fname, bool useCrc)
:PeerX(d, fname, useCrc), 
NCGbyte(useCrc ? 'C' : NAK),
goodBlk(false), 
goodBlk1st(false), 
syncLoss(false), // transfer will end when syncLoss becomes true
numLastGoodBlk(0)
{
}

/* Only called after an SOH character has been received.
The function tries
to receive the remaining characters to form a complete
block.  The member
variable goodBlk1st will be made true if this is the first
time that the block was received in "good" condition.
*/
void ReceiverX::getRestBlk()
{
    // This should function should check the checksum or CRC and the decided if ack or nack
    // Needs to decide if checksum or crc
    // Read from disk to compare to previous blk??

	// ********* this function must be improved ***********
	PE_NOT(myReadcond(mediumD, &rcvBlk[1], REST_BLK_SZ_CRC, REST_BLK_SZ_CRC, 0, 0), REST_BLK_SZ_CRC);
	goodBlk1st = goodBlk = true;
}

//Write chunk (data) in a received block to disk
void ReceiverX::writeChunk()
{
	PE_NOT(write(transferringFileD, &rcvBlk[DATA_POS], CHUNK_SZ), CHUNK_SZ);
}

int
ReceiverX::
openFileForTransfer()
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    transferringFileD = PE2(myCreat(fileName, mode), fileName);
    return transferringFileD;
}

int
ReceiverX::
closeTransferredFile()
{
    return myClose(transferringFileD);
}

//Send 8 CAN characters in a row to the XMODEM sender, to inform it of
//	the cancelling of a file transfer
void ReceiverX::can8()
{
	// no need to space in time CAN chars coming from receiver
    char buffer[CAN_LEN];
    memset( buffer, CAN, CAN_LEN);
    PE_NOT(myWrite(mediumD, buffer, CAN_LEN), CAN_LEN);
}

void ReceiverX::receiveFile()
{
    if(openFileForTransfer() == -1) {
        can8();
        // cout /* cerr */ << "Error opening/creating output file named: " << fileName << endl;
        result = "CreatError";
    }
    else {
        ReceiverX& ctx = *this; // needed to work with SmartState-generated code

        // ***** improve this member function *****

        // below is just an example template.  You can follow a
        //  different structure if you want.

        // inform sender that the receiver is ready and that the
        //      sender can send the first block
        sendByte(ctx.NCGbyte);

        //run while the blocks from sender give us a SOH
        while(PE_NOT(myRead(mediumD, rcvBlk, 1), 1), (rcvBlk[0] == SOH))
        {
            // needs improvment
            ctx.getRestBlk();

            // getrestblk should check the CRC/chksum and then we can send a nack
            // if it didnt match or ack and write use write chunk to write to disc
            ctx.sendByte(ACK); // assume the expected block was received correctly.
            ctx.writeChunk();
        };
        // assume EOT was just read in the condition for the while loop
        ctx.sendByte(NAK); // NAK the first EOT
        PE_NOT(myRead(mediumD, rcvBlk, 1), 1); // presumably read in another EOT

        // Check if the file closed properly.  If not, result should be "CloseError".
        if (ctx.closeTransferredFile()) {
            ; // ***** fill this in *****
            // send nak ??
        }
        else {
             ctx.sendByte(ACK);  // ACK the second EOT
             ctx.result="Done";
        }
    }
}
