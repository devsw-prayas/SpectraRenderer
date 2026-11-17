#include "Bootstrap.h"

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <CoriumSync.h>

using namespace Corium::Core;
using namespace Corium::Core::Factory;
using namespace Corium::Core::Frame;
using namespace Corium::Core::Utils;
using namespace Corium::Runtime::Sync;

namespace {
	bool frameExample() {
		FrameStackDesc desc{};
		init(desc);
		validate(desc);

		int steps = 0;
		auto frame = NativeFrame::createFrame(makeClosure<void()>([&steps]() {
			++steps;
			NativeFrame::yieldToNative(this_frame::currentFrame(), true);
		}, 0), desc);

		DefaultThreadFactory factory{};
		const ThreadHandle handle = factory.createAndStart(makeClosure<void()>([&frame, &steps]() {
			NativeFrame::switchTo(this_frame::currentFrame(), frame.value(), false);
			++steps;
		}, 0), "Bootstrap Frame");

		const bool ok = NativeThread::joinThread(handle) && steps == 2;
		printf("[bootstrap] frame switch %s\n", ok ? "ok" : "FAILED");
		return ok;
	}

	bool latchExample() {
		constexpr uint32_t kWorkers = 4;
		CountDownLatch latch(kWorkers);
		std::atomic<uint32_t> done{ 0 };

		std::thread workers[kWorkers];
		for (auto& worker : workers)
			worker = std::thread([&] { done.fetch_add(1); latch.countDown(); });
		latch.await();
		for (auto& worker : workers) worker.join();

		const bool ok = done.load() == kWorkers;
		printf("[bootstrap] latch %s\n", ok ? "ok" : "FAILED");
		return ok;
	}

	bool exchangerExample() {
		Exchanger<uint32_t> exchanger;
		uint32_t fromThread = 0;
		std::thread partner([&] { fromThread = exchanger.exchange(2u); });
		const uint32_t fromMain = exchanger.exchange(1u);
		partner.join();

		const bool ok = fromMain == 2u && fromThread == 1u;
		printf("[bootstrap] exchanger %s\n", ok ? "ok" : "FAILED");
		return ok;
	}
}

int bootstrapRun() {
	const bool ok = frameExample() && latchExample() && exchangerExample();
	return ok ? 0 : 1;
}
