#pragma once
#include <functional>
#include "SpectraCore.h"
#include "PlatformHandles.h"
namespace spectra::core::concurrent {
	class SPECTRA_CORE CPUThreadFactory {
	public:
		virtual ~CPUThreadFactory() = default;
		virtual handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& options, std::function<void()>, bool useJThread) = 0;

		CPUThreadFactory(const CPUThreadFactory&) = delete;
		CPUThreadFactory& operator=(const CPUThreadFactory&) = delete;
		CPUThreadFactory(CPUThreadFactory&&) = default;
		CPUThreadFactory& operator=(CPUThreadFactory&&) = default;
	protected:
		CPUThreadFactory() = default;
	};

	class SPECTRA_CORE DefaultCPUThreadFactory : public CPUThreadFactory {
	public:
		concurrent::handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class SPECTRA_CORE HighPriorityCPUThreadFactory : public CPUThreadFactory {
	public:
		concurrent::handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class SPECTRA_CORE IOCPUThreadFactory : public CPUThreadFactory {
	public:
		concurrent::handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class SPECTRA_CORE DebugCPUThreadFactory : public CPUThreadFactory {
	public:
		concurrent::handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};
}
