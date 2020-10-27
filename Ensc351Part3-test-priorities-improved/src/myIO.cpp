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
    int  socketPair;
    bool socketPairOpen;

    int  numBytes;
};


//Define a map variable to be used
std::mutex                      mut;

std::map<int,socketInfo>        socket_map;

std::condition_variable         cond_var;


////////////////////////////////////////////////////////
/// Returns boolean if both descriptor specified is in socket_map and partner is open
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

    //                       {socketpair, socketPairOpen, numBytes, }
    struct socketInfo des0 = {des_array[1], true, 0, };
    struct socketInfo des1 = {des_array[0], true, 0};

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

        rv = write(des, buf, nbyte);

        //if there was an error we don't want to change with our numBytes
        if(rv >= 0)
            socket_map[des].numBytes += nbyte;

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

        //Erase our socket from mapping
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
        cond_var.wait(lk, [des]{return socket_map[des].numBytes <= 0 || !socket_map[des].socketPairOpen;});

        rv = 0;
        }
    else
        {
        rv = tcdrain(des);           //Use standard tcdrain() for file descriptors
        }

    return rv;
    }

/*
    Arguments:
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
    int rv = 0;                    //Assume error to start

    if(!desIsOpenSocket(des))       //if the socket were trying to read from is not open
        {
        rv = wcsReadcond(des, buf, n, min, time, timeout);
        }
    //if the call wants a timeout right away or there are too many bytes to read
    else if(min == 0)
        {
        //Read the output
        rv = wcsReadcond(des, buf, n, min, time, timeout);

        //Modify the number of bytes in the buffer
        if(rv >= 0)
            {
            std::lock_guard<std::mutex> lk(mut);
            socket_map[socket_map[des].socketPair].numBytes -= rv;
            }
        }
    else
        {
        do {
            std::unique_lock<std::mutex> lk(mut);

            //If there is something in the buffer to be read or our socket pair gets closed
            cond_var.wait(lk, [des]{return socket_map[socket_map[des].socketPair].numBytes > 0 || !socket_map[des].socketPairOpen;});

            //Read the output
            numBytesRead = wcsReadcond(des, ((char*) buf) + numBytesRead, n, 1, time, timeout);

            rv += numBytesRead;             //Increment return value by the number of bytes read

            //Modify the number of bytes in the socket to match the current state
            if(numBytesRead >= 0)
                socket_map[socket_map[des].socketPair].numBytes -= numBytesRead;

            //Notify our tcdrain() cond_varaible to check again
            cond_var.notify_all();

            } while(rv < min && (std::lock_guard<std::mutex> (mut), socket_map[des].socketPairOpen));
        }

    return rv;
    }

