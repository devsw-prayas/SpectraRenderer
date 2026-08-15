#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsDiagnostics.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <cstdint>
#include <memory>
#include <vector>

class ShadowedMemoryOperatorPlusInBoundsSucceeds final
	: public Hades::Runtime::IFixture<ShadowedMemoryOperatorPlusInBoundsSucceeds, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ShadowedMemoryOperatorPlusInBoundsSucceeds(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_BackingBuffer.assign(m_RegionSize, 0);
		m_Allocator.init(m_BackingBuffer.data(), m_RegionSize);
		m_Region = std::make_unique<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>>(
			m_Allocator,
			m_RegionSize,
			m_RegistryCapacity,
			m_BackingBuffer.data()
		);
	}

	void executeImpl() noexcept {
		KERBECS_ASSERT(m_Region->initialized() && "Region initialization failed");

		m_Handle = m_Region->allocate<int>(8);
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		for (int i = 0; i < 8; ++i) {
			m_ElementHandle = m_Handle + i;
			KERBECS_ASSERT(static_cast<bool>(m_ElementHandle) && "handle + i returned null handle for in-bounds offset");
			m_ConstructSuccess = m_Region->construct(m_ElementHandle, 100 + i);
			KERBECS_ASSERT(m_ConstructSuccess && "Construct failed for element");
		}

		for (int i = 0; i < 8; ++i) {
			m_ElementHandle = m_Handle + i;
			KERBECS_ASSERT(static_cast<bool>(m_ElementHandle) && "handle + i returned null handle on access check");
			KERBECS_ASSERT(*m_ElementHandle == 100 + i && "Dereferenced handle value does not match constructed value");
		}

		m_OutOfBoundsHandle = m_Handle + 8;
		KERBECS_ASSERT(!static_cast<bool>(m_OutOfBoundsHandle) && "handle + 8 did not return null handle for out-of-bounds offset");

		m_DestroySuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}

private:
	static constexpr size_t m_RegionSize{ 4096 };
	static constexpr size_t m_RegistryCapacity{ 64 };

	std::vector<uint8_t> m_BackingBuffer;
	Kerbecs::Allocators::StaticAllocator m_Allocator;
	std::unique_ptr<Kerbecs::NormalRegion<Kerbecs::Allocators::StaticAllocator>> m_Region;

	Kerbecs::ShadowedMemory<int> m_Handle{};
	Kerbecs::ShadowedMemory<int> m_ElementHandle{};
	bool m_ConstructSuccess{ false };
	Kerbecs::ShadowedMemory<int> m_OutOfBoundsHandle{};
	bool m_DestroySuccess{ false };
};
