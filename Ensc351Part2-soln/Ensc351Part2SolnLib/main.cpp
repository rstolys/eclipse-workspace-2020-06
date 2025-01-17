#include "posixThread.hpp"

using namespace std;

int Ensc351Part2(); // should we include a header file?

int main()
{
    // Pre-allocate some memory for the process.
    // Seems to make priorities work better, at least
    // when using gdb.
    // For some programs, the amount of memory allocated
    // here should perhaps be greater.
    void* ptr = malloc(5000);
    free(ptr);

    try {       // ... realtime policy.
        // program is started off at realtime priority 99
        pthreadSupport::setSchedPrio(60); // drop priority down somewhat.  FIFO?

        return Ensc351Part2();
    }
    catch (system_error& error) {
        cout << "Error: " << error.code() << " - " << error.what() << '\n';
        cout << "Have you launched process with a realtime scheduling policy?" << endl;
        return error.code().value();
    }
    catch (...) { throw; }
}

