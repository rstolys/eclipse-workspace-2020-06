/* Ensc351Part3-test-priorities-improved.cpp -- October -- Copyright 2020 Craig Scratchley */
#include <sys/socket.h>
#include <stdlib.h>				// for exit()
#include <sched.h>
#include "posixThread.hpp"
#include "AtomicCOUT.h"
#include "VNPE.h"
#include "myIO.h"

#define REPORT0(S) cout << threadName << ": " << #S << "; statement will now be started" << endl << flush; \
    S; \
    cout << threadName << ": " << #S << "; statement has now finished " << endl << flush;
#define REPORT1(FC) {cout << threadName << ": " << #FC << " will now be called" << endl << flush; \
    int RV = FC; \
    cout << threadName << ": " << #FC << " result was " << RV; \
    if (RV == -1) cout << " errno " << errno << ": " << strerror(errno); \
    cout << endl << flush;}
#define REPORT2(FC) {cout << threadName << ": " << #FC << " will now be called" << endl << flush; \
    int RV = FC; \
    cout << threadName << ": " << #FC << " result was " << RV; \
    if (RV == -1) cout << " errno " << errno << ": " << strerror(errno); \
    else if (RV > 0) cout << " Ba: " << Ba; \
    cout << endl << flush;}

/* This program can be used to test your changes to myIO.cpp
 *
 * Put this project in the same workspace as your Ensc351Part2SolnLib and Ensc351 library projects,
 * and build it.
 *
 * With 3 created threads for a total of 4 threads, the output that I get is:
 *

[...]
Primary Thread now executing at policy (should be 1) 1 and priority (should be 30) 30
T41: mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr) will now be called
T41: mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr) result was 0
T41: mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr2) will now be called
T41: mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr2) result was 0
T41: myReadcond(daSktPr[1], Ba, 20, 0, 0, 0) will now be called
T41: myReadcond(daSktPr[1], Ba, 20, 0, 0, 0) result was 0
T41: posixThread threadT32(60, threadT32Func); statement will now be started
T32: PE_NOT(myWrite(daSktPr[0], "abcd", 4), 4) will now be called
T32: PE_NOT(myWrite(daSktPr[0], "abcd", 4), 4) result was 4
T32: myTcdrain(daSktPr[0]) will now be called
T41: posixThread threadT32(60, threadT32Func); statement has now finished
T41: posixThread threadT42(70, threadT42Func); statement will now be started
T42: PE_NOT(myWrite(daSktPr[1], "ijkl", 5), 5) will now be called
T42: PE_NOT(myWrite(daSktPr[1], "ijkl", 5), 5) result was 5
T42: myTcdrain(daSktPr[1]) will now be called
T41: posixThread threadT42(70, threadT42Func); statement has now finished
T41: myReadcond(daSktPr[1], Ba, 20, 12, 0, 0) will now be called
T32: myTcdrain(daSktPr[0]) result was 0
T32: PE_NOT(myWrite(daSktPr[0], "123456789", 10), 10) will now be called
T32: PE_NOT(myWrite(daSktPr[0], "123456789", 10), 10) result was 10
T32: setSchedPrio(40) will now be called
T41: myReadcond(daSktPr[1], Ba, 20, 12, 0, 0) result was 14 Ba: abcd123456789
T41: myReadcond(daSktPr[1], Ba, 20, 12, 0, 0) will now be called
T32: setSchedPrio(40) result was 0
T32: setSchedPrio(80) will now be called
T32: setSchedPrio(80) result was 0
T32: PE_NOT(myWrite(daSktPr[0], "xyz", 4), 4) will now be called
T32: PE_NOT(myWrite(daSktPr[0], "xyz", 4), 4) result was 4
T32: myClose(daSktPr[0]) will now be called
T32: myClose(daSktPr[0]) result was 0
T32: setSchedPrio(40) will now be called
T42: myTcdrain(daSktPr[1]) result was 0
T41: myReadcond(daSktPr[1], Ba, 20, 12, 0, 0) result was 4 Ba: xyz
T41: threadT32.join(); statement will now be started
T32: setSchedPrio(40) result was 0
T32: PE_NOT(myWrite(daSktPr2[0], "mno", 4), 4) will now be called
T32: PE_NOT(myWrite(daSktPr2[0], "mno", 4), 4) result was 4
T32: myClose(daSktPr2[0]) will now be called
T32: myClose(daSktPr2[0]) result was 0
T41: threadT32.join(); statement has now finished
T41: threadT42.join(); statement will now be started
T41: threadT42.join(); statement has now finished
T41: myWrite(daSktPr[1], "Added", 6) will now be called
T41: myWrite(daSktPr[1], "Added", 6) result was -1 errno 32: Broken pipe
T41: myRead(daSktPr2[1], Ba, 20) will now be called
T41: myRead(daSktPr2[1], Ba, 20) result was 4 Ba: mno
T41: myRead(daSktPr2[1], Ba, 20) will now be called
T41: myRead(daSktPr2[1], Ba, 20) result was -1 errno 104: Connection reset by peer
T41: myClose(daSktPr2[1]) will now be called
T41: myClose(daSktPr2[1]) result was 0
T41: myClose(daSktPr[1]) will now be called
T41: myClose(daSktPr[1]) result was 0
T41: myClose(daSktPr[1]) will now be called
T41: myClose(daSktPr[1]) result was -1 errno 9: Bad file descriptor
T41: myRead(daSktPr[1], Ba, 20) will now be called
T41: myRead(daSktPr[1], Ba, 20) result was -1 errno 9: Bad file descriptor
T41: myReadcond(daSktPr[1], Ba, 20, 0, 0, 0) will now be called
T41: myReadcond(daSktPr[1], Ba, 20, 0, 0, 0) result was -1 errno 9: Bad file descriptor
T41: myWrite(daSktPr[1], Ba, 20) will now be called
T41: myWrite(daSktPr[1], Ba, 20) result was -1 errno 9: Bad file descriptor
T41: myTcdrain(daSktPr[1]) will now be called
T41: myTcdrain(daSktPr[1]) result was -1 errno 9: Bad file descriptor
 *
 */

