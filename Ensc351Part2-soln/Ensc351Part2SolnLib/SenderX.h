#ifndef SENDER_H
#define SENDER_H

#include <unistd.h>
#include <stdint.h> // uint8_t
#include "PeerX.h"

class SenderX : public PeerX
{
public:
	SenderX(const char *fname, int d);
	void prep1stBlk(); // tries to prepare the first block.

	void
	//SenderX::
	cs1stBlk()  /* refit the 1st block with a checksum */
	;

	void sendBlkPrepNext(); // While sending the now current block for the first time, prepare the next block if possible.
    void resendBlk(); // Resends the block that had been sent previously to the xmodem receiver.

    int
    //SenderX::
    openFileToTransfer()
    ;

    int
    //SenderX::
    closeTransferredFile()
    ;

    void can8(); // Send CAN_LEN copies of CAN characters in a row.
    void sendFile();

    ssize_t bytesRd;  // The number of bytes last read from the input file.

	/* true if Checksum has not been requested by receiver and first CRC16 block
	 * has not yet been received correctly
	 */
	bool firstCrcBlk;
	
    bool openErr; // was there an error opening the file to send.

    /* A variable which counts the number of problem responses received. The reception
     *  of an ACK resets the count. */
//  unsigned errCnt;    // found in PeerX.h

private:
	// uint8_t blkBuf[BLK_SZ_CRC];     // a  block
	//uint8_t blkBufs[BLK_SZ_CRC][2];	// Array of two blocks
	blkT blkBufs[2];	// Array of two blocks

	uint8_t blkNum;		// number of current block to be acknowledged

	// Send the block, less the block's last byte, to the receiver
	uint8_t sendMostBlk(blkT blkBuf);
//	uint8_t sendMostBlk(uint8_t blkBuf[BLK_SZ_CRC])
//	;

	// Send the last byte of a block to the receiver
	void
	//SenderX::
	sendLastByte(uint8_t lastByte)
	;

    void genBlk(blkT blkBuf); // tries to generate a block.
//	void genBlk(uint8_t blkBuf[BLK_SZ_CRC]) // get rid of this line.
//	;
};

#endif
