#pragma once
#include "RHIUtils.h"
#include "SpecRHICompiler.h"

namespace Spectra::RHI {
	class SPEC_RHI_ALIGNAS(8) IRHIObject {
	public:
		virtual void addRef() ABSTRACT;
		virtual void release() ABSTRACT;
		SPEC_RHI_NODISCARD virtual Utils::RHIBackendType getBackendType() const ABSTRACT;

		IRHIObject() = default;
		IRHIObject(const IRHIObject&) = delete;
		IRHIObject(IRHIObject&&) = delete;
		IRHIObject& operator=(const IRHIObject&) noexcept = delete;
		IRHIObject& operator=(IRHIObject&&) noexcept = delete;
		virtual ~IRHIObject() = default;
	};

	struct SPEC_RHI_ALIGNAS(32) RHICapabilities final {
		bool m_SupportsGraphics;
		bool m_SupportsCompute;
		bool m_SupportsRayTracingHW;
		bool m_SupportsRayTracingSW;

		bool m_SupportsBindless;
		bool m_SupportsGPUDirect;
		bool m_SupportsExternalMemory;
	private:
		bool pad_ = false;
	public:
		uint64_t m_MaxBufferSize;
		uint32_t m_MaxRayRecursionDepth;

		RHICapabilities() = default;
		RHICapabilities(const RHICapabilities&) = default;
		RHICapabilities(RHICapabilities&&) noexcept = default;
		RHICapabilities& operator=(const RHICapabilities&) = default;
		RHICapabilities& operator=(RHICapabilities&&) noexcept = default;
	};

	class IRHIDevice : public IRHIObject {
		
	};
}
