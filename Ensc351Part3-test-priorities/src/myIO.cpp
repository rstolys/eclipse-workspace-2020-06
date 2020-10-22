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
// Helpers: Jayden Cole
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
//% * Your group name should be "P3_<userid1>_<userid2>" (eg. P3_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : myIO.cpp
// Version     : September 28, 2020
// Description : Wrapper I/O functions for ENSC-351
// Copyright (c) 2020 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <unistd.h>			// for read/write/close
#include <fcntl.h>			// for open/creat
#include <sys/socket.h> 		// for socketpair
#include <stdarg.h>
#include <termios.h>
#include <map>
#include <mutex>
#include <condition_variable>

#include "SocketReadcond.h"


//Protoype here to allow for function call earlier than declaration
int myReadcond(int des, void * buf, int n, int min, int time, int timeout);

//Define struct to ...
struct socketInfo {
    bool blk_read;
    bool blk_written;
    int  numBytes;
    int  socketPair;
};


//Define a map variable to be used 
std::mutex                      mut;

std::map<int,socketInfo>        socket_map;

std::condition_variable         read_cond_var;
std::condition_variable         write_cond_var;



bool desIsSocket(int des)
{
    std::lock_guard<std::mutex> lk(mut);
    return socket_map.find(des) != socket_map.end();
}


int myOpen(const char *pathname, int flags, ...) //, mode_t mode)
{
    mode_t mode = 0;
    // in theory we should check here whether mode is needed.
    va_list arg;
    va_start (arg, flags);
    mode = va_arg (arg, mode_t);
    va_end (arg);
	return open(pathname, flags, mode);
}

int myCreat(const char *pathname, mode_t mode)
{
	return creat(pathname, mode);
}

int mySocketpair( int domain, int type, int protocol, int des_array[2] )
{
    int returnVal = socketpair(domain, type, protocol, des_array);

    //Add each of the descriptors to the des_map
    std::lock_guard<std::mutex> lk(mut);          //Lock the mutex to allow us to edit global varaibles

    struct socketInfo des0; des0.blk_read = false; des0.blk_written = true; des0.numBytes = 0; des0.socketPair = des_array[1];
    struct socketInfo des1; des1.blk_read = false; des1.blk_written = true; des1.numBytes = 0; des1.socketPair = des_array[0];

    socket_map[des_array[0]] = des0;
    socket_map[des_array[1]] = des1;

    return returnVal;
}

ssize_t myRead( int des, void* buf, size_t nbyte )
{
    ssize_t res;

    if(desIsSocket(des))            //if the descriptor provided exists in our map
    {
        //Cast the call to myReadcond()
        res = myReadcond(des, buf, nbyte, 1, 0, 0);       //Values of 0 ensure there is no timeout
    }
    else
    {
        res = read(des, buf, nbyte );
    }
    
    return res;
}

ssize_t myWrite( int des, const void* buf, size_t nbyte )
{
    //Ensure the global varaible for blk_read is false

    if(desIsSocket(des))        //if this des is a socket
    {
        std::lock_guard<std::mutex> lk(mut);

        //Record write operation info
        socket_map[des].blk_read = false;
        socket_map[des].blk_written = true;
        socket_map[des].numBytes += nbyte;

        write_cond_var.notify_all();
    }

	return write(des, buf, nbyte);
}

int myClose( int des )
{
	int res = close(des);

    //if descriptor was closed proeprly 
    if(res != -1)
    {
        std::lock_guard<std::mutex> lk(mut);            //Lock the mutex to allow us to edit global varaibles
        socket_map.erase(des);                          //remove the descriptor from the socket map
                //This is safe since map will try to remove the des but if it doesnt exist it will simply do nothing
    //We need to make sure we clear any awaiting read commands so that we avoid deadlock

    }
   
    return res;         //Return the original result 
}

int myTcdrain(int des)
{ 
    int rv;         //return value

    if(desIsSocket(des))         //If this is a socket
    {
        //Wait until the block has been read from the buffer
        std::unique_lock<std::mutex> lk(mut);
        read_cond_var.wait(lk, [des]{return socket_map[des].blk_read;});

        rv = 0;
    }
    else 
    {
        rv = tcdrain(des);           //Use standard tcdrain() for file descriptors
    }

	return rv;
}

/* Arguments:
des
    The file descriptor associated with the terminal device that you want to read from.
buf
    A pointer to a buffer into which readcond() can put the data.
n
    The maximum number of bytes to read.
min, time, timeout
    When used in RAW mode, these arguments override the behavior of the MIN and TIME members of the terminal's termios structure. For more information, see...
 *
 *  https://developer.blackberry.com/native/reference/core/com.qnx.doc.neutrino.lib_ref/topic/r/readcond.html
 *
 *  */
int myReadcond(int des, void * buf, int n, int min, int time, int timeout)
{
    int numBytesRead = 0;
    int rv = 0;

    if(desIsSocket(des))         //Check this is a socket just to be sure -- even though this function is only called for sockets
    {

        if((std::lock_guard<std::mutex> (mut), socket_map[socket_map[des].socketPair].numBytes <= n))
        {
            do
            {
                std::unique_lock<std::mutex> lk(mut);
                write_cond_var.wait(lk, [des]{return socket_map[socket_map[des].socketPair].blk_written;});

                int nBytes = socket_map[socket_map[des].socketPair].numBytes;

                //Read the output
                rv = wcsReadcond(des, buf + sizeof(char)*numBytesRead, nBytes, ((nBytes == 0) ? 0 : 1), time, timeout);

                numBytesRead += rv;

                socket_map[socket_map[des].socketPair].blk_read = true;
                socket_map[socket_map[des].socketPair].blk_written = false;
                socket_map[socket_map[des].socketPair].numBytes = 0;
                read_cond_var.notify_all();                                      //Notify our condition varaible to check again
            }
            while(numBytesRead < min);
        }
        else if((std::lock_guard<std::mutex> (mut), socket_map[socket_map[des].socketPair].numBytes > n))
        {
            //Read the output
            rv = wcsReadcond(des, buf, n, min, time, timeout);

            std::lock_guard<std::mutex> lk(mut);
            socket_map[socket_map[des].socketPair].numBytes -= n;
        }
    }

    return rv;
}


