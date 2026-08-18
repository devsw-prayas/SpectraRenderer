#pragma once
#include "RHIUtils.h"
#include "RHIDescs.h"
#include "SpecRHICompiler.h"

namespace Spectra::RHI {
	class IRHIFence;
	class IRHIQuery;
	class IRHIResource;
	class IRHIBuffer;
	class IRHITexture;
	class IRHISampler;
	class IRHIPipeline;
	class IRHIBindingLayout;
	class IRHIBindingSet;
	class IRHIAccelerationStructure;
	class IRHIShaderBindingTable;
	class IRHICommandList;
	class IRHIQueue;
	class IRHIStreamingEngine;
	class IRHIDevice;

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

	// Timeline semaphore abstraction. Maps to a VkSemaphore(TIMELINE) on Vulkan and a CUDA event pair on CUDA.
	// The counter is monotonically increasing; signal() must always be called with a value strictly greater than the current one.
	class IRHIFence : public IRHIObject {
	public:
		// Blocks the calling CPU thread until the fence reaches v_Value, or returns RHIResult::Timeout after
		// v_TimeoutNs elapses. v_TimeoutNs == UINT64_MAX waits forever.
		virtual Utils::RHIResult wait(uint64_t v_Value, uint64_t v_TimeoutNs) ABSTRACT;

		// Advances the fence counter from the CPU side. v_Value must be > getCurrentValue().
		virtual Utils::RHIResult signal(uint64_t v_Value) ABSTRACT;

		// Returns the last value the GPU has signalled. Non-blocking; safe to poll every frame.
		SPEC_RHI_NODISCARD virtual uint64_t getCurrentValue() const ABSTRACT;

		IRHIFence() = delete;
		IRHIFence(const IRHIFence&) = delete;
		IRHIFence& operator=(const IRHIFence&) noexcept = delete;
		~IRHIFence() override = default;
	};

	class IRHIQuery : public IRHIObject {
	public:
		// Results are only valid after the associated fence has signalled.
		virtual Utils::RHIResult getTimestampResults(uint64_t* p_OutValues, uint32_t v_Count) ABSTRACT;

		// Converts raw GPU ticks to nanoseconds.
		SPEC_RHI_NODISCARD virtual double getTimestampPeriodNs() const ABSTRACT;
	};

	class IRHIResource : public IRHIObject {
	public:
		virtual uint64_t getDeviceAddress() ABSTRACT;
		virtual Utils::RHIResourceOwner getCurrentOwner() ABSTRACT;
		SPEC_RHI_NODISCARD virtual Utils::RHIResourceOrigin getOrigin() const ABSTRACT;
	};

	// Map() is only valid on Upload and Readback allocations; calling it on a Device-local buffer is a validation error.
	// PersistentlyMapped buffers remain mapped for their entire lifetime — do not call unmap() on them.
	class IRHIBuffer : public IRHIResource {
	public:
		virtual void* map() ABSTRACT;
		virtual void unmap() ABSTRACT;
		SPEC_RHI_NODISCARD virtual Utils::RHIMemoryFlags getMemoryFlags() const ABSTRACT;
	};

	// Typed image resource. Wraps a VkImage+VkImageView pair on Vulkan, a cudaArray_t + texture/surface object on CUDA.
	// Mip chains are not exposed directly; mip selection happens at the binding or command level.
	class IRHITexture : public IRHIResource {
	public:
		SPEC_RHI_NODISCARD virtual uint32_t getWidth() const ABSTRACT;
		SPEC_RHI_NODISCARD virtual uint32_t getHeight() const ABSTRACT;
		SPEC_RHI_NODISCARD virtual uint32_t getDepth() const ABSTRACT; // 1 for 2D
		SPEC_RHI_NODISCARD virtual uint32_t getMipLevels() const ABSTRACT;
		SPEC_RHI_NODISCARD virtual uint32_t getArrayLayers() const ABSTRACT;
		SPEC_RHI_NODISCARD virtual Utils::RHIFormat getFormat() const ABSTRACT;

		IRHITexture() = delete;
		IRHITexture(const IRHITexture&) = delete;
		IRHITexture& operator=(const IRHITexture&) noexcept = delete;
		~IRHITexture() override = default;
	};

