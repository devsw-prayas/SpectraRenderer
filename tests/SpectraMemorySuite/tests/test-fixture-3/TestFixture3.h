#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/MemorySuiteBoot.h"
#include <CoriumDiagnostics.h>

class TestFixture3 final
	: public Hades::Runtime::IFixture<TestFixture3, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture3(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		CORIUM_ASSERT(MemorySuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Memory::Heap;
		SyncHeap& heap = HeapRegistry::forNode(0);
		HeapRegistry::trim();
		const size_t before = heap.poolSize();

		void* small = heap.allocate(256);
		void* big = heap.allocate(size_t{ 96 } << 20);
		CORIUM_ASSERT(small && big);
		const size_t grown = heap.poolSize();
		heap.deallocate(big);
		HeapRegistry::trim();
		const size_t trimmed = heap.poolSize();
		CORIUM_ASSERT(heap.checkHeap());

		void* regrow = heap.allocate(size_t{ 32 } << 20);
		CORIUM_ASSERT(regrow && heap.checkHeap() && "The pool must grow again after a trim");
		heap.deallocate(regrow);
		heap.deallocate(small);
		HeapRegistry::trim();

		CORIUM_ASSERT(grown > before && "A 96 MiB allocation must grow the pool");
		CORIUM_ASSERT(trimmed < grown && trimmed <= before + kGrowChunk && "trim must give the transient growth back");
		CORIUM_ASSERT(heap.checkHeap());
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
