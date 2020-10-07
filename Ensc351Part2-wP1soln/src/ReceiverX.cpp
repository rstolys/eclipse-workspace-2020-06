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
// File Name   : ReceiverX.cpp
// Version     : September 24th, 2020
// Description : Starting point for ENSC 351 Project Part 2
// Original portions Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <string.h>         // for memset()
#include <fcntl.h>
#include <stdint.h>
//#include <iostream>
#include "myIO.h"
#include "ReceiverX.h"
#include "VNPE.h"


//using namespace std;

ReceiverX::
ReceiverX(int d, const char *fname, bool useCrc)
:PeerX(d, fname, useCrc), NCGbyte(useCrc ? 'C' : NAK), goodBlk(false), 
                          goodBlk1st(false), syncLoss(false), numLastGoodBlk(0)
                                             // transfer will end when syncLoss becomes true
    {
    blockAlreadyRecieved = false;
    }

////////////////////////////////////////////////////////////////
//
// Will get the reminder of the block sent 
//
// CRAIGS COMMENNTS:
// Only called after an SOH character has been received.
// The function tries
// to receive the remaining characters to form a complete
// block.  The member
// variable goodBlk1st will be made true if this is the first
// time that the block was received in "good" condition.
//
////////////////////////////////////////////////////////////////
void ReceiverX::getRestBlk()
    {
    //Execute slightly different read process depening on if CRC checksum or regular checksum being used
    if(this->Crcflg)
        {
        //Define the CRC checksum to be computed
        uint16_t* CRC_tot;

        //Read the final byte from the socket -- I think the address provided should be indexed to the end of the data and 1st CRC byte...
        PE_NOT(myReadcond(mediumD, &rcvBlk[1], REST_BLK_SZ_CRC, REST_BLK_SZ_CRC, 0, 0), REST_BLK_SZ_CRC);

        //Compute the crc checksum
        crc16ns(CRC_tot, &rcvBlk[DATA_POS]);

        //Check if the block number is not what we are expecting 
        if(rcvBlk[BLK_NUM_BYTE] != numLastGoodBlk + 1)
            {
            if(rcvBlk[BLK_NUM_BYTE] > numLastGoodBlk + 1)
                {
                syncLoss = true;                    // we missed a block. This is a fatal loss of syncronization
                goodBlk = false;                    // Indicate a bad block. Will end up aborting transmission
                }
            else if(rcvBlk[BLK_NUM_BYTE] == numLastGoodBlk)
                {
                blockAlreadyRecieved = true;        // Our ACK likely got corrupted. Resend out ACK
                goodBlk = false;                    // Indicate this isn't a good block. We don't want to rewrite it
                }  
            }
        else 
            {
            //Check the CRC to see if it valid
            bool validChkSum = (CRC_tot[0] == rcvBlk[CHK_SUM_START]);
            validChkSum = (CRC_tot[1] == rcvBlk[CHK_SUM_START + 1])
            goodBlk = validChkSum;

            //Set our last good block to the current block
            numLastGoodBlk++;
            }
        }
    else    //We are not using CRC but instead using regular checksum
        {
        //Define the checksum to be computed
        uint8_t* CS_tot;

        //Read the final byte from the socket -- I think the address provided should be indexed to the end of the data
        PE_NOT(myReadcond(mediumD, &rcvBlk[1], REST_BLK_SZ_CS, REST_BLK_SZ_CS, 0, 0), REST_BLK_SZ_CS);

        //compute the checksum 
        checksum8bit(CS_tot, &rcvBlk[DATA_POS], CHUNK_SZ);


        //Check if the block number is not what we are expecting 
        if(rcvBlk[BLK_NUM_BYTE] != numLastGoodBlk + 1)
            {
            if(rcvBlk[BLK_NUM_BYTE] > numLastGoodBlk + 1)
                {
                syncLoss = true;                    // We missed a block. This is a fatal loss of syncronization
                goodBlk = false;                    // Indicate a bad block. Will end up aborting transmission
                }
            else if(rcvBlk[BLK_NUM_BYTE] == numLastGoodBlk)
                {
                blockAlreadyRecieved = true;        // Our ACK likely got corrupted. Resend out ACK
                goodBlk = false;                    // Indicate this isn't a good block. We don't want to rewrite it
                }  
            }
        else 
            {
            //Check the checksum to see if it valid
            goodBlk = (CS_tot == rcvBlk[CHK_SUM_START]) ? true : false;

            //Set our last good block to the current block
            numLastGoodBlk++;
            }
        }

    return;
    }


////////////////////////////////////////////////////////////////
//
// Write chunk (data) in a received block to disk
//
////////////////////////////////////////////////////////////////
void ReceiverX::writeChunk()
    {
	PE_NOT(write(transferringFileD, &rcvBlk[DATA_POS], CHUNK_SZ), CHUNK_SZ);
    }


////////////////////////////////////////////////////////////////
//
// Open the file to write the contents of the transfer to
//
////////////////////////////////////////////////////////////////
int ReceiverX::openFileForTransfer()
    {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    transferringFileD = PE2(myCreat(fileName, mode), fileName);
    return transferringFileD;
    }


////////////////////////////////////////////////////////////////
//
// Close the file at the end of the transmission
//
////////////////////////////////////////////////////////////////
int ReceiverX::closeTransferredFile()
    {
    return myClose(transferringFileD);
    }