	// Wraps sampler state for texture sampling. On Vulkan this is a real VkSampler; on CUDA it is a lightweight
	// ref-counted holder whose RHISamplerDesc contents are folded into the cudaTextureDesc/cudaResourceDesc pair
	// at bind time — CUDA creates no driver-level sampler object.
	class IRHISampler : public IRHIObject {
	public:
		SPEC_RHI_NODISCARD virtual const RHISamplerDesc& getDesc() const ABSTRACT;

		IRHISampler() = delete;
		IRHISampler(const IRHISampler&) = delete;
		IRHISampler& operator=(const IRHISampler&) noexcept = delete;
		~IRHISampler() override = default;
	};

	// Discriminates pipeline kind for command-list bind-point validation without an application-side cast.
	class IRHIPipeline : public IRHIObject {
	public:
		enum class Kind : uint8_t { Graphics, Compute, RayTracing };

		SPEC_RHI_NODISCARD virtual Kind getKind() const ABSTRACT;
		SPEC_RHI_NODISCARD virtual IRHIBindingLayout* getBindingLayout() const ABSTRACT;
	};

	// Maps to VkDescriptorSetLayout + VkPipelineLayout on Vulkan; a flat index array used to pack kernel params on CUDA.
	class IRHIBindingLayout : public IRHIObject {
	public:
		SPEC_RHI_NODISCARD virtual const RHIBindingLayoutDesc& getDesc() const ABSTRACT;

		IRHIBindingLayout() = delete;
		IRHIBindingLayout(const IRHIBindingLayout&) = delete;
		IRHIBindingLayout& operator=(const IRHIBindingLayout&) noexcept = delete;
		~IRHIBindingLayout() override = default;
	};

	// On CUDA, updateBindings() writes into a flat void*[] forwarded as kernel params at launch time.
	class IRHIBindingSet : public IRHIObject {
	public:
		virtual void updateBindings(const RHIBinding* p_Bindings, uint32_t v_Count) ABSTRACT;
	};

	// Hardware acceleration structures (VK_KHR_ray_tracing_pipeline / OptiX) expose a GPU traversal address;
	// software BVH8 fallbacks (no driver-level acceleration structure support) expose a backing buffer instead.
	// getKind() determines which accessor is valid for a given instance — the other is meaningless.
	class IRHIAccelerationStructure : public IRHIObject {
	public:
		enum class Kind : uint8_t { Hardware, SoftwareBVH8 };

		SPEC_RHI_NODISCARD virtual Kind getKind() const ABSTRACT;

		virtual uint64_t getTraversalAddress() ABSTRACT; // valid only if Kind::Hardware
		virtual IRHIBuffer* getBackingBuffer() ABSTRACT;  // valid only if Kind::SoftwareBVH8
	};

	// Opaque handle to a built shader binding table. Backend construction — the SBT buffer and strided address
	// regions on Vulkan, the OptixShaderBindingTable struct on CUDA — is fully owned by createShaderBindingTable();
	// IRHICommandList::traceRays requires no additional per-call SBT logic on either backend.
	class IRHIShaderBindingTable : public IRHIObject {
	public:
		IRHIShaderBindingTable() = delete;
		IRHIShaderBindingTable(const IRHIShaderBindingTable&) = delete;
		IRHIShaderBindingTable& operator=(const IRHIShaderBindingTable&) noexcept = delete;
		~IRHIShaderBindingTable() override = default;
	};

	class IRHICommandList : public IRHIObject {
	public:
		// Implicitly resets this list's recorded state and its underlying command pool allocation. A list must not
		// be re-recorded while a prior submit() using it is still in flight — the application is responsible for
		// frame-in-flight rotation, gated by the list's associated completion fence.
		virtual void begin() ABSTRACT;
		virtual void end() ABSTRACT;

		// Barriers
		virtual void barrier(uint32_t v_Count, const RHIBarrierDesc* p_Barriers) ABSTRACT;

		// Upload. p_Src is read by the GPU whenever this command list actually executes, not at call time — the
		// caller must guarantee it remains valid for the full pipelined frame. Prefer a persistently-mapped
		// Upload-location buffer over transient stack/heap memory as the source.
		virtual void uploadBuffer(IRHIBuffer* p_Dst, uint64_t v_DstOffset, const void* p_Src, uint64_t v_Size) ABSTRACT;
		virtual void uploadTexture(IRHITexture* p_Dst, const void* p_Src) ABSTRACT;