using namespace std;
using namespace pthreadSupport;

static int daSktPr[2];    // Descriptor Array for Socket Pair
static int daSktPr2[2];    // Descriptor Array for 2nd Socket Pair
cpu_set_t set;
int myCpu=0;

void threadT42Func(void) // starts at priority 70
{
    const char* threadName = "T42";
    PE_0(pthread_setname_np(pthread_self(), threadName));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

    REPORT1(PE_NOT(myWrite(daSktPr[1], "ijkl", 5), 5));
	REPORT1(myTcdrain(daSktPr[1])); // will block until myClose(daSktPr[0])

    // output happens at this time from the above REPORT1
}

void threadT32Func(void) // starts at priority 60 and drops to priority 40
{
    const char* threadName = "T32";
    PE_0(pthread_setname_np(pthread_self(), threadName));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

    REPORT1(PE_NOT(myWrite(daSktPr[0], "abcd", 4), 4));
    REPORT1(myTcdrain(daSktPr[0])); // will block until 1st myReadcond(..., 12, ...);

    // output happens at this time from the above REPORT1
    REPORT1(PE_NOT(myWrite(daSktPr[0], "123456789", 10), 10)); // don't forget nul termination character
    REPORT1(setSchedPrio(40));

    REPORT1(setSchedPrio(80));
    REPORT1(PE_NOT(myWrite(daSktPr[0], "xyz", 4), 4));
    REPORT1(myClose(daSktPr[0]));
    REPORT1(setSchedPrio(40));

    REPORT1(PE_NOT(myWrite(daSktPr2[0], "mno", 4), 4));
    REPORT1(myClose(daSktPr2[0]));

    // output happens at this time from the above REPORT1
}

void threadT41Func(void)
{
    const char* threadName = "T41";
    PE_0(pthread_setname_np(pthread_self(), threadName));
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

    REPORT1(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr));
    REPORT1(mySocketpair(AF_LOCAL, SOCK_STREAM, 0, daSktPr2));
    char    Ba[20];
    REPORT2(myReadcond(daSktPr[1], Ba, 20, 0, 0, 0));
    REPORT0(posixThread threadT32(60, threadT32Func));

    REPORT0(posixThread threadT42(70, threadT42Func));

    REPORT2(myReadcond(daSktPr[1], Ba, 20, 12, 0, 0));   // will block until myWrite of 10 characters

    // output happens at this time from the above REPORT2
    REPORT2(myReadcond(daSktPr[1], Ba, 20, 12, 0, 0));   // will block until myclose(daSktPr[0])

    // output happens at this time from the above REPORT2
    REPORT0(threadT32.join());

    // output happens at this time from the above REPORT0
    REPORT0(threadT42.join());
	REPORT1(myWrite(daSktPr[1], "Added", 6));
    REPORT2(myRead(daSktPr2[1], Ba, 20));
    REPORT2(myRead(daSktPr2[1], Ba, 20));
	REPORT1(myClose(daSktPr2[1]));
	REPORT1(myClose(daSktPr[1]));
	REPORT1(myClose(daSktPr[1]));
    REPORT2(myRead(daSktPr[1], Ba, 20));
    REPORT2(myReadcond(daSktPr[1], Ba, 20, 0, 0, 0));
    REPORT1(myWrite(daSktPr[1], Ba, 20));
    REPORT1(myTcdrain(daSktPr[1]));
}

int main() {
    CPU_SET(myCpu, &set);
    PE(sched_setaffinity(0, sizeof(set), &set)); // set processor affinity for current thread

    // Pre-allocate some memory for the process.
    // Seems to make priorities work better, at least
    // when using gdb.
    // For some programs, the amount of memory allocated
    // here should perhaps be greater.
    void* ptr = malloc(5000);
    free(ptr);

	sched_param sch;
	try{
		int policy = -1;
		getSchedParam(&policy, &sch);
		if (sch.__sched_priority < 98)
		    std::cout << "**** If you are debugging, debugger is not running at a high priority. ****\n" <<
		                    " **** This could cause problems with debugging.  Consider debugging\n" <<
		                    " **** with the proper debug launch configuration ****" << endl;
        std::cout << "Primary Thread was executing at policy " << policy << " and priority " <<  sch.sched_priority << '\n';
		sch.__sched_priority = 30;
		setSchedParam(SCHED_FIFO, sch); //SCHED_FIFO == 1, SCHED_RR == 2
		getSchedParam(&policy, &sch);
        std::cout << "Primary Thread now executing at policy (should be 1) " << policy << " and priority (should be 30) " <<  sch.sched_priority << '\n';
	}
	catch (std::system_error& error){
		std::cout << "Error: " << error.code() << " - " << error.what() << '\n';
	}

	posixThread T41(SCHED_FIFO, 50, threadT41Func);
	T41.join();
	return 0;
}


