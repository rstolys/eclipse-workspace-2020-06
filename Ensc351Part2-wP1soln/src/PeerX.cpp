//============================================================================
// File Name   : PeerX.cpp (interim version -- to be replaced with Part 1 submission and eventually solution)
// Version     : September 24th, 2020
// Description : Starting point for ENSC 351 Project with Part 1 Solution
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <arpa/inet.h> // for htons() -- not available with MinGW
#include <iostream>

#include "VNPE.h"

#include "PeerX.h"
#include "myIO.h"






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
void crc16ns(uint16_t* crc16nsP, uint8_t* buf, bool bigEndian)
    {
    register int wcj;
    register uint8_t *cp;
    unsigned oldcrc = 0;

	for(wcj = CHUNK_SZ, cp = buf; --wcj >= 0; ) 
    {
    oldcrc = updcrc((0377 & *cp++), oldcrc);       //For each byte, update the crc
	  }

    //Set oldcrc to the correct value of the crc for the block of data
    oldcrc = updcrc(0, updcrc(0, oldcrc));


    //Place bytes in network byte order -- Prof Craig Solution
    *crc16nsP = my_htons((uint16_t) oldcrc);


    //Place bytes in network byte order -- Our solution from part 1
    /*
    if(*(char *) & n == 1)  
        { 
        //Processor is little endian
        *crc16nsP = (((oldcrc & 0x00FF) << 8) | ((oldcrc & 0xFF00) >> 8 ));
        }
    else 
        {
        //Processor is big endian
        *crc16nsP = oldcrc;   
        }
    */

    return;
    }


////////////////////////////////////////////////////////////////
// 
// Place Bytes in Network Byte Order
//
// Code from Craig Scratchley
//
////////////////////////////////////////////////////////////////
uint16_t my_htons(uint16_t n)
    {
    unsigned char *np = (unsigned char *)&n;

    return ((uint16_t)np[0] << 8) | (uint16_t)np[1];
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
      *myChkSum = ((*myChkSum & 0xFF) + (buf[ii] & 0xFF)) & 0xFF;      
      }

    return;
  }


////////////////////////////////////////////////////////////////
// 
// Constructor of PeerX class. Initalizes all variables in class
//
////////////////////////////////////////////////////////////////
PeerX::PeerX(int d, const char *fname, bool useCrc)
:result("ResultNotSet"), errCnt(0), mediumD(d), fileName(fname), transferringFileD(-1), Crcflg(useCrc)   //Set default values
    {
    //No initalization needed
    }


////////////////////////////////////////////////////////////////
// 
// Will send a byte to the to remote peer across the medium
//
////////////////////////////////////////////////////////////////
void PeerX::sendByte(uint8_t byte)
    {
    //Write byte to mediumD
    int retVal = PE_NOT(myWrite(mediumD, &byte, sizeof(byte)));

	switch (retVal) 
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

    fflush(stdout);          // with MinGW the error doesn't show up sometimes.
    exit(EXIT_FAILURE);
    }