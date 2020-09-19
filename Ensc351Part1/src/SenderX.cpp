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
// TODO
//
////////////////////////////////////////////////////////////////
SenderX::SenderX(const char *fname, int d):PeerX(d, fname), bytesRd(-1), blkNum(1)  
  {
  // ********* initialize blkNum as you like ***********
  //TODO: Determine the size of the block based on crcFlag 
  //try this: -- blkBuf = new uint8_t[((Crcflg) ? BLK_SZ_CRC : BLK_SZ_CS)];
  // *** but first block sent will be block #1, not #0
  }


////////////////////////////////////////////////////////////////
//
// Will generate a block to be sent. Updates class variables to reflect actions
//
// TODO -- add checksum and header bytes to blockBuffer to be sent
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
	// ********* The next line needs to be changed ***********
	if (-1 == (bytesRd = myRead(transferringFileD, &blkBuf[0], CHUNK_SZ )))
    {
    ErrorPrinter("myRead(transferringFileD, &blkBuf[0], CHUNK_SZ )", __FILE__, __LINE__, errno);
    }
	else 
    {
    // ********* and additional code must be written ***********
    //If the bytesRd is less than 128 and greater than 0 then we want to pad the data until it is 128 bytes 
      // pad with 0s

    // ********* The next couple lines need to be changed ***********
    if(Crcflg)
      {
      uint16_t myCrc16ns;
      crc16ns(&myCrc16ns, &blkBuf[0]);
      }
    else 
      {
      uint8_t myCS;
      //createMyChecksum(&myCS, &blkBuf[0]);
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
// TODO
//
////////////////////////////////////////////////////////////////
void SenderX::sendFile()
  {
	transferringFileD = myOpen(fileName, O_RDWR, 0);

	if(transferringFileD == -1) 
    {
		// ********* fill in some code here to write 8 CAN characters ***********
		// can8();
		cout /* cerr */ << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	  }
	else 
    {
		cout << "Sender will send " << fileName << endl;

		// do the protocol, and simulate a receiver that positively acknowledges every
		//	block that it receives.


      // assume 'C' or NAK received from receiver to enable sending with CRC or checksum, respectively

		//TODO: look at best algortihm for sending data to reciever 
    do
      {
      //Generate block of data to be sent
		  genBlk(blkBuf); // prepare 1st block

      //if there is data to send
      if(bytesRd > 0)   
        {
        //Send  the block of data to the reciever  
        myWrite(mediumD, blkBuf, bytesRd);


        //Increment the block number
        blkNum++;
        }
      }
		while(bytesRd);
		/*{   *****Probably don't need this, commented out for now*********

			// ********* fill in some code here to write a block ***********
    
		  //blkNum++; // 1st block about to be sent or previous block was ACK'd
                // This should only increment after the block has been sent
			// assume sent block will be ACK'd
			//genBlk(blkBuf); // prepare next block
			// assume sent block was ACK'd
		  };
    */
		// finish up the protocol, assuming the receiver behaves normally
		// ********* fill in some code here ***********

		//(myClose(transferringFileD));
		if (-1 == myClose(transferringFileD))
      {
      ErrorPrinter("myClose(transferringFileD)", __FILE__, __LINE__, errno);  
      }
			
		result = "Done";
	  }
  }

