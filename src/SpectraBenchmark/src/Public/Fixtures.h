#pragma once
#include "SpectraBenchmark.h"

namespace Spectra::Benchmark {

	class SPEC_BENCHMARK IFixture {
	public:
		IFixture();
		virtual void startup() noexcept = 0;
		virtual void execute() noexcept = 0;
		virtual void shutdown() noexcept = 0;

		IFixture(const IFixture&) = delete;
		IFixture& operator=(const IFixture&) = delete;
		IFixture(IFixture&&) noexcept = delete;
		IFixture& operator=(IFixture&&) noexcept = delete;
	protected:
		virtual ~IFixture() = default;
	};


	class SPEC_BENCHMARK IContentionFixture : IFixture {
	public:
		virtual void read() noexcept = 0;
		virtual void write() noexcept = 0;

		IContentionFixture(const IContentionFixture&) = delete;
		IContentionFixture& operator=(const IContentionFixture&) = delete;
		IContentionFixture(IContentionFixture&&) noexcept = delete;
		IContentionFixture& operator=(IContentionFixture&&) noexcept = delete;
	protected:
		~IContentionFixture() override = default;
	};

#ifndef FIXTURE
#define FIXTURE(name, isContention) \
    class name final : public std::conditional_t<isContention, IContentionFixture, IFixture>
#endif

}