////////////////////////////////////////////////////////////////
//
// Send 8 CAN characters in a row to the XMODEM sender, to inform it of
//	the cancelling of a file transfer
//
////////////////////////////////////////////////////////////////
void ReceiverX::can8()
    {
	// no need to space in time CAN chars coming from receiver -- for this version
    char buffer[CAN_LEN];
    memset( buffer, CAN, CAN_LEN);
    PE_NOT(myWrite(mediumD, buffer, CAN_LEN), CAN_LEN);
    }


////////////////////////////////////////////////////////////////
//
// Implement the xModem protocol to revieve the file from the sender
//
////////////////////////////////////////////////////////////////
void ReceiverX::receiveFile()
    {
    uint8_t byteToSend; 

    if(openFileForTransfer() == -1) 
        {
        can8();
        result = "CreatError. Transmission Unsuccessful";
        }
    else 
        {
        ReceiverX& ctx = *this;     // needed to work with SmartState-generated code

        //Send our byte indicating checksum method we want to use
        sendByte(ctx.NCGbyte);


        //
        // Once we recieve the first block (with SOH as first character) we know tranmission startup is successful
        //      If we send CRC bytes and fail to recieve response then we need to try checksum 
        //      If we still fail to recieve any response to checksum then abort the transmission
        //
        int numberOfFailedCRC_Startups = 0; 
        int numberOfFailedCS_Startups = 0; 
        while(PE_NOT(myRead(mediumD, rcvBlk, 1), 1), (rcvBlk[0] != SOH))
            {
            if(ctx.NCGbyte == 'C')
                {
                numberOfFailedCRC_Startups++;
                }
            else 
                {
                numberOfFailedCS_Startups++;
                }
            

            if(numberOfFailedCRC_Startups >= 3)
                {
                //Set the block to a NAK and try to send that to the sender
                ctx.NCGbyte = NAK;      
                //This would occur is our sender is unable to accomodate CRC checksums
                }

            if(numberOfFailedCS_Startups >= 10)
                {
                //We are failing to reach our sender host. Abort the transmission
                can8();
                ctx.result = "Failed to startup transmission with checksum. Tranmission Failed";
                return;
                }
            
            //Send the byte again
            sendByte(ctx.NCGbyte);
            }

        //Set the CRC flag to indicate the checksum method we are using based on sender response
        if(ctx.NCGbyte = 'C';)
            this->Crcflg = true;
        else 
            this->Crcflg = false;


        //
        // The block we recieved did have an SOH in it and we are able to process it to check if it is valid
        //      All future blocks recieved will be processed using the below routine 
        //

        //perform the following actions to process byte and return response to sender
        bool transmissionActive = true;
        do
            {
            if(rcvBlk[0] == SOH)            //if our first byte is SOH, tramission is assumed to continue
                {
                //Reset the flags 
                blockAlreadyRecieved = false;
                goodBlk = false; 
                    //Don't need to reset syncLoss since this will 

                //Get the remainder of the block being sent
                ctx.getRestBlk();
                    //Flags for the validity of the block are set in the getRestBlk function
                    //Those flags are checked below to determine how to proceed

                if(goodBlk == true)
                    {
                    byteToSend = ACK;
                    ctx.sendByte(byteToSend);       // Send positive acknowledgement
                    ctx.writeChunk();               // Write the data to the file
                    }
                else if(blockAlreadyRecieved)       //If we have already recieved this block
                    {
                    byteToSend = ACK; 
                    ctx.sendByte(byteToSend);       // resend our positive acknowledgement          
                    } 
                else if(syncLoss)                   //This is the end of tranmission
                    {
                    can8();
                    ctx.result = "Fatal loss of syncronization. Tramission Failed";
                    return;
                    }
                else    //If we recieved a block with corrupted data
                    {
                    byteToSend = NAK; 
                    ctx.sendByte(byteToSend);       // send a negative acknowledgement
                    }

                //Read the next message
                PE_NOT(myRead(mediumD, rcvBlk, 1), 1);
                }
            else if(rcvBlk[0] == EOT)               //If the sender looks to be ending  the transmission
                {
                byteToSend = NAK;
                ctx.sendByte(byteToSend);           //Send a NAK to ensure that the sender did send a EOT

                //Read the next message
                PE_NOT(myRead(mediumD, rcvBlk, 1), 1);

                if(rcvBlk[0] == EOT)                //if we get a second EOT
                    {
                    byteToSend = ACK;
                    ctx.sendByte(byteToSend);       //Acknowledge that we got this byte and end our transmission

                    transmissionActive = false;    
                    }
                }
            else if(rcvBlk[0] == CAN)               //if the sender seems to be terminating the transmission
                {
                //The sender is terminating the transmission
                byteToSend = NAK;
                ctx.sendByte(byteToSend);           //Send NAK assuming it is an error

                //Read the next message and check if it is a CAN
                PE_NOT(myRead(mediumD, rcvBlk, 1), 1);

                if(rcvBlk[0] == CAN)
                    {
                    //terminate the transmission 
                    transmissionActive = false;
                    }
                }
            else    // We recieved a first byte other than a CAN, SOH or EOT
                {
                byteToSend = NAK;
                ctx.sendByte(byteToSend);           //Send NAK assuming it is an error
                
                //Read the next message
                PE_NOT(myRead(mediumD, rcvBlk, 1), 1);
                }
        
            } while (while(transmissionActive));


        // Check if the file closed properly
        if (ctx.closeTransferredFile()) 
            {
            ctx.result = "Close Error";
            }
        else 
            {
            ctx.result = "Done";
            }
        }
    }
