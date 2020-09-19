#ifndef SENDER_H
#define SENDER_H

#include <unistd.h>

#include "PeerX.h"

class SenderX : public PeerX
  {
	friend void testSenderX(const char* iFileName, int mediumD);

  public:
    ////////////////////////////////////////////////////////////////
    //
    // SenderX class constructor. Uses PeerX construtor through inheritance
    //
    ////////////////////////////////////////////////////////////////
    SenderX(const char *fname, int d);


    ////////////////////////////////////////////////////////////////
    //
    // Routine to send the files contents to the desired location 
    //    will be to output file for PART 1
    // Will generate blocks and write them to the output file
    //
    ////////////////////////////////////////////////////////////////
    void sendFile();

    ssize_t bytesRd;                      // The number of bytes last read from the input file.
  

  private:
    uint8_t* blkBuf = NULL;               //Block of data to be defined in size in class constructor
    uint8_t  dataBuf[CHUNK_SZ];           //A data block
    uint8_t  blkNum;	                    //Number of the current blocks to be acknowledged

    ////////////////////////////////////////////////////////////////
    //
    // Will generate a block to be sent. Updates class variables to reflect actions
    //
    ////////////////////////////////////////////////////////////////
    void genBlk(uint8_t* dataBytes);
  };

#endif
