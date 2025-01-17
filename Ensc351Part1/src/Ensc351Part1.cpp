//============================================================================
// Name        : Ensc351Part1.cpp
// Version     : September 9th
// Copyright   : Copyright 2020, Craig Scratchley
// Description : Starting point for Part 1 of Multipart Project
//============================================================================

//============================================================================
//
//  Add needed variables to senderX class -- done
//  Intialize SenderX Class     -- done
//
//  Complete genBlk function    -- done
//
//  Complete sendFile Function -- done
//
//  Create function to calculate checksum of block
//  Fix/ Complete the CRC checksum function
//
//============================================================================

#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>

#include "SenderX.h"
#include "myIO.h"

using namespace std;


////////////////////////////////////////////////////////////////
// 
// Testing function to call specific member functions to send file using xmodem protocol
//
////////////////////////////////////////////////////////////////
void testSenderX(const char* iFileName, int mediumD)
  {
  //Testing using checksum
  SenderX xSender(iFileName, mediumD, false);
  xSender.Crcflg = false;                   // test sending with checksum

  cout << "test sending with checksum" << endl;
  xSender.sendFile();
  cout << "Sender finished with result: " << xSender.result << endl << endl;


  //Testing using CRC
  SenderX xSender2(iFileName, mediumD, true);
  xSender2.Crcflg = true;                   // test sending with CRC16

  cout << "test sending with CRC" << endl;
  xSender2.sendFile();
  cout << "Sender finished with result: " << xSender2.result << endl << endl;

  }



////////////////////////////////////////////////////////////////
// 
// Starting point for project
//    Will setup multiple tests of file transfer protocol
//
////////////////////////////////////////////////////////////////
int main() 
  {
#ifdef __MINGW32__
  _fmode = _O_BINARY;  // needed for MinGW compiler which runs on MS Windows
#endif
  
  int rc = -1; 

  // for x86_64, output file will be in the Eclipse project.
  // for ppc, output file will be in the home directory:  /home/osboxes
  const char* oFileName = "xmodemSenderData.dat";
  mode_t mode = S_IRUSR | S_IWUSR;                                      // | S_IRGRP | S_IROTH;

  int mediumD = myCreat(oFileName, mode);
  if(mediumD == -1) 
    {
    cout /* cerr */ << "Error opening medium file named: " << oFileName << endl;
    ErrorPrinter("creat(oFileName, mode)", __FILE__, __LINE__, errno);
    rc = -1;
    }
  else 
    {
    testSenderX("/doesNotExist.txt", mediumD);                        // file does not exist
    testSenderX("/home/osboxes/.sudo_as_admin_successful", mediumD);  // empty file
    testSenderX("/home/osboxes/hs_err_pid11506.log", mediumD);          // normal text file

    if (-1 == myClose(mediumD)) 
      {
      ErrorPrinter("close(mediumD)", __FILE__, __LINE__, errno);
      rc = -1;
      }
    else
      {
      rc = 0;
      }
    }

  return rc;
  }
