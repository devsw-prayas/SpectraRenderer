#include "ThreadUtils.h"
#include <iostream>
#include <intrin.h>

using namespace Spectra::Platform;

int main() {
	Runtime::Thread::ThreadLaunchExecDesc desc;
	Runtime::Thread::initLaunchExecDesc(desc);

	std::cout << desc.m_CommitSize;
	
}