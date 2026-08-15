#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <Violation.h>

class ViolationStackPopReturnsFalseWhenEmpty final
	: public Hades::Runtime::IFixture<ViolationStackPopReturnsFalseWhenEmpty, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ViolationStackPopReturnsFalseWhenEmpty(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Out = Kerbecs::Violation{};
		m_Out.m_Kind = Kerbecs::ViolationKind::DoubleFree;
		m_Out.m_Address = reinterpret_cast<void*>(0xDEADBEEF);
		m_Out.m_BlockBase = reinterpret_cast<void*>(0xCAFEBABE);
		m_Out.m_BlockSize = 1024;
		m_Out.m_Timestamp = 12345;
		m_Out.m_ThreadID = 99;
		m_Popped = false;
	}

	void executeImpl() noexcept {
		m_Popped = Kerbecs::popViolation(m_Out);

		KERBECS_ASSERT(!m_Popped);
		KERBECS_ASSERT(m_Out.m_Kind == Kerbecs::ViolationKind::DoubleFree);
		KERBECS_ASSERT(m_Out.m_Address == reinterpret_cast<void*>(0xDEADBEEF));
		KERBECS_ASSERT(m_Out.m_BlockBase == reinterpret_cast<void*>(0xCAFEBABE));
		KERBECS_ASSERT(m_Out.m_BlockSize == 1024);
		KERBECS_ASSERT(m_Out.m_Timestamp == 12345);
		KERBECS_ASSERT(m_Out.m_ThreadID == 99);
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
	Kerbecs::Violation m_Out{};
	bool m_Popped{ false };
};

