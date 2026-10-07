#pragma once
// TODO: not run yet.
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumFrame.h>
#include <CoriumRuntime.h>
#include <CoriumThread.h>

#include <cstdio>

// A closure that simply returns falls into the trampoline, which hands control back and only then marks the frame TERMINATED.
class FrameRunToCompletionTerminatesAfterLanding final
	: public Hades::Runtime::IFixture<FrameRunToCompletionTerminatesAfterLanding, Hades::Runtime::NullDeviceAdapter> {
private:
#define FRAME_DONE_CHECK(c) do { if (!(c)) { std::printf("FRAME CHECK FAILED line %d: %s\n", __LINE__, #c); ++fails; } } while (0)

	static int run() noexcept {
		using namespace Corium::Core;
		using Frame::FrameState;
		using Frame::NativeFrame;
		using Frame::Provenance;

		int fails = 0;
		Frame::FrameHandle* native = std::addressof(Frame::this_thread::nativeContext());

		// The thread's own context is running, and its memory is the thread's, not the frame pool's.
		FRAME_DONE_CHECK(native->m_State == FrameState::RUNNING);
		FRAME_DONE_CHECK(native->m_Origin == Provenance::CALLEE_OWNED);
		FRAME_DONE_CHECK(Frame::this_thread::currentFrame() == native);

		// ---- pool-backed frame
		int poolRuns = 0;
		FrameState stateInside = FrameState::TERMINATED;
		auto poolClosure = makeClosure<void()>([&]() {
			++poolRuns;
			stateInside = Frame::this_thread::currentFrame()->m_State;
		});

		Frame::FrameStackDesc poolDesc;
		Frame::init(poolDesc);
		Frame::enableGuardPages(poolDesc, true);
		FRAME_DONE_CHECK(Frame::validate(poolDesc));

		auto pooled = NativeFrame::createFrame(std::move(poolClosure), poolDesc);
		FRAME_DONE_CHECK(pooled.has_value());
		if (!pooled) return fails + 1;

		FRAME_DONE_CHECK((*pooled)->m_Origin == Provenance::NUMA_GLOBAL);
		FRAME_DONE_CHECK(reinterpret_cast<uintptr_t>(*pooled) % Corium::Memory::PAGE_SIZE == 0);
		FRAME_DONE_CHECK((*pooled)->m_State == FrameState::READY);

		NativeFrame::switchTo(native, *pooled, false);
		FRAME_DONE_CHECK(poolRuns == 1);
		FRAME_DONE_CHECK(stateInside == FrameState::RUNNING);
		// The pool frame's block went back on landing, so its handle is not read here; the caller-owned frame below shows TERMINATED.
		FRAME_DONE_CHECK(Frame::this_thread::currentFrame() == native);
		FRAME_DONE_CHECK(native->m_State == FrameState::RUNNING);

		// ---- caller-owned frame on static storage
		CORIUM_ALIGNAS(64) static std::byte s_Block[Frame::kReservedHeaderSize + Frame::kFrameMinStackSize];

		int ownedRuns = 0;
		auto ownedClosure = makeClosure<void()>([&]() { ++ownedRuns; });

		Frame::FrameStackDesc ownedDesc;
		Frame::init(ownedDesc);
		Frame::setProvenance(ownedDesc, Provenance::CALLEE_OWNED);
		Frame::setMemoryLocation(ownedDesc, s_Block);
		Frame::setStackSize(ownedDesc, sizeof(s_Block));
		FRAME_DONE_CHECK(Frame::validate(ownedDesc));

		auto owned = NativeFrame::createFrame(std::move(ownedClosure), ownedDesc);
		FRAME_DONE_CHECK(owned.has_value());
		if (owned) {
			FRAME_DONE_CHECK(static_cast<void*>(*owned) == static_cast<void*>(s_Block));
			FRAME_DONE_CHECK((*owned)->m_Origin == Provenance::CALLEE_OWNED);
			NativeFrame::switchTo(native, *owned, false);
			FRAME_DONE_CHECK(ownedRuns == 1);
			FRAME_DONE_CHECK((*owned)->m_State == FrameState::TERMINATED);
		}

		// ---- a desc that was never validated is rejected, not guessed at
		Frame::FrameStackDesc unfrozen;
		Frame::init(unfrozen);
		auto idle = makeClosure<void()>([]() {});
		FRAME_DONE_CHECK(!NativeFrame::createFrame(std::move(idle), unfrozen).has_value());
		return fails;
	}
#undef FRAME_DONE_CHECK

public:
	explicit FrameRunToCompletionTerminatesAfterLanding(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		int fails = 0;
		Corium::Core::Factory::DefaultThreadFactory threadFactory;
		Corium::Core::ThreadHandle worker = threadFactory.createAndStart(
			Corium::Core::makeClosure<void()>([&fails]() { fails = run(); }), "FrameDoneWorker");
		Corium::Core::NativeThread::joinThread(worker);
		if (fails != 0) CORIUM_TRAP();
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		CORIUM_UNUSED(ro_Adapter);
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
