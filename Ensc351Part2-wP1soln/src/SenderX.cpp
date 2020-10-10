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
// File Name   : SenderX.cpp
// Version     : September 23rd, 2020
// Description : Starting point for ENSC 351 Project Part 2
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

//#include <iostream>
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <fcntl.h>	// for O_RDONLY

#include "myIO.h"
#include "SenderX.h"
#include "VNPE.h"

using namespace std;


////////////////////////////////////////////////////////////////
//
// SenderX class constructor. Uses PeerX construtor through inheritance
//
////////////////////////////////////////////////////////////////
SenderX::SenderX(const char *fname, int d)
:PeerX(d, fname), bytesRd(-1), firstCrcBlk(true), blkNum(1)
    {
    //No further initialization needed
    }


////////////////////////////////////////////////////////////////
//
// get rid of any characters that may have arrived from the medium
//
////////////////////////////////////////////////////////////////
void SenderX::dumpGlitches()
    {
	const int dumpBufSz = 20;
	char buf[dumpBufSz];
	int bytesRead;
	while (dumpBufSz == (bytesRead = PE(myReadcond(mediumD, buf, dumpBufSz, 0, 0, 0))));
    }



////////////////////////////////////////////////////////////////
//
// Send the block, less the block's last byte, to the receiver
// Returns the block's last byte
//
////////////////////////////////////////////////////////////////
uint8_t SenderX::sendMostBlk(blkT blkBuffer)
    {
	const int mostBlockSize = (this->Crcflg ? BLK_SZ_CRC : BLK_SZ_CS) - 1;
	PE_NOT(myWrite(mediumD, blkBuffer, mostBlockSize), mostBlockSize);
	return *(blkBuffer + mostBlockSize);
    }


////////////////////////////////////////////////////////////////
//
// Send the last byte of a block to the receiver
//
////////////////////////////////////////////////////////////////
void SenderX::sendLastByte(uint8_t lastByte)
    {
	PE(myTcdrain(mediumD));         // wait for previous part of block to be completely drained from the descriptor
	dumpGlitches();			        // dump any received glitches

	PE_NOT(myWrite(mediumD, &lastByte, sizeof(lastByte)), sizeof(lastByte));
    }


////////////////////////////////////////////////////////////////
//
// Will generate a block to be sent. Updates class variables to reflect actions
//
// CRAIG COMMENTS:
// tries to generate a block.  Updates the
// variable bytesRd with the number of bytes that were read
// from the input file in order to create the block. Sets
// bytesRd to 0 and does not actually generate a block if the end
// of the input file had been reached when the previously generated block 
// was prepared or if the input file is empty (i.e. has 0 length).
//
////////////////////////////////////////////////////////////////
void SenderX::genBlk(blkT blkBuf)
    {
	//Get the data from the input file and store it in the dataBuf array
	bytesRd = PE(read(transferringFileD, &blkBuf[DATA_POS], CHUNK_SZ ));

    //Add SOH btye 
    blkBuf[SOH_BYTE] = SOH; 

    //Cast block number to uint8_t and add it to block buffer
    blkBuf[BLK_NUM_BYTE] = (uint8_t) blkNum;

    //Compute the complment of the blkNum and add that to the blkBuf
    blkBuf[BLK_NUM_COMP_BYTE] = ~(blkBuf[BLK_NUM_BYTE]);        // '~' is bit flip operator

    if (blkNum == 48 || blkNum == 0)
    {
    	blkNum = blkNum;
    }

    //If we did not read a full 128 bytes, append with zeros 
    for(int i = bytesRd; i < CHUNK_SZ; i++)
        {
        blkBuf[DATA_POS + i] = CTRL_Z;                          //Hex for ctrl Z (^Z)
        }
    

    //Calculate checksum depending on if we are using crc or normal checksum
    if(Crcflg)
        {
        uint16_t myCrc16ns;
        crc16ns(&myCrc16ns, &blkBuf[DATA_POS]);

        //Append the checksum to the blkBuf
        memcpy(&blkBuf[CHK_SUM_START], &myCrc16ns, CRC_CHK_SUM_SIZE);
        }
    else 
        {
        uint8_t myChkSum;
        checksum8bit(&myChkSum, &blkBuf[DATA_POS], bytesRd);    //Defined in PeerX.cpp

        //Append the checksum to the blkBuf
        blkBuf[CHK_SUM_START] = myChkSum;
        }

	
    return;
    }


////////////////////////////////////////////////////////////////
//
// Prepare the first block of the data transfer
//
////////////////////////////////////////////////////////////////
void SenderX::prep1stBlk()
    {
	// We have already determined if we will be using a checksum or CRC
    // We can simply create this block using our regular method for this stage of the project

	genBlk(blkBufs[CURRENT]);
    }


////////////////////////////////////////////////////////////////
//
// refit the 1st block with a checksum
//
////////////////////////////////////////////////////////////////
void SenderX::cs1stBlk()
    {
	//We do not need to implement this at this stage in the project
    }