		// GPU-side copy. Distinct from uploadBuffer/uploadTexture (CPU->GPU only) — these are GPU-to-GPU, used for
		// readback staging, mip generation via blit, and texture-to-texture history-buffer copies.
		virtual void copyBuffer(IRHIBuffer* p_Src, uint64_t v_SrcOffset, IRHIBuffer* p_Dst, uint64_t v_DstOffset, uint64_t v_Size) ABSTRACT;
		virtual void copyTexture(IRHITexture* p_Src, IRHITexture* p_Dst) ABSTRACT;
		virtual void copyBufferToTexture(IRHIBuffer* p_Src, IRHITexture* p_Dst) ABSTRACT;
		virtual void copyTextureToBuffer(IRHITexture* p_Src, IRHIBuffer* p_Dst) ABSTRACT;
		virtual void blitTexture(IRHITexture* p_Src, IRHITexture* p_Dst) ABSTRACT;
		virtual void fillBuffer(IRHIBuffer* p_Dst, uint64_t v_Offset, uint64_t v_Size, uint32_t v_Value) ABSTRACT;
		virtual void clearTexture(IRHITexture* p_Texture, const float v_ClearValue[4]) ABSTRACT;

		// Binding
		virtual void bindPipeline(IRHIPipeline* p_Pipeline) ABSTRACT;
		virtual void bindBindingSet(IRHIBindingSet* p_Set, uint32_t v_Index,
			const uint32_t* p_DynamicOffsets = nullptr, uint32_t v_DynamicOffsetCount = 0) ABSTRACT;
		virtual void setPushConstants(const void* p_Data, uint32_t v_Size) ABSTRACT;
		virtual void bindVertexBuffers(uint32_t v_FirstBinding, uint32_t v_Count,
			IRHIBuffer* const* p_Buffers, const uint64_t* p_Offsets) ABSTRACT;
		virtual void bindIndexBuffer(IRHIBuffer* p_Buffer, uint64_t v_Offset, Utils::RHIIndexType v_IndexType) ABSTRACT;

		// Dynamic state
		virtual void setViewport(float v_X, float v_Y, float v_Width, float v_Height, float v_MinDepth, float v_MaxDepth) ABSTRACT;
		virtual void setScissor(int32_t v_X, int32_t v_Y, uint32_t v_Width, uint32_t v_Height) ABSTRACT;

		// Rendering. Brackets a scope of live render target attachments; see RHIRenderingInfo.
		virtual void beginRendering(const RHIRenderingInfo& r_Info) ABSTRACT;
		virtual void endRendering() ABSTRACT;

		// Dispatch / Draw
		virtual void dispatch(uint32_t v_X, uint32_t v_Y, uint32_t v_Z) ABSTRACT;
		virtual void draw(uint32_t v_VertexCount, uint32_t v_InstanceCount, uint32_t v_FirstVertex, uint32_t v_FirstInstance) ABSTRACT;
		virtual void drawIndexed(uint32_t v_IndexCount, uint32_t v_InstanceCount, uint32_t v_FirstIndex,
			int32_t v_VertexOffset, uint32_t v_FirstInstance) ABSTRACT;

		// Indirect
		virtual void drawIndirect(IRHIBuffer* p_Args, uint64_t v_Offset, uint32_t v_DrawCount, uint32_t v_Stride) ABSTRACT;
		virtual void drawIndexedIndirect(IRHIBuffer* p_Args, uint64_t v_Offset, uint32_t v_DrawCount, uint32_t v_Stride) ABSTRACT;
		virtual void dispatchIndirect(IRHIBuffer* p_Args, uint64_t v_Offset) ABSTRACT;
		virtual void drawIndirectCount(IRHIBuffer* p_Args, uint64_t v_ArgsOffset, IRHIBuffer* p_CountBuffer,
			uint64_t v_CountOffset, uint32_t v_MaxDrawCount, uint32_t v_Stride) ABSTRACT;

		// Ray tracing
		virtual void traceRays(IRHIPipeline* p_Pipeline, const RHITraceRaysDesc& r_Desc) ABSTRACT;
		virtual void buildAccelerationStructures(uint32_t v_Count, const RHIASBuildDesc* p_Builds) ABSTRACT;
		virtual void compactAccelerationStructure(IRHIAccelerationStructure* p_Src, IRHIAccelerationStructure* p_Dst) ABSTRACT;

		// Timestamp / Query
		virtual void writeTimestamp(IRHIQuery* p_Query, Utils::RHIStage v_Stage) ABSTRACT;
		virtual void resetQuery(IRHIQuery* p_Query, uint32_t v_First, uint32_t v_Count) ABSTRACT;

