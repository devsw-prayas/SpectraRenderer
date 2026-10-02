#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumRuntime.h>
#include <FrameUtils.h>

class FrameStackDescValidateEnforcesProvenanceRules final
	: public Hades::Runtime::IFixture<FrameStackDescValidateEnforcesProvenanceRules, Hades::Runtime::NullDeviceAdapter> {
private:
	using Desc = Corium::Core::Frame::FrameStackDesc;
	using Provenance = Corium::Core::Frame::Provenance;

	alignas(64) static inline unsigned char s_CalleeBlock[1024 * 1024];

	static Desc calleeDesc(void* p_Memory, size_t v_Size) {
		Desc desc;
		Corium::Core::Frame::init(desc);
		Corium::Core::Frame::setProvenance(desc, Provenance::CALLEE_OWNED);
		desc.m_Memory = p_Memory;
		desc.m_Size = v_Size;
		return desc;
	}

public:
	explicit FrameStackDescValidateEnforcesProvenanceRules(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		namespace F = Corium::Core::Frame;

		// Fields are poked directly where a setter would debug-assert, so validate itself is what's under test.
		Desc pooled;
		F::init(pooled);
		CORIUM_ASSERT(F::validate(pooled));
		CORIUM_ASSERT(pooled.m_State == Corium::Core::DescriptorState::FROZEN);
		CORIUM_ASSERT(!F::validate(pooled));

		F::setStackSize(pooled, F::kFrameMaxStackSize);
		CORIUM_ASSERT(pooled.m_Size == F::kFrameStackSize);

		Desc wrongSize;
		F::init(wrongSize);
		wrongSize.m_Size = F::kFrameMaxStackSize;
		CORIUM_ASSERT(!F::validate(wrongSize));

		Desc pooledWithMemory;
		F::init(pooledWithMemory);
		pooledWithMemory.m_Memory = s_CalleeBlock;
		CORIUM_ASSERT(!F::validate(pooledWithMemory));

		Desc badNode;
		F::init(badNode);
		badNode.m_NumaNode = CORIUM_MAX_NUMA;
		CORIUM_ASSERT(!F::validate(badNode));

		Desc guarded;
		F::init(guarded);
		F::enableGuardPages(guarded, true);
		CORIUM_ASSERT(F::validate(guarded));

		Desc callee = calleeDesc(s_CalleeBlock, sizeof(s_CalleeBlock));
		CORIUM_ASSERT(F::validate(callee));

		Desc calleeNull = calleeDesc(nullptr, sizeof(s_CalleeBlock));
		CORIUM_ASSERT(!F::validate(calleeNull));

		Desc calleeMisaligned = calleeDesc(s_CalleeBlock + 16, sizeof(s_CalleeBlock) - 64);
		CORIUM_ASSERT(!F::validate(calleeMisaligned));

		Desc calleeTooSmall = calleeDesc(s_CalleeBlock, F::kReservedHeaderSize + F::kFrameMinStackSize - 16);
		CORIUM_ASSERT(!F::validate(calleeTooSmall));

		Desc calleeMinimum = calleeDesc(s_CalleeBlock, F::kReservedHeaderSize + F::kFrameMinStackSize);
		CORIUM_ASSERT(F::validate(calleeMinimum));

		Desc calleeTooBig = calleeDesc(s_CalleeBlock, F::kFrameMaxStackSize + 16);
		CORIUM_ASSERT(!F::validate(calleeTooBig));

		Desc calleeOddSize = calleeDesc(s_CalleeBlock, sizeof(s_CalleeBlock) - 8);
		CORIUM_ASSERT(!F::validate(calleeOddSize));

		Desc calleeGuarded = calleeDesc(s_CalleeBlock, sizeof(s_CalleeBlock));
		F::enableGuardPages(calleeGuarded, true);
		CORIUM_ASSERT(!F::validate(calleeGuarded));
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
