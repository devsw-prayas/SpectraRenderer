#pragma once
#include "RHIUtils.h"
#include "SpecRHICompiler.h"

namespace Spectra::RHI {
	class IRHIResource;
	class IRHIBindingLayout;
	class IRHIAccelerationStructure;
	class IRHIFence;

	// Describes a GPU buffer allocation. location controls memory placement; flags control optional features.
	// DeviceAddress flag is required for bindless access and buffer device address usage.
	struct SPEC_RHI_ALIGNAS(8) RHIBufferDesc final {
		uint64_t                 m_Size;
		Utils::RHIMemoryLocation m_Location = Utils::RHIMemoryLocation::Device;
		Utils::RHIMemoryFlags    m_Flags    = Utils::RHIMemoryFlags::None;
	};

	// Describes a typed image resource. renderTarget and depthStencil are mutually exclusive.
	// depth must be 1 for 2D textures; arrayLayers must be 1 unless a texture array is explicitly required.
	struct SPEC_RHI_ALIGNAS(8) RHITextureDesc final {
		uint32_t              m_Width;
		uint32_t              m_Height;
		uint32_t              m_Depth           = 1;
		uint32_t              m_MipLevels       = 1;
		uint32_t              m_ArrayLayers     = 1;
		Utils::RHIFormat      m_Format;
		Utils::RHIMemoryFlags m_Flags           = Utils::RHIMemoryFlags::None;
		bool                  m_RenderTarget    = false;
		bool                  m_DepthStencil    = false;
		bool                  m_UnorderedAccess = false;
	};

	// Describes a resource pipeline barrier using the access-intent model (Stage + Access pairs on both sides).
	// Set srcQueue and dstQueue to trigger a queue family ownership transfer; leave both as None for a regular barrier.
	struct SPEC_RHI_ALIGNAS(8) RHIBarrierDesc final {
		IRHIResource*         m_Resource;
		Utils::RHIAccessInfo  m_SrcAccess;
		Utils::RHIAccessInfo  m_DstAccess;
		Utils::RHIQueueType   m_SrcQueue = Utils::RHIQueueType::None;
		Utils::RHIQueueType   m_DstQueue = Utils::RHIQueueType::None;
	};

	// Describes a single binding slot within a binding layout. stageVisibility is ignored by the CUDA backend.
	// count enables array bindings and defaults to 1 for scalar slots.
	struct RHIBindingLayoutItem final {
		uint32_t              m_Slot;
		Utils::RHIBindingType m_Type;
		Utils::RHIShaderStage m_StageVisibility;
		uint32_t              m_Count = 1;
	};

	// Describes the full set of binding slots for a pipeline, with optional push constant configuration.
	// pushConstantsEnabled must be true for IRHICommandList::SetPushConstants to be valid on a pipeline using this layout.
	struct SPEC_RHI_ALIGNAS(8) RHIBindingLayoutDesc final {
		const RHIBindingLayoutItem* m_Items;
		uint32_t                    m_ItemCount;
		uint32_t                    m_PushConstantSize     = 0;
		bool                        m_PushConstantsEnabled = false;
	};

	// Associates a GPU resource with a numbered binding slot for a single UpdateBindings call.
	// resource must match the RHIBindingType declared in the layout at the given slot position.
	struct SPEC_RHI_ALIGNAS(8) RHIBinding final {
		IRHIResource* m_Resource;
		uint32_t      m_Slot;
	};

	// Carries a single specialization constant for a graphics or compute shader stage.
	// Ignored entirely by the CUDA backend, which has no specialization constant mechanism.
	struct RHISpecConstant final {
		uint32_t m_Id;
		enum class Type : uint8_t { Bool, Int32, Uint32, Float32 } m_Type;
	private:
		uint8_t m_Pad[3] = {};
	public:
		union {
			bool     b;
			int32_t  i;
			uint32_t u;
			float    f;
		} m_Value;
	};

