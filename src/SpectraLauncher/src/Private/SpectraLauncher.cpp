#include <iostream>
#include <ThreadUtils.h>

#include "CoriumMemoryHandler.h"
#include "CoriumUtility.h"
#include "EngineAllocators.h"

int main() {
	Corium::Memory::Internal::init();
	Corium::Memory::Internal::AllocatorRegistry::initRegistry();
	int y = 100;

	const auto closure = Corium::Core::Utils::buildClosure<void(int)>(
		[](const int x) {
		for (int i = 0; i < x; i++) {
			std::cout << "Hello" << "\n";
		}
	});

	closure(y);
}