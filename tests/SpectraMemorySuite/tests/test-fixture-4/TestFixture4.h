#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/MemorySuiteBoot.h"
#include <CoriumDiagnostics.h>
#include <cstdint>

class TestFixture4 final
	: public Hades::Runtime::IFixture<TestFixture4, Hades::Runtime::NullDeviceAdapter> {
private:
	static constexpr size_t kMiB = size_t{ 1 } << 20;

	static bool isCommitted(const Spectra::Memory::MemoryHandle& ro_Handle, size_t v_Offset) noexcept {
		Spectra::Memory::MemoryQueryDesc desc{};
		desc.m_TargetAddress = static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + v_Offset;
		return Spectra::Memory::Memory::query(desc).m_State == Spectra::Memory::MemoryState::COMMIT;
	}

	static Spectra::Memory::MemoryHandle reserveScratch(size_t v_Size) noexcept {
		using namespace Spectra::Memory;
		VirtualMemoryDesc desc{};
		initMemoryDesc(desc);
		setSize(desc, v_Size);
		setMemoryState(desc, MemoryState::RESERVE);
		return Memory::reserve(desc);
	}

public:
	explicit TestFixture4(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		CORIUM_ASSERT(MemorySuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Memory;
		using namespace Spectra::Memory::Allocators;

		MemoryHandle plainHandle = reserveScratch(16 * kMiB);
		LinearArena plain;
		plain.init(&plainHandle);
		CORIUM_ASSERT(plain.allocate(0) == nullptr && "Zero size");
		CORIUM_ASSERT(plain.allocate(64, 3) == nullptr && "Non-power-of-two alignment");
		CORIUM_ASSERT(plain.allocate(32 * kMiB) == nullptr && "Larger than the region");
		CORIUM_ASSERT(plain.allocate(SIZE_MAX) == nullptr && "Overflowing size");
		void* aligned = plain.allocate(100, 4096);
		CORIUM_ASSERT(aligned && (reinterpret_cast<uintptr_t>(aligned) & 4095) == 0);
		CORIUM_ASSERT(plain.allocate(16 * kMiB - 100) != nullptr && plain.allocate(1) == nullptr && "The arena fills exactly");

		MemoryHandle linearHandle = reserveScratch(64 * kMiB);
		ReclaimableLinearArena linear;
		linear.init(&linearHandle);
		auto* big = static_cast<uint8_t*>(linear.allocate(10 * kMiB));
		big[10 * kMiB - 1] = 0xAB;
		CORIUM_ASSERT(isCommitted(linearHandle, 10 * kMiB - 1));
		linear.reset();
		CORIUM_ASSERT(!isCommitted(linearHandle, 0) && !isCommitted(linearHandle, 10 * kMiB - 1) && "reset() must decommit");
		auto* again = static_cast<uint8_t*>(linear.allocate(kMiB));
		again[0] = 1;
		CORIUM_ASSERT(again == big && isCommitted(linearHandle, 0) && "The arena must recommit after reset()");

		MemoryHandle stackHandle = reserveScratch(64 * kMiB);
		ReclaimableStackArena stack;
		stack.init(&stackHandle);
		CORIUM_UNUSED(stack.allocate(kMiB));
		const auto marker = stack.mark();
		auto* spike = static_cast<uint8_t*>(stack.allocate(20 * kMiB));
		spike[20 * kMiB - 1] = 1;
		stack.unwind(marker);
		CORIUM_ASSERT(isCommitted(stackHandle, 0) && "The chunk holding the marker stays committed");
		CORIUM_ASSERT(!isCommitted(stackHandle, 4 * kMiB) && !isCommitted(stackHandle, 21 * kMiB - 1) && "unwind() must decommit past the marker");
		CORIUM_UNUSED(stack.allocate(8 * kMiB));
		stack.trim(0);
		CORIUM_ASSERT(isCommitted(stackHandle, 9 * kMiB - 1) && "trim() must keep live allocations committed");
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
