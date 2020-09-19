/*
 * PeerX.h
 *
 *      Author: wcs
 */

#ifndef PEERX_H_
#define PEERX_H_

#include <stdint.h>                                               // for uint8_t

#define CHUNK_SZ	            128
#define SOH_OH  	            1			                              //SOH Byte Overhead
#define BLK_NUM_AND_COMP_OH   2 	                                //Overhead for blkNum and its complement
#define DATA_POS  	          (SOH_OH + BLK_NUM_AND_COMP_OH)	    //Position of data in buffer
#define PAST_CHUNK 	          (DATA_POS + CHUNK_SZ)		            //Position of checksum in buffer

#define CS_OH                 1			                              //Overhead for CheckSum
#define REST_BLK_OH_CS        (BLK_NUM_AND_COMP_OH + CS_OH)	      //Overhead in rest of block
#define REST_BLK_SZ_CS        (CHUNK_SZ + REST_BLK_OH_CS)
#define BLK_SZ_CS  	 	        (SOH_OH + REST_BLK_SZ_CS)

#define CRC_OH                2			                              //Overhead for CRC16
#define REST_BLK_OH_CRC       (BLK_NUM_AND_COMP_OH + CRC_OH)	    //Overhead in rest of block
#define REST_BLK_SZ_CRC       (CHUNK_SZ + REST_BLK_OH_CRC)
#define BLK_SZ_CRC  	        (SOH_OH + REST_BLK_SZ_CRC)

//Define btye locations for the header and data blocks
#define SOH_BYTE              0                                   //SOH byte
#define BLK_NUM_BYTE          1                                   //Block number byte
#define BLK_NUM_COMP_BYTE     2                                   //Complement of block number byte
#define BLK_DATA_START        3                                   //Data starting byte
#define CHK_SUM_START         131                                 //Checksum starting byte (for both normal and crc)

#define SOH 0x01
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18                                        // 24  
#define	CTRL_Z 0x1A                                     // 26

typedef uint8_t blkT[BLK_SZ_CRC];

////////////////////////////////////////////////////////////////
// 
// Will print an error message to the screen with specified parameters
//
////////////////////////////////////////////////////////////////
void ErrorPrinter (const char* functionCall, const char* file, int line, int error);


////////////////////////////////////////////////////////////////
// 
// Will create the CRC checksum to append to the block of data
//
////////////////////////////////////////////////////////////////
void crc16ns (uint16_t* crc16nsP, uint8_t* buf);


////////////////////////////////////////////////////////////////
// 
// Will create an 8 bit checksum to append to the block of data
//
////////////////////////////////////////////////////////////////
void checksum8bit(uint8_t* myChkSum, uint8_t* buf);


class PeerX 
{
public:
  ////////////////////////////////////////////////////////////////
  // 
  // Constructor of PeerX class. Initalizes all variables in class
  //
  ////////////////////////////////////////////////////////////////
	PeerX(int d, const char *fname, bool useCrc=true);
	const char* result;         // result of the file transfer

protected:
	int mediumD;                // descriptor for serial port or delegate
	const char* fileName;
	int transferringFileD;	    // descriptor for file being read from or written to. -- ??? can't be both
	bool Crcflg;                // use CRC if true (or else checksum if false)

	//Send a byte to the remote peer across the medium
	void /*PeerX::*/ sendByte(uint8_t byte);
};

#endif /* PEERX_H_ */