	// Describes a single shader stage: bytecode points to SPIR-V (Vulkan) or PTX/CUBIN (CUDA), size is in bytes.
	// constants and constantCount are silently ignored on the CUDA backend.
	struct SPEC_RHI_ALIGNAS(8) RHIShaderDesc final {
		void*              m_Bytecode      = nullptr;
		size_t             m_Size          = 0;
		const char*        m_EntryPoint    = nullptr;
		IRHIBindingLayout* m_Layout        = nullptr;
		RHISpecConstant*   m_Constants     = nullptr;
		uint32_t           m_ConstantCount = 0;
	};

	// Configures triangle fill and cull mode, front face winding, and optional depth bias for a graphics pipeline.
	// depthBiasConst and depthBiasSlope are in hardware units; set both to 0.f to disable depth bias entirely.
	struct RHIRasterizerDesc final {
		Utils::RHIFillMode m_FillMode       = Utils::RHIFillMode::Solid;
		Utils::RHICullMode m_CullMode       = Utils::RHICullMode::Back;
		bool               m_FrontCCW       = true;
	private:
		uint8_t            m_Pad            = 0;
	public:
		float              m_DepthBiasConst = 0.f;
		float              m_DepthBiasSlope = 0.f;
	};

	// Configures depth test, depth write, and stencil enable for a graphics pipeline.
	// Stencil operations are reserved for a future spec revision; stencilEnable is stored but unimplemented.
	struct RHIDepthStencilDesc final {
		bool                m_DepthTestEnable  = true;
		bool                m_DepthWriteEnable = true;
		Utils::RHICompareOp m_DepthCompareOp   = Utils::RHICompareOp::Less;
		bool                m_StencilEnable    = false;
	};

	// Describes per-render-target blend state. blendEnable = false bypasses all blend factor and op evaluation.
	// Alpha blend uses separate srcBlendAlpha, dstBlendAlpha, and blendOpAlpha fields from color blend.
	struct RHIRenderTargetBlendDesc final {
		bool                  m_BlendEnable    = false;
		Utils::RHIBlendFactor m_SrcBlend       = Utils::RHIBlendFactor::One;
		Utils::RHIBlendFactor m_DstBlend       = Utils::RHIBlendFactor::Zero;
		Utils::RHIBlendOp     m_BlendOp        = Utils::RHIBlendOp::Add;
		Utils::RHIBlendFactor m_SrcBlendAlpha  = Utils::RHIBlendFactor::One;
		Utils::RHIBlendFactor m_DstBlendAlpha  = Utils::RHIBlendFactor::Zero;
		Utils::RHIBlendOp     m_BlendOpAlpha   = Utils::RHIBlendOp::Add;
	};

	// Describes one vertex attribute: which binding it reads from, its format, and byte offset within the stride.
	// location maps to the GLSL attribute location; offset is relative to the start of each vertex record.
	struct RHIVertexAttribute final {
		uint32_t         m_Location;
		uint32_t         m_Binding;
		Utils::RHIFormat m_Format;
		uint32_t         m_Offset;
	};

	// Describes one vertex buffer binding: the stride in bytes between records and the advance rate.
	// Multiple bindings can coexist, each supplying a distinct attribute stream to the same draw call.
	struct RHIVertexBinding final {
		uint32_t                   m_Binding;
		uint32_t                   m_Stride;
		Utils::RHIVertexInputRate  m_InputRate;
	};

	// Describes the attachment format layout for a pipeline without owning attachments.
	// Maps to VkPipelineRenderingCreateInfo with VK_KHR_dynamic_rendering; no legacy VkRenderPass is created.
	struct RHIRenderingLayout final {
		Utils::RHIFormat m_ColorFormats[8];
		uint32_t         m_ColorCount = 0;
		Utils::RHIFormat m_DepthFormat;
		bool             m_HasDepth   = false;
	private:
		uint8_t          m_Pad[3]    = {};
	public:
	};

