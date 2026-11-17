#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformMemory.h>
#include <cstring>

class TestFixture3 final
	: public Hades::Runtime::IFixture<TestFixture3, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture3(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		using VM = Memory::PlatformVirtualMemory;
		constexpr size_t kSize = 8u << 20;

		Memory::VirtualMemoryDesc reserveDesc{};
		Memory::initMemoryDesc(reserveDesc);
		Memory::setSize(reserveDesc, kSize);
		Memory::setMemoryState(reserveDesc, Memory::MemoryState::RESERVE);
		Memory::VirtualMemoryHandle handle = VM::reserve(reserveDesc);
		PLATFORM_CHECK(Memory::isValid(handle) && handle.m_TotalSize >= kSize);

		auto state = [&](size_t v_Offset) {
			Memory::MemoryQueryDesc query{};
			query.m_TargetAddress = static_cast<uint8_t*>(handle.m_BaseAddress) + v_Offset;
			return VM::query(query).m_State;
		};
		PLATFORM_CHECK(state(0) == Memory::MemoryState::RESERVE);

		Memory::VirtualMemoryDesc commitDesc{};
		Memory::initMemoryDesc(commitDesc);
		Memory::setTargetAddress(commitDesc, handle.m_BaseAddress);
		Memory::setSize(commitDesc, kSize / 2);
		Memory::setMemoryState(commitDesc, Memory::MemoryState::COMMIT);
		Memory::setProtection(commitDesc, Memory::MemoryProtect::READ_WRITE);
		VM::commit(handle, commitDesc);
		PLATFORM_CHECK(state(0) == Memory::MemoryState::COMMIT && state(kSize / 2 - 1) == Memory::MemoryState::COMMIT);
		PLATFORM_CHECK(state(kSize / 2) == Memory::MemoryState::RESERVE && "Only the requested range is committed");

		std::memset(handle.m_BaseAddress, 0x5A, kSize / 2);
		PLATFORM_CHECK(static_cast<uint8_t*>(handle.m_BaseAddress)[kSize / 2 - 1] == 0x5A);

		Memory::setMemoryState(commitDesc, Memory::MemoryState::DECOMMIT);
		VM::decommit(handle, commitDesc);
		PLATFORM_CHECK(state(0) == Memory::MemoryState::RESERVE && "Decommitted pages go back to reserved");

		Memory::setMemoryState(reserveDesc, Memory::MemoryState::RELEASE);
		VM::release(handle, reserveDesc);
		PLATFORM_CHECK(!Memory::isValid(handle) && "release() invalidates the handle");
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
