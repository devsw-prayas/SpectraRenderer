#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsAllocators.h>
#include <KerbecsCompiler.h>
#include <KerbecsDiagnostics.h>
#include <KerbecsRuntime.h>
#include <Region.h>
#include <ShadowedMemory.h>
#include <ShadowUtils.h>
#include <Violation.h>
#include <cstdint>
#include <memory>
#include <vector>

#if defined(_MSC_VER)
KERBECS_NOINLINE static bool triggerAndCatchUaf(const Kerbecs::ShadowedMemory<int>& handle) noexcept {
	__try {
		int val = *handle;
		(void)val;
		return false;
	}
	__except (1) {
		return true;
	}
}
#endif

class ShadowedMemoryUseAfterFreeTrapsOnAccess final
	: public Hades::Runtime::IFixture<ShadowedMemoryUseAfterFreeTrapsOnAccess, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ShadowedMemoryUseAfterFreeTrapsOnAccess(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
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
		KERBECS_ASSERT(m_Handle.operator int* () != nullptr && "Allocation returned null handle payload");

		m_ConstructSuccess = m_Region->construct(m_Handle, 42);
		KERBECS_ASSERT(m_ConstructSuccess && "Construct failed");
		KERBECS_ASSERT(*m_Handle == 42 && "Dereferenced handle value mismatch");

		m_DestroySuccess = m_Region->destroy(m_Handle);
		KERBECS_ASSERT(m_DestroySuccess && "Destroy failed");

		m_ShadowState = Kerbecs::shadowStateOf(m_Handle, sizeof(int));
		KERBECS_ASSERT(m_ShadowState == Kerbecs::Shadow::Utils::MemoryState::DESTROYED && "Shadow state is not DESTROYED after destroy");

#if defined(_MSC_VER)
		m_Caught = triggerAndCatchUaf(m_Handle);
		KERBECS_ASSERT(m_Caught && "Expected access of destroyed handle to trap via _check()");

		m_Violation = {};
		m_ViolationPushed = Kerbecs::popViolation(m_Violation);
		KERBECS_ASSERT(m_ViolationPushed && "Expected a violation to be pushed on Use-After-Free access");
		KERBECS_ASSERT(m_Violation.m_Kind == Kerbecs::ViolationKind::UseAfterFree && "Expected ViolationKind to be UseAfterFree");
#else
		m_Layout = reinterpret_cast<const ShadowedMemoryLayout*>(&m_Handle);
		KERBECS_ASSERT(m_Layout->metaPtr != nullptr && "MetaPtr should be valid");
		m_Info = static_cast<const Kerbecs::Runtime::AccessInfo*>(m_Layout->metaPtr);
		m_CurrentGen = m_Info->m_Generation.load(std::memory_order_acquire);
		KERBECS_ASSERT(m_CurrentGen == 0 && "Metadata generation should be 0 after destroy");
		KERBECS_ASSERT(m_CurrentGen != m_Layout->generation && "Handle generation should mismatch metadata generation causing _check() UAF trap");
#endif
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
	bool m_ConstructSuccess{ false };
	bool m_DestroySuccess{ false };
	Kerbecs::Shadow::Utils::MemoryState m_ShadowState{};

#if defined(_MSC_VER)
	bool m_Caught{ false };
	Kerbecs::Violation m_Violation{};
	bool m_ViolationPushed{ false };
#else
	struct ShadowedMemoryLayout {
		void* metaPtr;
		void* payloadPtr;
		void* shadowPtr;
		uint64_t generation;
	};
	const ShadowedMemoryLayout* m_Layout{ nullptr };
	const Kerbecs::Runtime::AccessInfo* m_Info{ nullptr };
	uint64_t m_CurrentGen{ 0 };
#endif
};