////////////////////////////////////////////////////////////////
//
// While sending the now current block for the first time
//      prepare the next block if possible.
//
////////////////////////////////////////////////////////////////
void SenderX::sendBlkPrepNext()
    {
    //Save the current block into the saved block position before it gets overwritten
    memcpy(&blkBufs[SAVED], &blkBufs[CURRENT], BLK_SZ_CRC);

	blkNum++;                   // 1st block about to be sent or previous block ACK'd
	uint8_t lastByte = sendMostBlk(blkBufs[CURRENT]);
	genBlk(blkBufs[CURRENT]);   // prepare next block
	sendLastByte(lastByte);
    }


////////////////////////////////////////////////////////////////
//
// Resends the block that had been sent previously to the xmodem receiver
//
////////////////////////////////////////////////////////////////
void SenderX::resendBlk()
    {
    //Send the saved block
    uint8_t lastByte = sendMostBlk(blkBufs[SAVED]);

    //Send the final byte 
    sendLastByte(lastByte);
    }


////////////////////////////////////////////////////////////////
//
// Opens the file specified to send via the xModem protocol
//
////////////////////////////////////////////////////////////////
int SenderX::openFileToTransfer()
    {
    transferringFileD = myOpen(fileName, O_RDONLY);
    return transferringFileD;
    }


////////////////////////////////////////////////////////////////
//
// Send CAN_LEN copies of CAN characters in a row to the XMODEM receiver
//      to inform it of the cancelling of a file transfer
//
////////////////////////////////////////////////////////////////
void SenderX::can8()
    {
	// No need to space in time CAN chars for Part 2.
	// This function will be more complicated in later parts. 
    char buffer[CAN_LEN];
    memset( buffer, CAN, CAN_LEN);
    PE_NOT(myWrite(mediumD, buffer, CAN_LEN), CAN_LEN);
    }


////////////////////////////////////////////////////////////////
//
// Send CAN_LEN copies of CAN characters in a row to the XMODEM receiver
//      to inform it of the cancelling of a file transfer
//
////////////////////////////////////////////////////////////////
void SenderX::sendFile()
    {
    SenderX& ctx = *this;       // needed to work with SmartState-generated code

    // ***** modify the below code according to the protocol *****
    // below is just a starting point.  You can follow a
    //  different structure if you want.

    char byteToReceive;
    PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);   
    
    //Loop until we recieve the correct byte to set the flag
    bool flagNotSet = true;
    int numberOfInvalidMessages = 0; 
    while(flagNotSet)
        {
        //If we recieve 10 consecutive invalid characters
        if(numberOfInvalidMessages >= 10)
            {
            can8();
            ctx.result = "ExcessiveErrors";
            return;
            }

        //Set the Crcflg depening on byte recieved
        if(byteToReceive == 'C')
            {
            ctx.Crcflg = true;          //Set program to CRC mode
            flagNotSet = false;
            }
        else if(byteToReceive == NAK)
            {
            ctx.Crcflg = false;         //Set program to checksum mode
            flagNotSet = false;
            }
        else 
            {
            numberOfInvalidMessages++;
            PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);   
            }
        }
    
    //Open the file to send to the reciever
	if(openFileToTransfer() == -1) 
        {
		can8();
		result = "OpenError";
	    }
	else 
        {
        //Prepare the first block for sending 
		ctx.prep1stBlk();

		while (ctx.bytesRd) 
            {
		    // send the prepared block and prepare the next one
            //      will have the save the current block in buffer until we get a positive acknowledgement
			ctx.sendBlkPrepNext();

			
			// read the byte returned by the recieved to determine next step
			PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);

            int numberOfNAKs = 0; 
            while(byteToReceive != ACK)
                {
                numberOfNAKs++;

                //After we recieve 10 consecutive NAKs we will abort transmission
                if(numberOfNAKs >= 10)
                    {
                    can8();
                    ctx.result = "Fatal loss of connection. Transmission Failed";
                    return;
                    }

                //Resend the previous block
                SenderX::resendBlk();
            
                //Wait for positive acknowledgement of that block -- continue loop until we get ACK
                PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);
                }

            //We have recieved a positive acknowlegdment of the first block
            ctx.firstCrcBlk = false;
		    }

		ctx.sendByte(EOT);                                  // send an EOT character
		PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);      // read the reciever response

        //If we do not recieve an ACK -- try again
        int numberOfNAKs = 0; 
        while(byteToReceive != ACK)
            {
            numberOfNAKs++;

            if(numberOfNAKs >= 10)
                {
                can8();
                ctx.result = "Fatal loss of synchronization or connection during EOT communication. Transmission was unsuccessful";
                return;
                }
            
            ctx.sendByte(EOT);                                  //Send another EOT until it is positivly acknowledged
		    PE_NOT(myRead(mediumD, &byteToReceive, 1), 1);      // Read the response from the reciever
            }
		
        //We successfully recieived a positive acknowledgement to EOT character -- transmission successful
		ctx.result = "Done";

        //Close the file we are transferring 
		PE(myClose(transferringFileD));
	    }

    return;
    }
