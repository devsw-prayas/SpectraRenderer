#pragma once
#include "RHIUtils.h"
#include "SpecRHICompiler.h"

namespace Spectra::RHI {
	class IRHIObject;
	class IRHIResource;
	class IRHIBuffer;
	class IRHITexture;
	class IRHISampler;
	class IRHIBindingLayout;
	class IRHIPipeline;
	class IRHIAccelerationStructure;
	class IRHIShaderBindingTable;
	class IRHIFence;

	// Describes a GPU buffer allocation. location controls memory placement; flags control optional features.
	// DeviceAddress flag is required for bindless access and buffer device address usage.
	struct SPEC_RHI_ALIGNAS(8) RHIBufferDesc final {
		uint64_t                 m_Size;
		Utils::RHIMemoryLocation m_Location  = Utils::RHIMemoryLocation::Device;
		Utils::RHIMemoryFlags    m_Flags     = Utils::RHIMemoryFlags::None;
		const char*              m_DebugName = nullptr;
	};

	// Describes a typed image resource. renderTarget and depthStencil are mutually exclusive.
	// depth must be 1 for 2D textures; arrayLayers must be 1 unless a texture array is explicitly required.
	// externalHandle is a Win32 HANDLE for imported textures (RHIResourceOrigin::Imported); null = normal internal allocation.
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
		bool                  m_Sampled         = false;
		bool                  m_TransferSrc     = false;
		bool                  m_TransferDst     = false;
	private:
		uint16_t              m_Pad             = 0;
	public:
		void*                 m_ExternalHandle  = nullptr;
		const char*           m_DebugName       = nullptr;
	};

	// Describes a resource pipeline barrier using the access-intent model (Stage + Access pairs on both sides).
	// Set srcQueue and dstQueue to trigger a queue family ownership transfer; leave both as None for a regular barrier.
	// Subresource range fields default to the whole resource; a *Count of ~0u means "all remaining".
	struct SPEC_RHI_ALIGNAS(8) RHIBarrierDesc final {
		IRHIResource*         m_Resource;
		Utils::RHIAccessInfo  m_SrcAccess;
		Utils::RHIAccessInfo  m_DstAccess;
		Utils::RHIQueueType   m_SrcQueue        = Utils::RHIQueueType::None;
		Utils::RHIQueueType   m_DstQueue        = Utils::RHIQueueType::None;
		uint32_t              m_BaseMipLevel    = 0;
		uint32_t              m_MipLevelCount   = ~0u;
		uint32_t              m_BaseArrayLayer  = 0;
		uint32_t              m_ArrayLayerCount = ~0u;
	};

	// Describes a single binding slot within a binding layout. stageVisibility is ignored by the CUDA backend.
	// count enables array bindings and defaults to 1 for scalar slots. bindless is ignored on CUDA (flat index array
	// has no concept of bounded/unbounded binding); on Vulkan it maps to PARTIALLY_BOUND_BIT + UPDATE_AFTER_BIND_BIT.
	struct RHIBindingLayoutItem final {
		uint32_t              m_Slot;
		Utils::RHIBindingType m_Type;
		Utils::RHIShaderStage m_StageVisibility;
		uint32_t              m_Count    = 1;
		bool                  m_Bindless = false;
	};

	// Describes the full set of binding slots for a pipeline, with optional push constant configuration.
	// pushConstantsEnabled must be true for IRHICommandList::SetPushConstants to be valid on a pipeline using this layout.
	struct SPEC_RHI_ALIGNAS(8) RHIBindingLayoutDesc final {
		const RHIBindingLayoutItem* m_Items;
		uint32_t                    m_ItemCount;
		uint32_t                    m_PushConstantSize     = 0;
		bool                        m_PushConstantsEnabled = false;
	};

	// Associates a GPU resource with a numbered binding slot for an UpdateBindings call.
	// Buffer offset/range select a sub-range; samplers describe a separate sampler binding.
	struct SPEC_RHI_ALIGNAS(8) RHIBinding final {
		IRHIObject*  m_Resource;
		uint32_t     m_Slot;
	private:
		uint32_t     m_Pad = 0;
	public:
		uint64_t     m_Offset  = 0;
		uint64_t     m_Range   = 0;
		IRHISampler* m_Sampler = nullptr;
	};

	// Describes sampler state: filtering, addressing, anisotropy, border color, and LOD clamp range.
	struct SPEC_RHI_ALIGNAS(8) RHISamplerDesc final {
		Utils::RHIFilterMode  m_MinFilter   = Utils::RHIFilterMode::Linear;
		Utils::RHIFilterMode  m_MagFilter   = Utils::RHIFilterMode::Linear;
		Utils::RHIFilterMode  m_MipFilter   = Utils::RHIFilterMode::Linear;
		Utils::RHIAddressMode m_AddressU    = Utils::RHIAddressMode::Repeat;
		Utils::RHIAddressMode m_AddressV    = Utils::RHIAddressMode::Repeat;
		Utils::RHIAddressMode m_AddressW    = Utils::RHIAddressMode::Repeat;
		Utils::RHIBorderColor m_BorderColor = Utils::RHIBorderColor::TransparentBlack;
	private:
		uint8_t               m_Pad         = 0;
	public:
		float                 m_MaxAnisotropy = 1.0f;
		float                 m_MinLod        = 0.0f;
		float                 m_MaxLod        = 1000.0f;
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

	// Describes one render target attachment for a BeginRendering/EndRendering scope: the texture to render into,
	// and its load/store behavior. clearValue is only consulted when loadOp == RHILoadOp::Clear.
	struct SPEC_RHI_ALIGNAS(8) RHIRenderAttachment final {
		IRHITexture*      m_Texture;
		Utils::RHILoadOp  m_LoadOp        = Utils::RHILoadOp::Load;
		Utils::RHIStoreOp m_StoreOp       = Utils::RHIStoreOp::Store;
	private:
		uint8_t           m_Pad[6]        = {};
	public:
		float             m_ClearValue[4] = { 0.f, 0.f, 0.f, 0.f };
	};

	// Describes the full set of live attachments bound for a BeginRendering/EndRendering scope. depthAttachment may be null.
	// RHIRenderingLayout (pipeline creation) describes formats only, for compatibility matching; this binds live textures.
	struct SPEC_RHI_ALIGNAS(8) RHIRenderingInfo final {
		const RHIRenderAttachment* m_ColorAttachments;
		uint32_t                   m_ColorCount;
		const RHIRenderAttachment* m_DepthAttachment = nullptr;
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

	// Describes an acceleration structure allocation. sizeHint must come from IRHIDevice::getAccelerationStructureBuildSizes.
	// AllowCompaction and AllowUpdate flags must be set at creation time; they cannot be added after build.
	struct SPEC_RHI_ALIGNAS(8) RHIASDesc final {
		uint64_t               m_SizeHint;
		Utils::RHIASBuildFlags m_Flags = Utils::RHIASBuildFlags::None;
		Utils::RHIASType       m_Type;
	private:
		uint8_t                m_Pad[3] = {};
	public:
	};

	// Describes build-time geometry for one acceleration structure build. A BLAS build reads the vertex/index buffer
	// fields; a TLAS build reads instanceBuffer, which must contain RHIASInstance-formatted data. Only one path is
	// populated per geometry entry.
	struct SPEC_RHI_ALIGNAS(8) RHIASGeometryDesc final {
		IRHIBuffer*          m_VertexBuffer  = nullptr;
		uint64_t             m_VertexOffset  = 0;
		uint32_t             m_VertexCount   = 0;
		uint32_t             m_VertexStride  = 0;
		Utils::RHIFormat     m_VertexFormat  = Utils::RHIFormat::RGB32_SFLOAT;
		IRHIBuffer*          m_IndexBuffer   = nullptr;
		uint64_t             m_IndexOffset   = 0;
		uint32_t             m_TriangleCount = 0;
		Utils::RHIIndexType  m_IndexType;
		IRHIBuffer*          m_InstanceBuffer = nullptr;
		uint32_t             m_InstanceCount  = 0;
	private:
		uint32_t             m_Pad = 0;
	public:
	};

	// One TLAS instance record in the RHI's unified cross-backend format — resolves the abstraction break where the
	// instance buffer was an untyped IRHIBuffer* requiring hand-constructed, backend-specific memory layouts.
	// transform is row-major 3x4. blasAddress comes from the referenced BLAS's IRHIAccelerationStructure::getTraversalAddress().
	struct SPEC_RHI_ALIGNAS(8) RHIASInstance final {
		float    m_Transform[12];
		uint32_t m_InstanceId;
		uint32_t m_InstanceMask = 0xFF;
		uint32_t m_SbtOffset    = 0;
		uint32_t m_Flags        = 0;
		uint64_t m_BlasAddress;
	};

	// Describes one acceleration structure build or refit. update=false is a full build (source ignored).
	// update=true with source==target performs an in-place refit; update=true with source!=target refits into a
	// distinct AS, leaving source intact. update=true with source==nullptr is RHIResult::InvalidUsage.
	// target must have been created with RHIASBuildFlags::AllowUpdate to be eligible for any update=true build.
	struct SPEC_RHI_ALIGNAS(8) RHIASBuildDesc final {
		IRHIAccelerationStructure* m_Target;
		IRHIAccelerationStructure* m_Source = nullptr;
		const RHIASGeometryDesc*   m_Geometries;
		uint32_t                   m_GeometryCount;
		bool                       m_Update = false;
	private:
		uint8_t                    m_Pad[3] = {};
	public:
		IRHIBuffer*                m_ScratchBuffer;
	};

	// One shader binding table record. inlineData/inlineDataSize supply per-record shader arguments; every record in
	// a given category (raygen/miss/hit/callable) is padded to the max inlineDataSize seen in that category.
	struct SPEC_RHI_ALIGNAS(8) RHISBTRecord final {
		uint32_t    m_GroupIndex;
		uint32_t    m_InlineDataSize = 0;
		const void* m_InlineData     = nullptr;
	};

	// Describes a full shader binding table build. rayTypeCount is the authoritative stride used at hit-group lookup
	// time (hitRecordIndex = instance.sbtOffset + geometryIndex * rayTypeCount + rayTypeIndex) and must match the
	// ray-type count assumed when the corresponding RHIASInstance::sbtOffset values were computed at TLAS build time.
	// A mismatch produces incorrect hit-record lookups with no error — silently wrong shading.
	struct SPEC_RHI_ALIGNAS(8) RHISBTDesc final {
		IRHIPipeline*       m_Pipeline; // must be IRHIPipeline::Kind::RayTracing
		const RHISBTRecord* m_RaygenRecords;
		uint32_t            m_RaygenCount;
		const RHISBTRecord* m_MissRecords;
		uint32_t            m_MissCount;
		const RHISBTRecord* m_HitRecords;
		uint32_t            m_HitCount;
		uint32_t            m_RayTypeCount;
		const RHISBTRecord* m_CallableRecords = nullptr;
		uint32_t            m_CallableCount   = 0;
	};

	// Describes a ray trace dispatch with launch dimensions and a prebuilt shader binding table.
	struct SPEC_RHI_ALIGNAS(8) RHITraceRaysDesc final {
		IRHIAccelerationStructure* m_AS;
		IRHIShaderBindingTable*    m_SBT;
		uint32_t                   m_Width;
		uint32_t                   m_Height;
		uint32_t                   m_Depth;
	private:
		uint32_t                   m_Pad = 0;
	public:
	};

	// Describes a queue submission with array-based wait/signal timeline fences, mirroring vkQueueSubmit2's
	// VkSubmitInfo2 array semantics. The common case (one wait, one signal) is a trivial construction with
	// waitCount = signalCount = 1.
	struct SPEC_RHI_ALIGNAS(8) RHISubmitDesc final {
		const IRHIFence* const* m_WaitFences;
		uint32_t                m_WaitCount;
		const uint64_t*         m_WaitValues;
		const IRHIFence* const* m_SignalFences;
		uint32_t                m_SignalCount;
		const uint64_t*         m_SignalValues;
	};

	// Carries per-frame synchronization context. frameIndex monotonically increases each frame.
	// For frames with multiple independent queues, pass the fence of the longest-lived queue.
	struct SPEC_RHI_ALIGNAS(8) RHIFrameContext final {
		uint64_t   m_FrameIndex;
		IRHIFence* m_CompletionFence;
	};
}