		// Debug
		virtual void pushDebugGroup(const char* p_Label) ABSTRACT;
		virtual void popDebugGroup() ABSTRACT;
	};

	// CUDA_Stream is not a Vulkan queue family: submit() on a CUDA_Stream queue does not produce a VkSubmitInfo.
	// RHISubmitDesc's wait/signal fence arrays map to cudaStreamWaitEvent / cudaEventRecord respectively there.
	class IRHIQueue : public IRHIObject {
	public:
		virtual void submit(IRHICommandList* const* p_CmdLists, uint32_t v_CmdListCount, const RHISubmitDesc& r_Desc) ABSTRACT;

		// Hard CPU stall on the entire queue's history. Teardown-only — not a per-frame or per-draw synchronization
		// primitive; using it as such defeats the frame-pipelining model by forcing full CPU/GPU sync on every call.
		// For fine-grained waits on specific GPU work, use IRHIFence::wait(value, timeoutNs) directly.
		virtual void waitIdle() ABSTRACT;
	};

	// Optional capability for large asset streaming. Call IRHIDevice::getCapabilities().m_SupportsGPUDirect before
	// constructing a CUDA streaming engine. Vulkan uses an async transfer queue; CUDA uses native GPUDirect Storage.
	class IRHIStreamingEngine {
	public:
		// Asynchronously loads a file directly into VRAM. Returns a fence the caller can wait on.
		virtual IRHIFence* loadAssetToVRAM(const char* p_Path, IRHIBuffer* p_Dst, uint64_t v_Offset) ABSTRACT;

		virtual ~IRHIStreamingEngine() = default;
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
		SPEC_RHI_MAYBE_UNUSED bool pad_ = false;
	public:
		uint64_t    m_MaxBufferSize;
		uint32_t    m_MaxRayRecursionDepth;
		uint32_t    m_MaxTextureDimension2D = 8192;
		uint32_t    m_MaxTextureDimension3D = 2048;
		uint32_t    m_MaxArrayLayers        = 2048;
		uint32_t    m_GraphicsQueueCount    = 1;
		uint32_t    m_ComputeQueueCount     = 1;
		uint32_t    m_TransferQueueCount    = 1;
		uint32_t    m_DriverVersion         = 0;
		const char* m_DriverName            = nullptr;

		RHICapabilities() = default;
		RHICapabilities(const RHICapabilities&) = default;
		RHICapabilities(RHICapabilities&&) noexcept = default;
		RHICapabilities& operator=(const RHICapabilities&) = default;
		RHICapabilities& operator=(RHICapabilities&&) noexcept = default;
	};

	// Factory for all GPU objects. Capability querying via getCapabilities() must happen before any resource creation.
	// Constructed exclusively via RHIBuilder — IRHIDevice has no public construction path of its own.
	class IRHIDevice : public IRHIObject {
	public:
		SPEC_RHI_NODISCARD virtual const RHICapabilities& getCapabilities() const ABSTRACT;
		virtual void setDebugCallback(Utils::RHIDebugCallback v_Callback) ABSTRACT;
		SPEC_RHI_NODISCARD virtual Utils::RHIResult getFormatSupport(Utils::RHIFormat v_Format,
			Utils::RHIBindingType v_Usage, bool* p_OutSupported) const ABSTRACT;

		// Lifetime. Queues p_Object (or p_Objects) for destruction; drained at the start of each IRHIQueue::submit
		// once p_Fence reaches v_FenceValue. Passing a null fence is a validation error.
		virtual void deferRelease(IRHIObject* p_Object, IRHIFence* p_Fence, uint64_t v_FenceValue) ABSTRACT;
		virtual void deferReleaseBatch(IRHIObject* const* p_Objects, uint32_t v_Count, IRHIFence* p_Fence, uint64_t v_FenceValue) ABSTRACT;

		// Factories
		virtual Utils::RHIResult createBuffer(const RHIBufferDesc& r_Desc, IRHIBuffer** p_Out) ABSTRACT;
		virtual Utils::RHIResult createTexture(const RHITextureDesc& r_Desc, IRHITexture** p_Out) ABSTRACT;
		virtual Utils::RHIResult createSampler(const RHISamplerDesc& r_Desc, IRHISampler** p_Out) ABSTRACT;

