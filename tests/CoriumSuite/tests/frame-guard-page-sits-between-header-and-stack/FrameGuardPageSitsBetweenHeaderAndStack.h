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

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// With guards on, a pool block is: page 0 header + register blob, page 1 no-access, stack above. With guards off, page 1 is stack.
// The protection query is Win32-only; elsewhere the fixture only checks the placement rules.
class FrameGuardPageSitsBetweenHeaderAndStack final
	: public Hades::Runtime::IFixture<FrameGuardPageSitsBetweenHeaderAndStack, Hades::Runtime::NullDeviceAdapter> {
private:
#define FRAME_GUARD_CHECK(c) do { if (!(c)) { std::printf("FRAME CHECK FAILED line %d: %s\n", __LINE__, #c); ++fails; } } while (0)

#if defined(_WIN32)
	static DWORD protectionOf(const void* p_Addr) noexcept {
		MEMORY_BASIC_INFORMATION info{};
		VirtualQuery(p_Addr, &info, sizeof(info));
		return info.Protect;
	}
#endif

	static int run() noexcept {
		using namespace Corium::Core;
		using Frame::NativeFrame;
		constexpr size_t page = Corium::Memory::PAGE_SIZE;

		int fails = 0;
		auto idle = makeClosure<void()>([]() {});

		Frame::FrameStackDesc guarded;
		Frame::init(guarded);
		Frame::enableGuardPages(guarded, true);
		FRAME_GUARD_CHECK(Frame::validate(guarded));

		auto withGuard = NativeFrame::createFrame(std::move(idle), guarded);
		FRAME_GUARD_CHECK(withGuard.has_value());
		if (withGuard) {
			const auto* base = reinterpret_cast<const uint8_t*>(*withGuard);
			FRAME_GUARD_CHECK(reinterpret_cast<uintptr_t>(base) % page == 0);
			FRAME_GUARD_CHECK((*withGuard)->m_StackSize == Frame::kFrameStackSize);
#if defined(_WIN32)
			FRAME_GUARD_CHECK(protectionOf(base) == PAGE_READWRITE);               // header + register blob
			FRAME_GUARD_CHECK(protectionOf(base + page) == PAGE_NOACCESS);         // the guard
			FRAME_GUARD_CHECK(protectionOf(base + 2 * page) == PAGE_READWRITE);    // stack
			FRAME_GUARD_CHECK(protectionOf(base + Frame::kFrameStackSize - 1) == PAGE_READWRITE);
#endif
		}

		Frame::FrameStackDesc open;
		Frame::init(open);
		FRAME_GUARD_CHECK(Frame::validate(open));

		auto withoutGuard = NativeFrame::createFrame(makeClosure<void()>([]() {}), open);
		FRAME_GUARD_CHECK(withoutGuard.has_value());
#if defined(_WIN32)
		if (withoutGuard) FRAME_GUARD_CHECK(protectionOf(reinterpret_cast<const uint8_t*>(*withoutGuard) + page) == PAGE_READWRITE);
#endif

		// Caller-owned memory cannot be guarded: there is no VA headroom to carve one from.
		Frame::FrameStackDesc owned;
		Frame::init(owned);
		Frame::setProvenance(owned, Frame::Provenance::CALLEE_OWNED);
		Frame::setMemoryLocation(owned, withGuard ? static_cast<void*>(*withGuard) : nullptr);
		Frame::setStackSize(owned, Frame::kFrameStackSize);
		Frame::enableGuardPages(owned, true);
		FRAME_GUARD_CHECK(!Frame::validate(owned));
		return fails;
	}
#undef FRAME_GUARD_CHECK

public:
	explicit FrameGuardPageSitsBetweenHeaderAndStack(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		int fails = 0;
		Corium::Core::Factory::DefaultThreadFactory threadFactory;
		Corium::Core::ThreadHandle worker = threadFactory.createAndStart(
			Corium::Core::makeClosure<void()>([&fails]() { fails = run(); }), "FrameGuardWorker");
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
