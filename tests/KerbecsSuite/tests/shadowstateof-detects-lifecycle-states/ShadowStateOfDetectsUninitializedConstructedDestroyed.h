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

class ShadowStateOfDetectsUninitializedConstructedDestroyed final
	: public Hades::Runtime::IFixture<ShadowStateOfDetectsUninitializedConstructedDestroyed, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ShadowStateOfDetectsUninitializedConstructedDestroyed(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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

		m_Handle = m_Region->allocate<int>();
		KERBECS_ASSERT(static_cast<bool>(m_Handle) && "Allocation returned null handle payload");

		m_UninitShadowState = Kerbecs::shadowStateOf(m_Handle, sizeof(int));
		KERBECS_ASSERT(m_UninitShadowState == Kerbecs::Shadow::Utils::MemoryState::UNINITIALIZED && "Shadow state is not UNINITIALIZED after allocate");

		m_ConstructSuccess = m_Region->construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");

		m_ConstructedShadowState = Kerbecs::shadowStateOf(m_Handle, sizeof(int));
		KERBECS_ASSERT(m_ConstructedShadowState == Kerbecs::Shadow::Utils::MemoryState::CONSTRUCTED && "Shadow state is not CONSTRUCTED after construct");

		m_DestroySuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");

		m_DestroyedShadowState = Kerbecs::shadowStateOf(m_Handle, sizeof(int));
		KERBECS_ASSERT(m_DestroyedShadowState == Kerbecs::Shadow::Utils::MemoryState::DESTROYED && "Shadow state is not DESTROYED after destroy");
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
	Kerbecs::Shadow::Utils::MemoryState m_UninitShadowState{};
	bool m_ConstructSuccess{ false };
	Kerbecs::Shadow::Utils::MemoryState m_ConstructedShadowState{};
	bool m_DestroySuccess{ false };
	Kerbecs::Shadow::Utils::MemoryState m_DestroyedShadowState{};
};


