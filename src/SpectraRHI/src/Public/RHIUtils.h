#pragma once
#include "SpectraRHI.h"

namespace Spectra::RHI::Utils {
	enum class RHIBackendType : uint8_t {
		CUDA,
		VULKAN
	};

	enum class RHIMemoryLocation : uint8_t {
		Device,
		Upload,
		Readback
	};

	enum class RHIMemoryFlags : uint8_t {
		None = 0,
		Dedicated = 1 << 0,
		PersistentlyMapped = 1 << 1,
		ExternalShared = 1 << 2,
		DeviceAddress = 1 << 3
	};

	enum class RHIStage : uint8_t {
		None, 
		VertexInput, 
		Graphics, 
		Compute, 
		RayTracing, 
		Transfer, 
		All
	};

	enum class RHIAccess : uint8_t {
		None,
		Read,
		Write, 
		RenderTarget,
		DepthWrite,
		ASBuild
	};

	enum class RHIFillMode : uint8_t {
		Solid, 
		Wireframe
	};

	enum class RHICUllMode : uint8_t {
		None,
		Front,
		Back
	};

	enum class RHICompareOp : uint8_t {
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	enum class RHIBlendFactor : uint8_t {
		Zero,
		One,
		SrcAlpha,
		OneMinusSrcAlpha,
		DestAlpha,
		OneMinusDestAlpha
	};

	enum class RHIBlendOp : uint8_t {
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class RHIVertexInputRate : uint8_t {
		PerVertex,
		PerInstance
	};

	enum class RHIPrimitiveTopology : uint8_t {
		
	};
}