	// Aggregates all pipeline state required to create a rasterization pipeline. Vulkan only; returns Unsupported on CUDA.
	// vertexAttributes and vertexBindings may be null with count = 0 for pipelines with no vertex input stage.
	struct SPEC_RHI_ALIGNAS(8) RHIGraphicsPipelineDesc final {
		RHIShaderDesc            m_VertexShader;
		RHIShaderDesc            m_FragmentShader;
		RHIRasterizerDesc        m_Rasterizer;
		RHIDepthStencilDesc      m_DepthStencil;
		RHIRenderTargetBlendDesc m_BlendState[8];
		RHIVertexAttribute*      m_VertexAttributes     = nullptr;
		uint32_t                 m_VertexAttributeCount = 0;
		RHIVertexBinding*        m_VertexBindings       = nullptr;
		uint32_t                 m_VertexBindingCount   = 0;
		Utils::RHIPrimitiveTopology m_Topology          = Utils::RHIPrimitiveTopology::TriangleList;
	private:
		uint8_t                  m_Pad[3]               = {};
	public:
		RHIRenderingLayout       m_RenderingLayout;
		IRHIBindingLayout*       m_BindingLayout        = nullptr;
	};

	// Describes a compute or PTX kernel pipeline. Valid on both Vulkan and CUDA backends.
	// On CUDA, shader.bytecode points to a compiled PTX module and entryPoint names the kernel function.
	struct SPEC_RHI_ALIGNAS(8) RHIComputePipelineDesc final {
		RHIShaderDesc      m_Shader;
		IRHIBindingLayout* m_BindingLayout = nullptr;
	};

	// Describes one shader group in a ray tracing pipeline. type determines which index fields are active.
	// Unused index fields must be set to ~0u; the backend ignores them based on the group type.
	struct RHIRTShaderGroup final {
		enum class Type : uint8_t { RayGen, Miss, TrianglesHit, ProceduralHit } m_Type;
	private:
		uint8_t m_Pad[3] = {};
	public:
		uint32_t m_RayGenOrMissIndex = ~0u;
		uint32_t m_ClosestHitIndex   = ~0u;
		uint32_t m_AnyHitIndex       = ~0u;
		uint32_t m_IntersectionIndex = ~0u;
	};

	// Describes a full ray tracing pipeline with all shader stages and hit group definitions.
	// maxRecursionDepth maps to VkRayTracingPipelineCreateInfoKHR::maxPipelineRayRecursionDepth or OptiX trace depth.
	struct SPEC_RHI_ALIGNAS(8) RHIRTPipelineDesc final {
		const RHIShaderDesc*    m_Stages;
		const RHIRTShaderGroup* m_Groups;
		IRHIBindingLayout*      m_BindingLayout;
		uint32_t                m_StageCount;
		uint32_t                m_GroupCount;
		uint32_t                m_MaxRecursionDepth;
	};

	// Describes an acceleration structure allocation. sizeHint must come from GetAccelerationStructureBuildSizes.
	// AllowCompaction and AllowUpdate flags must be set at creation time; they cannot be added after build.
	struct SPEC_RHI_ALIGNAS(8) RHIASDesc final {
		uint64_t               m_SizeHint;
		Utils::RHIASBuildFlags m_Flags = Utils::RHIASBuildFlags::None;
		Utils::RHIASType       m_Type;
	private:
		uint8_t                m_Pad[3] = {};
	public:
	};

	// Describes a ray trace dispatch with launch dimensions and a backend-specific extension block.
	// backendExt carries the SBT descriptor on Vulkan or the OptiX parameter block on CUDA; cast at the call site.
	struct SPEC_RHI_ALIGNAS(8) RHITraceRaysDesc final {
		IRHIAccelerationStructure* m_AS;
		void*                      m_BackendExt;
		uint32_t                   m_Width;
		uint32_t                   m_Height;
		uint32_t                   m_Depth;
	private:
		uint32_t                   m_Pad = 0;
	public:
	};

	// Describes a command list submission with timeline fence wait and signal values.
	// waitFence and signalFence must not be null; passing null is a validation error caught in debug builds.
	struct SPEC_RHI_ALIGNAS(8) RHISubmitDesc final {
		IRHIFence* m_WaitFence;
		uint64_t   m_WaitValue;
		IRHIFence* m_SignalFence;
		uint64_t   m_SignalValue;
	};

	// Carries per-frame synchronization context. frameIndex monotonically increases each frame.
	// For frames with multiple independent queues, pass the fence of the longest-lived queue.
	struct SPEC_RHI_ALIGNAS(8) RHIFrameContext final {
		uint64_t   m_FrameIndex;
		IRHIFence* m_CompletionFence;
	};
}
