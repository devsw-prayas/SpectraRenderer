#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <CoriumMemory.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <atomic>
#include <cstring>
#include <memory>
#include <thread>

class TestFixture60 final
	: public Hades::Runtime::IFixture<TestFixture60, Hades::Runtime::NullDeviceAdapter> {
private:
	static inline std::atomic<int> s_Result{ 0 };

public:
	explicit TestFixture60(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		s_Result.store(0);
		bool rawUnattached = false;
		std::thread([&rawUnattached]() { rawUnattached = Frame::this_frame::currentFrame() == nullptr; }).join();
		CORIUM_ASSERT(rawUnattached && "A thread Corium never attached has no frame");

		Factory::DefaultThreadFactory factory{};
		const ThreadHandle handle = factory.createAndStart(makeClosure<void()>([]() {
			Frame::FrameHandle* current = Frame::this_frame::currentFrame();
			if (!current) { s_Result.store(2); return; }
			Frame::FrameHandle& native = Frame::this_frame::nativeContext();
			auto* bytes = reinterpret_cast<std::byte*>(current);
			const bool ok = current == std::addressof(native)
				&& reinterpret_cast<uintptr_t>(current) % 64 == 0
				&& static_cast<std::byte*>(current->m_RegBlob) == bytes + sizeof(Frame::FrameHandle);
			std::memset(current->m_RegBlob, 0xAB, Corium::Memory::Internal::FrameRegBlobSize);
			s_Result.store(ok ? 1 : 3);
		}), "FrameAttach");

		CORIUM_ASSERT(NativeThread::joinThread(handle));
		CORIUM_ASSERT(s_Result.load() == 1 && "Corium threads run on a 64-aligned native context with the blob right after it");
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
