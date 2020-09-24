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
// Helpers: We recieved assistance from Jayden Cole
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
//% * Your group name should be "P1_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : SenderX.cpp
// Version     : September 3rd, 2020
// Description : Starting point for ENSC 351 Project
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <iostream>
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <errno.h>
#include <fcntl.h>	// for O_RDWR

#include "myIO.h"
#include "SenderX.h"

using namespace std;


////////////////////////////////////////////////////////////////
//
// SenderX class constructor. Uses PeerX construtor through inheritance
//
////////////////////////////////////////////////////////////////
SenderX::SenderX(const char *fname, int d):PeerX(d, fname), bytesRd(-1), blkNum(1), oneByte(0) 
  {
  //No additional intialization needed
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
void SenderX::genBlk(uint8_t* dataBytes)
  {
	//Get the data from the input file and store it in the dataBuf array
	if(-1 == (bytesRd = myRead(transferringFileD, &dataBuf[0], CHUNK_SZ)))
    {
    ErrorPrinter("myRead(transferringFileD, &blkBuf[0], CHUNK_SZ )", __FILE__, __LINE__, errno);
    }
	else 
    {
    //Add SOH btye 
    blkBuf[SOH_BYTE] = SOH; 

    //Cast block number to uint8_t and add it to block buffer
    blkBuf[BLK_NUM_BYTE] = (uint8_t) blkNum;

    //Compute the complment of the blkNum and add that to the blkBuf
    blkBuf[BLK_NUM_COMP_BYTE] = ~(blkBuf[BLK_NUM_BYTE]);        // '~' is bit flip operator

  
    //If we did not read a full 128 bytes, append with zeros 
    for(int i = bytesRd; i < CHUNK_SZ; i++)
      {
      dataBuf[i] = 0x1A;                                        //Hex for ctrl Z (^Z)
      } 

    //Add all 128 bytes of data to the blkBuf array
    memcpy(&blkBuf[BLK_DATA_START], dataBuf, CHUNK_SZ);
    

    

    //Calculate checksum depending on if we are using crc or normal checksum
    if(Crcflg)
      {
      uint16_t myCrc16ns;
      crc16ns(&myCrc16ns, &dataBuf[0], bigEndian);

      //Append the checksum to the blkBuf
      memcpy(&blkBuf[CHK_SUM_START], &myCrc16ns, CRC_CHK_SUM_SIZE);
      }
    else 
      {
      uint8_t myChkSum;
      checksum8bit(&myChkSum, &dataBuf[0]);

      //Append the checksum to the blkBuf
      blkBuf[CHK_SUM_START] = myChkSum;
      }
    }
	
  return;
  }


////////////////////////////////////////////////////////////////
//
// Routine to send the files contents to the desired location 
//    will be to output file for PART 1
// Will generate blocks and write them to the output file
//
////////////////////////////////////////////////////////////////
void SenderX::sendFile()
  {
	transferringFileD = myOpen(fileName, O_RDWR, 0);

	if(transferringFileD == -1) 
    {
    //Show that we have experienced an error and are aborting the current transmission
    oneByte = CAN;
    sendByte(oneByte);

    //Log the error
		cout /* cerr */ << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	  }
	else 
    {
		cout << "Sender will send " << fileName << endl;

    //Determine if we are using 8 or 16 bit checksum
    int numBytesInBlock = (Crcflg) ? BLK_SZ_CRC : BLK_SZ_CS;

    //***
    //Assuming startup protocol has already begun and we have determined which checksum type we are using CRC 16 bit vs normal 8 bit
    //***

    //Generate the first block of to be transmitted -- this is done so that the while loop will terminate as soon as the byteRd is 0
    genBlk(blkBuf);

    //Send blocks of data and generate new blocks until we have read entire file
    while(bytesRd)
      {
      //Send each byte of the block
      for(int byteNum = 0; byteNum < numBytesInBlock; byteNum++)
        {
        //Send byte to reciever
        sendByte(blkBuf[byteNum]);
        }

      //***
      //We are assuming this messaged was properly acknowledged 
      //***

      //Increment block number 
      blkNum++;

      //Generate the next block to transmitted
      genBlk(blkBuf);
      };
    
		
    //Now that all of the data blocks have been sent, send a end of transmission byte
    oneByte = EOT;
    sendByte(oneByte);

    //Close file now that we are finished with transmission
		if(-1 == myClose(transferringFileD))
      {
      ErrorPrinter("myClose(transferringFileD)", __FILE__, __LINE__, errno);  
      }
			
		result = "Done";
	  }
  }

