#ifdef WIN32

// Signals currently not supported on Windows
void installSignalHandlers()
{
}

#else

#include "logfile.h"
#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;

#define NUMSIG 256

typedef void (*SignalHandler)(int);

SignalHandler oldSigHandlers[NUMSIG];

void terminationHandler(int sig)
{
	cerr << endl;
	cerr << "UNEXPECTED TERMINATION OF PROGRAM!" << endl;
	cerr.flush();

	LogFile::writeToAllLogFiles("UNEXPECTED TERMINATION OF PROGRAM!");

	if (sig < NUMSIG && oldSigHandlers[sig])
		oldSigHandlers[sig](sig);

	exit(-1);
}

void installSignalHandlers()
{
	for (int i = 0 ; i < NUMSIG ; i++)
		oldSigHandlers[i] = 0;

	int sigs[] = { SIGABRT, SIGQUIT, SIGSEGV, SIGTERM, -1 };
	int pos = 0;
	while (sigs[pos] >= 0)
	{
		int sigNum = sigs[pos];
		oldSigHandlers[sigNum] = signal(sigNum, terminationHandler);
		pos++;
	}
}

#endif // WIN32
