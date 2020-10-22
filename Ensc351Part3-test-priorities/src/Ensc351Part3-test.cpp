/* Ensc351Part3-test-priorities.cpp -- October -- Copyright 2020 Craig Scratchley */
#include <sys/socket.h>
#include <stdlib.h>				// for exit()
#include <sched.h>
#include "posixThread.hpp"
#include "AtomicCOUT.h"
#include "VNPE.h"
#include "myIO.h"

/* This program can be used to test your changes to myIO.cpp
 *
 * Put this project in the same workspace as your Ensc351Part2SolnLib and Ensc351 library projects,
 * and build it.
 *
 * With 3 created threads for a total of 4 threads, the output that I get is:
 *

Primary Thread was executing at policy 0 and priority 0
Primary Thread now executing at policy (should be 1) 1 and priority (should be 30) 30
RetVal 0 in T41: 0
RetVal 1 in T41: 14 Ba: abcd123456789
RetVal in T42: 0
RetVal in T32: 0
myWrite(daSktPr[1], "Added", 6) -1 errno 32: Broken pipe
RetVal 2 in T41: 4 Ba: xyz
myRead(daSktPr[1], Ba, 200) -1
myRead(daSktPr[1], Ba, 200) -1 errno 104: Connection reset by peer
myClose(daSktPr[1]) 0
myClose(daSktPr[1]) -1 errno 9: Bad file descriptor
myRead(daSktPr[1], Ba, 200) -1 errno 9: Bad file descriptor
myReadcond(daSktPr[1], Ba, 200, 0, 0, 0) -1 errno 9: Bad file descriptor
myWrite(daSktPr[1], Ba, 200) -1 errno 9: Bad file descriptor
myTcdrain(daSktPr[1]) -1 errno 9: Bad file descriptor

 *
 */

using namespace std;
using namespace pthreadSupport;

static int daSktPr[2];	  // Descriptor Array for Socket Pair
cpu_set_t set;
int myCpu=0;

void threadT42Func(void) // starts at priority 70
{
    PE_0(pthread_setname_np(pthread_self(), "T42"));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

	PE_NOT(myWrite(daSktPr[1], "ijkl", 5), 5);
	int RetVal = PE(myTcdrain(daSktPr[1])); // will block until myClose
	cout << "RetVal in T42: " << RetVal << endl << flush;
}

void threadT32Func(void) // starts at priority 60 and drops to priority 40
{
    PE_0(pthread_setname_np(pthread_self(), "T32"));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

    PE_NOT(myWrite(daSktPr[0], "abcd", 4), 4);
	PE(myTcdrain(daSktPr[0])); // will block until 1st myReadcond

	setSchedPrio(40);

	PE_NOT(myWrite(daSktPr[0], "123456789", 10), 10); // don't forget nul termination character

	PE(myWrite(daSktPr[0], "xyz", 4));
	int RetVal = PE(myClose(daSktPr[0]));

	cout << "RetVal in T32: " << RetVal << endl << flush;
}

void threadT41Func(void)
{
    PE_0(pthread_setname_np(pthread_self(), "T41"));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

	PE(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr));
    char    Ba[20];
    int
    RetVal = PE(myReadcond(daSktPr[1], Ba, 200, 0, 0, 0));  // will block until myWrite of 10 characters
    cout << "RetVal 0 in T41: " << RetVal << endl << flush;
	posixThread threadT32(60, threadT32Func);

	posixThread threadT42(70, threadT42Func);

	RetVal = PE(myReadcond(daSktPr[1], Ba, 20, 12, 0, 0));  // will block until myWrite of 10 characters
	cout << "RetVal 1 in T41: " << RetVal << " Ba: " << Ba << endl << flush;

    RetVal = PE(myReadcond(daSktPr[1], Ba, 20, 12, 0, 0)); // will block until myClose

	threadT32.join();
	threadT42.join(); // only needed if you created the thread above.
	cout << "myWrite(daSktPr[1], \"Added\", 6) " << myWrite(daSktPr[1], "Added", 6);
	    cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "RetVal 2 in T41: " << RetVal << " Ba: " << Ba << endl << flush;
	cout << "myRead(daSktPr[1], Ba, 200) " << myRead(daSktPr[1], Ba, 200);
		cout << endl;
	cout << "myRead(daSktPr[1], Ba, 200) " << myRead(daSktPr[1], Ba, 200);
//		cout << endl;
	    cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "myClose(daSktPr[1]) " << myClose(daSktPr[1]);
		cout << endl;
//	    cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "myClose(daSktPr[1]) " << myClose(daSktPr[1]);
//		cout << endl;
	    cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "myRead(daSktPr[1], Ba, 200) " << myRead(daSktPr[1], Ba, 200);
//		cout << endl;
        cout << " errno " << errno << ": " << strerror(errno) << endl;
    cout << "myReadcond(daSktPr[1], Ba, 200, 0, 0, 0) " << myReadcond(daSktPr[1], Ba, 200, 0, 0, 0);
//      cout << endl;
        cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "myWrite(daSktPr[1], Ba, 200) " << myWrite(daSktPr[1], Ba, 200);
//		cout << endl;
        cout << " errno " << errno << ": " << strerror(errno) << endl;
	cout << "myTcdrain(daSktPr[1]) " << myTcdrain(daSktPr[1]);
//		cout << endl;
		cout << " errno " << errno << ": " << strerror(errno) << endl;
}

int main() {
    CPU_SET(myCpu, &set);
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

	sched_param sch;
	sched_param sch2;
	try{
		int policy = -1;
		getSchedParam(&policy, &sch2);
		std::cout << "Primary Thread was executing at policy " << policy << " and priority " <<  sch2.sched_priority << '\n';
		sch.__sched_priority = 30;
		setSchedParam(SCHED_FIFO, sch); //SCHED_FIFO == 1, SCHED_RR == 2
		getSchedParam(&policy, &sch2);
        std::cout << "Primary Thread now executing at policy (should be 1) " << policy << " and priority (should be 30) " <<  sch2.sched_priority << '\n';
	}
	catch (std::system_error& error){
		std::cout << "Error: " << error.code() << " - " << error.what() << '\n';
	}

	posixThread T41(SCHED_FIFO, 50, threadT41Func);
	T41.join();
	return 0;
}


