#pragma once
#include <Fixture.h>
#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>
#include <KerbecsDiagnostics.h>
#include <Violation.h>
#include <thread>
#include <atomic>

class ViolationStackIsPerThread final
	: public Hades::Runtime::IFixture<ViolationStackIsPerThread, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ViolationStackIsPerThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_ThreadAPushed.store(false, std::memory_order_relaxed);
		m_ThreadBTested.store(false, std::memory_order_relaxed);
		m_ThreadBPopped = true;
		m_OutB = Kerbecs::Violation{};
		m_OutA = Kerbecs::Violation{};
		m_ThreadAPopped = false;
	}

	void executeImpl() noexcept {
		std::thread threadA([&]() {
			Kerbecs::Violation v = Kerbecs::makeViolation(
				Kerbecs::ViolationKind::DoubleFree,
				reinterpret_cast<void*>(0x1234),
				reinterpret_cast<void*>(0x1000),
				64
			);
			Kerbecs::Internal::pushViolation(v);
			m_ThreadAPushed.store(true, std::memory_order_release);

			while (!m_ThreadBTested.load(std::memory_order_acquire)) {
				std::this_thread::yield();
			}
		});

		std::thread threadB([&]() {
			while (!m_ThreadAPushed.load(std::memory_order_acquire)) {
				std::this_thread::yield();
			}

			m_ThreadBPopped = Kerbecs::popViolation(m_OutB);
			m_ThreadBTested.store(true, std::memory_order_release);
		});

		threadA.join();
		threadB.join();

		// Thread B should not have popped the violation pushed by Thread A
		KERBECS_ASSERT(!m_ThreadBPopped);

		// Thread A should be able to pop its own violation
		m_ThreadAPopped = Kerbecs::popViolation(m_OutA);
		KERBECS_ASSERT(m_ThreadAPopped);
		KERBECS_ASSERT(m_OutA.m_Kind == Kerbecs::ViolationKind::DoubleFree);
		KERBECS_ASSERT(m_OutA.m_Address == reinterpret_cast<void*>(0x1234));
		KERBECS_ASSERT(m_OutA.m_BlockBase == reinterpret_cast<void*>(0x1000));
		KERBECS_ASSERT(m_OutA.m_BlockSize == 64);
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
	std::atomic<bool> m_ThreadAPushed{ false };
	std::atomic<bool> m_ThreadBTested{ false };
	bool m_ThreadBPopped{ true };
	Kerbecs::Violation m_OutB{};
	Kerbecs::Violation m_OutA{};
	bool m_ThreadAPopped{ false };
};

