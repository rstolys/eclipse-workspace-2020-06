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
// Helpers: None at all
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
// File Name   : PeerX.cpp
// Version     : September 3rd, 2020
// Description : Starting point for ENSC 351 Project
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

//#include <arpa/inet.h>      // for htons() -- not available with MinGW
#include <stdio.h>            // for fprintf()
#include <string.h>           // for strerror()
#include <stdlib.h>	          // for exit()
#include <errno.h>
#include <iostream>

#include "PeerX.h"
#include "myIO.h"


////////////////////////////////////////////////////////////////
// 
// Will print an error message to the screen with specified parameters
//
////////////////////////////////////////////////////////////////
void ErrorPrinter (const char* functionCall, const char* file, int line, int error)
  {
	fprintf (stdout/*stderr*/, " \n!!! Error %d (%s) occurred at line %d of file %s\n"
			    "\t resulted from invocation: %s\n"
			    "\t Exiting program!\n",
			    error, strerror(error), line, file, functionCall);

	fflush(stdout);         // with MinGW the error doesn't show up sometimes.
	exit(EXIT_FAILURE);
  }


/*
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology.
 */

////////////////////////////////////////////////////////////////
// 
// Will update the crc for the data provided
//
// The following XMODEM crc routine is taken from "rbsb.c".  Please refer to
//  the source code for these programs (contained in RZSZ.ZOO) for usage.
// As found in Chapter 8 of the document "ymodem.txt".
//   Original 1/13/85 by John Byrns
//
////////////////////////////////////////////////////////////////
unsigned short updcrc(register int c, register unsigned crc)
  {
	register int count;

	for (count=8; --count>=0;) 
    {
		if (crc & 0x8000) 
      {
			crc <<= 1;
			crc += (((c<<=1) & 0400)  !=  0);
			crc ^= 0x1021;
		  }
		else 
      {
			crc <<= 1;
			crc += (((c<<=1) & 0400)  !=  0);
		  }
	  }
	return crc;
  }


////////////////////////////////////////////////////////////////
// 
// Will create the CRC checksum to append to the block of data
//
// CRAIG COMMENTS: 
// Should return via crc16nsP a crc16 in 'network byte order'.
// Derived from code in "rbsb.c" (see above).
// Line comments in function below show lines removed from original code.
//
////////////////////////////////////////////////////////////////
void crc16ns(uint16_t* crc16nsP, uint8_t* buf)
  {
  register int wcj;
  register uint8_t *cp;
  unsigned oldcrc = 0;

	for(wcj = CHUNK_SZ, cp = buf; --wcj >= 0; ) 
    {
    oldcrc = updcrc((0377& *cp++), oldcrc);       //For each byte, update the crc
	  }

  //Set oldcrc to the correct value of the crc for the block of data
  oldcrc = updcrc(0,updcrc(0,oldcrc));
  
  //
  // TODO: put bytes in network byte order -- MSB byte first -- need to know endianness of processor
  //
  *crc16nsP = oldcrc;             // Changed from 0, to the value of CRC16

  return;
  }


////////////////////////////////////////////////////////////////
// 
// Will create an 8 bit checksum to append to the block of data
//
////////////////////////////////////////////////////////////////
void checksum8bit(uint8_t* myChkSum, uint8_t* buf, ssize_t bytesRd)
  {
    //Set as default value for now
    *myChkSum = 0x00;

    // If the last block chksum is calculated we only want to add to the end of the blk
    for (int ii = BLK_DATA_START; ii < bytesRd + BLK_DATA_START; ii++)
      {
      //By using binary and operation we force a binary operation and discard the carry
      *myChkSum = (*myChkSum & 0xFF) + (buf[ii] & 0xFF) & 0xFF;      
      }

    return;
  }


////////////////////////////////////////////////////////////////
// 
// Constructor of PeerX class. Initalizes all variables in class
//
////////////////////////////////////////////////////////////////
PeerX::PeerX(int d, const char *fname, bool useCrc)
:result("ResultNotSet"), mediumD(d), fileName(fname), transferringFileD(-1), Crcflg(useCrc)   //Set default values
  {
  //No Initalization code needed -- all variables initalized above
  }



////////////////////////////////////////////////////////////////
// 
// Will send a byte to the to remote peer across the medium (file)
//
////////////////////////////////////////////////////////////////
void PeerX::sendByte(uint8_t byte)
  {
	switch (int retVal = myWrite(mediumD, &byte, sizeof(byte))) 
    {
		case 1:
      {
      return;
      }
		case -1:
      {
      ErrorPrinter("myWrite(mediumD, &byte, sizeof(byte))", __FILE__, __LINE__, errno);
			break;
      }
		default:
      {
      std::cout /* cerr */ << "Wrong number of bytes written: " << retVal << std::endl;
			exit(EXIT_FAILURE);
      }
	  }

  return;
  }
