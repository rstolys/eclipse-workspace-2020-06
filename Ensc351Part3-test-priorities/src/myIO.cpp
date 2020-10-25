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

#include <unistd.h>         // for read/write/close
#include <fcntl.h>          // for open/creat
#include <sys/socket.h>         // for socketpair
#include <stdarg.h>
#include <termios.h>
#include <map>
#include <mutex>
#include <condition_variable>

#include "SocketReadcond.h"


//Protoype here to allow for function call earlier than declaration
int myReadcond(int des, void * buf, int n, int min, int time, int timeout);

//Define struct to hold socket information about our sockets
struct socketInfo {
    bool blk_read;
    bool blk_written;
    bool socketPairOpen;
    int  numBytes;
    int  socketPair;
};


//Define a map variable to be used
std::mutex                      mut;

std::map<int,socketInfo>        socket_map;

std::condition_variable         read_cond_var;
std::condition_variable         write_cond_var;
std::condition_variable         cond_var;


////////////////////////////////////////////////////////
/// Returns boolean if descriptor specified is in socket_map and partner is open
////////////////////////////////////////////////////////
bool desIsOpenSocket(int des)
    {
    std::lock_guard<std::mutex> lk(mut);
    return (socket_map.find(des) != socket_map.end()) && (socket_map[des].socketPairOpen);
    }

////////////////////////////////////////////////////////
/// Returns boolean if descriptor specified is in socket_map
////////////////////////////////////////////////////////
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


int mySocketpair(int domain, int type, int protocol, int des_array[2])
    {
    int returnVal = socketpair(domain, type, protocol, des_array);

    //Add each of the descriptors to the des_map
    std::lock_guard<std::mutex> lk(mut);          //Lock the mutex to allow us to edit global varaibles

    //                       {blk_read, blk_written, socketPairOpen, numBytes, socketpair}
    struct socketInfo des0 = {false, false, true, 0, des_array[1]};
    struct socketInfo des1 = {false, false, true, 0, des_array[0]};

    socket_map[des_array[0]] = des0;
    socket_map[des_array[1]] = des1;

    return returnVal;
    }


ssize_t myRead(int des, void* buf, size_t nbyte)
    {
    ssize_t res = -1;       //Assume error

    if(desIsSocket(des))            //if the descriptor provided exists in our map
        {
        //Cast the call to myReadcond()
        res = myReadcond(des, buf, nbyte, 1, 0, 0);       //Values of 0 ensure there is no timeout
        }
    else
        {
        res = read(des, buf, nbyte);
        }

    return res;
    }


ssize_t myWrite( int des, const void* buf, size_t nbyte )
    {
    ssize_t rv;         //return value

    if(desIsOpenSocket(des))        //if this des is a socket
        {
        std::lock_guard<std::mutex> lk(mut);

        //Record write operation info
        socket_map[des].blk_read = false;
        socket_map[des].blk_written = true;
        socket_map[des].numBytes += nbyte;

        rv = write(des, buf, nbyte);

        cond_var.notify_all();
        }
    else
        {
        rv = write(des, buf, nbyte);
        }

    return rv;
    }


int myClose(int des)
    {
    if(desIsOpenSocket(des))
        {
        std::lock_guard<std::mutex> lk(mut);        //Lock mutex

        //if the other socket is still open make sure it knows its pair was closed
        if(socket_map[des].socketPairOpen)
            {
            socket_map[socket_map[des].socketPair].socketPairOpen = false;
            cond_var.notify_all();            //Notify condition varaibles to check again if waiting to read from write of this socket
            }

        //Erase our both sockets from mapping
        socket_map.erase(des);
        }

    return close(des);
    }


int myTcdrain(int des)
    {
    int rv;         //return value

    if(desIsOpenSocket(des))         //If this is a socket
        {
        //Wait until the block has been read from the buffer
        std::unique_lock<std::mutex> lk(mut);
        cond_var.wait(lk, [des]{return socket_map[des].blk_read || !socket_map[des].socketPairOpen;});

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
    int rv = -1;            //Assume error to start

    if(!desIsOpenSocket(des))
        {
        rv = wcsReadcond(des, buf, n, min, time, timeout);
        }
    else if(min  == 0)      //if the call wants a timeout right away
        {
        //Read the output
        rv = wcsReadcond(des, buf, n, min, time, timeout);

        std::lock_guard<std::mutex> lk(mut);
        socket_map[socket_map[des].socketPair].numBytes -= rv;
        }
    else if((std::lock_guard<std::mutex> (mut), socket_map[socket_map[des].socketPair].numBytes <= n))
        {
        do {
            std::unique_lock<std::mutex> lk(mut);

            // wait until we have a block written or our socket pair is closed
            cond_var.wait(lk, [des]{return socket_map[socket_map[des].socketPair].blk_written || !socket_map[des].socketPairOpen;});

            //if the socket pair got closed while we were waiting we need to return from the read function
            if(socket_map[des].socketPairOpen)
                {
                int nBytes = socket_map[socket_map[des].socketPair].numBytes;

                //Read the output
                numBytesRead += wcsReadcond(des, ((char*) buf) + numBytesRead, n, 1, time, timeout);

                rv = numBytesRead;             //Increment return value by the number of bytes read

                socket_map[socket_map[des].socketPair].blk_read = true;
                socket_map[socket_map[des].socketPair].blk_written = false;
                socket_map[socket_map[des].socketPair].numBytes -= numBytesRead;
                cond_var.notify_all();                                     //Notify our tcdrain() cond_varaible to check again
                }
            } while(numBytesRead < min && (std::lock_guard<std::mutex> (mut), socket_map[des].socketPairOpen));
        }
    else if((std::lock_guard<std::mutex> (mut), socket_map[socket_map[des].socketPair].numBytes > n))
        {
        //Read the output
        rv = wcsReadcond(des, buf, n, min, time, timeout);

        std::lock_guard<std::mutex> lk(mut);
        socket_map[socket_map[des].socketPair].numBytes -= n;
        }

    return rv;
    }

