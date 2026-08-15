#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumEnvironment.h>
#include <CoriumMemoryHandler.h>
#include <CoriumRuntime.h>
#include <EngineAllocators.h>
#include <cstdint>

class GeneralAllocatorAllocateDeallocateThroughput final
	: public Hades::Runtime::IFixture<GeneralAllocatorAllocateDeallocateThroughput, Hades::Runtime::NullDeviceAdapter> {
private:
	decltype(&Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0]) m_pAlloc{ nullptr };
	static constexpr uint32_t s_iterations = 100'000;
	uint64_t m_ops{ 0 };
	void* m_ptr{ nullptr };
	uint64_t m_OperationsCount{ 0 };

public:
	explicit GeneralAllocatorAllocateDeallocateThroughput(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_pAlloc = &Corium::Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];
		m_ops = 0;
		m_ptr = nullptr;
		m_OperationsCount = 0;
	}

	void executeImpl() noexcept {
		using namespace Corium::Memory::Internal;

		for (uint32_t i = 0; i < s_iterations; ++i) {
			size_t allocSize = 16 + (i % 64);
			m_ptr = m_pAlloc->allocate(allocSize, 16);
			CORIUM_ASSERT(m_ptr != nullptr);
			m_pAlloc->deallocate(m_ptr, allocSize);
			m_ops += 2;
		}

		m_OperationsCount = m_ops;
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
		m_OperationsCount = 0;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return m_OperationsCount;
	}
};