		// Graphics pipelines return RHIResult::Unsupported on the CUDA backend.
		virtual Utils::RHIResult createGraphicsPipeline(const RHIGraphicsPipelineDesc& r_Desc, IRHIPipeline** p_Out) ABSTRACT;
		virtual Utils::RHIResult createComputePipeline(const RHIComputePipelineDesc& r_Desc, IRHIPipeline** p_Out) ABSTRACT;
		virtual Utils::RHIResult createRayTracingPipeline(const RHIRTPipelineDesc& r_Desc, IRHIPipeline** p_Out) ABSTRACT;

		virtual Utils::RHIResult createBindingLayout(const RHIBindingLayoutDesc& r_Desc, IRHIBindingLayout** p_Out) ABSTRACT;
		virtual Utils::RHIResult createBindingSet(IRHIBindingLayout* p_Layout, IRHIBindingSet** p_Out) ABSTRACT;

		virtual Utils::RHIResult createFence(uint64_t v_InitialValue, bool v_ExportForCuda, IRHIFence** p_Out) ABSTRACT;
		virtual Utils::RHIResult createQuery(Utils::RHIQueryType v_Type, uint32_t v_Count, IRHIQuery** p_Out) ABSTRACT;
		virtual Utils::RHIResult createQueue(Utils::RHIQueueType v_Type, IRHIQueue** p_Out) ABSTRACT;
		virtual Utils::RHIResult createCommandList(Utils::RHIQueueType v_Type, IRHICommandList** p_Out) ABSTRACT;

		// Acceleration structures. RHIASDesc::m_SizeHint must come from getAccelerationStructureBuildSizes.
		virtual Utils::RHIResult getAccelerationStructureBuildSizes(const RHIASGeometryDesc* p_Geometries, uint32_t v_GeometryCount,
			Utils::RHIASType v_Type, Utils::RHIASBuildFlags v_Flags, uint64_t* p_OutASSize, uint64_t* p_OutScratchSize) ABSTRACT;
		virtual Utils::RHIResult createAccelerationStructure(const RHIASDesc& r_Desc, IRHIAccelerationStructure** p_Out) ABSTRACT;
		virtual Utils::RHIResult getCompactedSize(IRHIAccelerationStructure* p_AS, uint64_t* p_OutSize) ABSTRACT;

		// Shader binding table
		virtual Utils::RHIResult createShaderBindingTable(const RHISBTDesc& r_Desc, IRHIShaderBindingTable** p_Out) ABSTRACT;

		// CreateStreamingEngine on Vulkan uses the async transfer queue path; on CUDA it requires m_SupportsGPUDirect == true.
		virtual Utils::RHIResult createStreamingEngine(IRHIStreamingEngine** p_Out) ABSTRACT;

		// Frame lifecycle. Owned entirely by this device — no cross-device frame concept exists in the RHI. Where
		// more than one IRHIDevice exists concurrently, each tracks its own frame lifecycle independently.
		virtual RHIFrameContext beginFrame() ABSTRACT;
		virtual void endFrame(RHIFrameContext v_Context) ABSTRACT;
		SPEC_RHI_NODISCARD virtual RHIFrameContext getCurrentFrameContext() const ABSTRACT;
	};

	// Sole construction path for an IRHIDevice; each builder and device is independently scoped.
	// Required capabilities are hard build-time gates and return Unsupported when unavailable.
	class RHIBuilder final {
	public:
		explicit RHIBuilder(Utils::RHIBackendType v_Backend);

		// Common across both backends
		RHIBuilder& setDebugCallback(Utils::RHIDebugCallback v_Callback);
		RHIBuilder& enableValidation(bool v_Enable);

		// Vulkan-specific — ignored on the CUDA backend
		RHIBuilder& setPreferredAdapter(uint32_t v_AdapterIndex);
		RHIBuilder& requireRayTracing(bool v_Required);

		// CUDA-specific — ignored on the Vulkan backend
		RHIBuilder& setCudaDeviceOrdinal(int v_Ordinal);
		RHIBuilder& requireGPUDirect(bool v_Required);

		Utils::RHIResult build(IRHIDevice** p_Out);

	private:
		Utils::RHIBackendType   m_Backend;
		Utils::RHIDebugCallback m_DebugCallback     = nullptr;
		uint32_t                m_PreferredAdapter  = ~0u;
		int                     m_CudaDeviceOrdinal = 0;
		bool                    m_EnableValidation  = false;
		bool                    m_RequireRayTracing = false;
		bool                    m_RequireGPUDirect  = false;
	};
}
