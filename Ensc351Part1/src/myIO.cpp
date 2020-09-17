//============================================================================
// Name        : myIO.cpp
// Version     : UNKNOWNN
// Copyright   : Copyright 2020, Craig Scratchley
// Description : Wrapper functions for ENSC-351, Simon Fraser University
//                These functions will be re-implemented later in the course
//============================================================================

#include <unistd.h>			      // for read/write/close
#include <fcntl.h>	          // for open/creat


////////////////////////////////////////////////////////////////
// 
// Will open file specified in pathname 
//
////////////////////////////////////////////////////////////////
int myOpen(const char *pathname, int flags, mode_t mode)
  {
	return open(pathname, flags, mode);
  }


////////////////////////////////////////////////////////////////
// 
// Will create a file pointer to allow for writing to file 
//
////////////////////////////////////////////////////////////////
int myCreat(const char *pathname, mode_t mode)
  {
	return creat(pathname, mode);
  }


////////////////////////////////////////////////////////////////
// 
// Will read the specified number of bytes from the file specified
//
////////////////////////////////////////////////////////////////
ssize_t myRead( int fildes, void* buf, size_t nbyte )
  {
	return read(fildes, buf, nbyte );
  }


////////////////////////////////////////////////////////////////
// 
// Will write the specified number of bytes to the specified file
//
////////////////////////////////////////////////////////////////
ssize_t myWrite( int fildes, const void* buf, size_t nbyte )
  {
	return write(fildes, buf, nbyte );
  }


////////////////////////////////////////////////////////////////
// 
// Will close the specified file 
//
////////////////////////////////////////////////////////////////
int myClose( int fd )
  {
	return close(fd);
  }
