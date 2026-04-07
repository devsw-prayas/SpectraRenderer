#pragma once

#include "SpectraCudaBackend.h"

namespace Spectra::Cuda::Utils {

	// =========================================================
	// OptiX Opaque Handles
	// =========================================================

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuOptixContext final {
		void* m_Handle = nullptr;

		GpuOptixContext() = default;
		~GpuOptixContext() = default;

		GpuOptixContext(const GpuOptixContext&) = default;
		GpuOptixContext& operator=(const GpuOptixContext&) = default;

		GpuOptixContext(GpuOptixContext&&) noexcept = default;
		GpuOptixContext& operator=(GpuOptixContext&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const { return m_Handle != nullptr; }
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(GpuOptixContext) == 8, "Invalid GpuOptixContext size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<GpuOptixContext>, "GpuOptixContext must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<GpuOptixContext>, "GpuOptixContext must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<GpuOptixContext>, "GpuOptixContext must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuOptixModule final {
		void* m_Handle = nullptr;

		GpuOptixModule() = default;
		~GpuOptixModule() = default;

		GpuOptixModule(const GpuOptixModule&) = default;
		GpuOptixModule& operator=(const GpuOptixModule&) = default;

		GpuOptixModule(GpuOptixModule&&) noexcept = default;
		GpuOptixModule& operator=(GpuOptixModule&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const { return m_Handle != nullptr; }
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(GpuOptixModule) == 8, "Invalid GpuOptixModule size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<GpuOptixModule>, "GpuOptixModule must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<GpuOptixModule>, "GpuOptixModule must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<GpuOptixModule>, "GpuOptixModule must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuOptixProgramGroup final {
		void* m_Handle = nullptr;

		GpuOptixProgramGroup() = default;
		~GpuOptixProgramGroup() = default;

		GpuOptixProgramGroup(const GpuOptixProgramGroup&) = default;
		GpuOptixProgramGroup& operator=(const GpuOptixProgramGroup&) = default;

		GpuOptixProgramGroup(GpuOptixProgramGroup&&) noexcept = default;
		GpuOptixProgramGroup& operator=(GpuOptixProgramGroup&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const { return m_Handle != nullptr; }
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(GpuOptixProgramGroup) == 8, "Invalid GpuOptixProgramGroup size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<GpuOptixProgramGroup>, "GpuOptixProgramGroup must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<GpuOptixProgramGroup>, "GpuOptixProgramGroup must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<GpuOptixProgramGroup>, "GpuOptixProgramGroup must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuOptixPipeline final {
		void* m_Handle = nullptr;

		GpuOptixPipeline() = default;
		~GpuOptixPipeline() = default;

		GpuOptixPipeline(const GpuOptixPipeline&) = default;
		GpuOptixPipeline& operator=(const GpuOptixPipeline&) = default;

		GpuOptixPipeline(GpuOptixPipeline&&) noexcept = default;
		GpuOptixPipeline& operator=(GpuOptixPipeline&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const { return m_Handle != nullptr; }
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(GpuOptixPipeline) == 8, "Invalid GpuOptixPipeline size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<GpuOptixPipeline>, "GpuOptixPipeline must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<GpuOptixPipeline>, "GpuOptixPipeline must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<GpuOptixPipeline>, "GpuOptixPipeline must be trivially move assignable");

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) GpuOptixTraversableHandle final {
		uint64_t m_Handle = 0;

		GpuOptixTraversableHandle() = default;
		~GpuOptixTraversableHandle() = default;

		GpuOptixTraversableHandle(const GpuOptixTraversableHandle&) = default;
		GpuOptixTraversableHandle& operator=(const GpuOptixTraversableHandle&) = default;

		GpuOptixTraversableHandle(GpuOptixTraversableHandle&&) noexcept = default;
		GpuOptixTraversableHandle& operator=(GpuOptixTraversableHandle&&) noexcept = default;

		SPEC_CUDA_BK_NODISCARD bool isValid() const { return m_Handle != 0; }
	};

	SPEC_CUDA_BK_STATIC_ASSERT(sizeof(GpuOptixTraversableHandle) == 8, "Invalid GpuOptixTraversableHandle size, must be 64bit");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_standard_layout_v<GpuOptixTraversableHandle>, "GpuOptixTraversableHandle must maintain standard layout");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_copyable_v<GpuOptixTraversableHandle>, "GpuOptixTraversableHandle must be trivially copyable");
	SPEC_CUDA_BK_STATIC_ASSERT(std::is_trivially_move_assignable_v<GpuOptixTraversableHandle>, "GpuOptixTraversableHandle must be trivially move assignable");

	// =========================================================
	// OptiX Core Enums
	// =========================================================

	enum class SPEC_CUDA_BK_RUNTIME_API OptixLogLevel final : uint8_t {
		DISABLE = 0,
		FATAL   = 1,
		ERROR   = 2,
		WARN    = 3,
		PRINT   = 4
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixValidationMode final : uint8_t {
		VALIDATION_OFF = 0,
		VALIDATION_ON  = 1
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixCompileOptimizationLevel final : uint8_t {
		LEVEL_0,
		LEVEL_1,
		LEVEL_2,
		LEVEL_3,
		DEFAULT = LEVEL_3
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixCompileDebugLevel final : uint8_t {
		LEVEL_NONE,
		LEVEL_MINIMAL,
		LEVEL_MODERATE,
		LEVEL_FULL,
		DEFAULT = LEVEL_NONE
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixProgramGroupKind final : uint8_t {
		RAYGEN,
		MISS,
		EXCEPTION,
		HITGROUP,
		CALLABLES
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixBuildInputType final : uint8_t {
		TRIANGLES,
		CUSTOM_PRIMITIVES,
		INSTANCES,
		INSTANCE_POINTERS
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixBuildOperation final : uint8_t {
		BUILD,
		UPDATE
	};

	enum class SPEC_CUDA_BK_RUNTIME_API OptixBuildFlags : uint8_t {
		NONE                       = 0,
		ALLOW_UPDATE               = 1 << 0,
		ALLOW_COMPACTION           = 1 << 1,
		PREFER_FAST_TRACE          = 1 << 2,
		PREFER_FAST_BUILD          = 1 << 3,
		ALLOW_RANDOM_VERTEX_ACCESS = 1 << 4
	};

	// =========================================================
	// OptiX Descriptor Structs
	// =========================================================

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixContextOptions final {
		using LogCallback = void(*)(unsigned int, const char*, const char*, void*);

		LogCallback         m_LogCallbackFunction = nullptr;
		void*               m_LogCallbackData     = nullptr;
		uint32_t            m_LogCallbackLevel    = 4;
		OptixValidationMode m_Validation          = OptixValidationMode::VALIDATION_OFF;

		OptixContextOptions() = default;
		~OptixContextOptions() = default;

		OptixContextOptions(const OptixContextOptions&) = default;
		OptixContextOptions& operator=(const OptixContextOptions&) = default;

		OptixContextOptions(OptixContextOptions&&) noexcept = default;
		OptixContextOptions& operator=(OptixContextOptions&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) OptixModuleCompileOptions final {
		int32_t                       m_MaxRegisterCount = 0;
		OptixCompileOptimizationLevel m_OptLevel         = OptixCompileOptimizationLevel::DEFAULT;
		OptixCompileDebugLevel        m_DebugLevel       = OptixCompileDebugLevel::DEFAULT;
		uint32_t                      m_BoundValuesCount = 0;

		OptixModuleCompileOptions() = default;
		~OptixModuleCompileOptions() = default;

		OptixModuleCompileOptions(const OptixModuleCompileOptions&) = default;
		OptixModuleCompileOptions& operator=(const OptixModuleCompileOptions&) = default;

		OptixModuleCompileOptions(OptixModuleCompileOptions&&) noexcept = default;
		OptixModuleCompileOptions& operator=(OptixModuleCompileOptions&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixPipelineCompileOptions final {
		bool        m_UsesMotionBlur                        = false;
		uint32_t    m_TraversableGraphFlags                 = 0;
		int32_t     m_NumPayloadValues                      = 0;
		int32_t     m_NumAttributeValues                    = 0;
		uint32_t    m_ExceptionFlags                        = 0;
		const char* m_PipelineLaunchParamsVariableName      = nullptr;
		uint32_t    m_UsesPrimitiveTypeFlags                = 0;

		OptixPipelineCompileOptions() = default;
		~OptixPipelineCompileOptions() = default;

		OptixPipelineCompileOptions(const OptixPipelineCompileOptions&) = default;
		OptixPipelineCompileOptions& operator=(const OptixPipelineCompileOptions&) = default;

		OptixPipelineCompileOptions(OptixPipelineCompileOptions&&) noexcept = default;
		OptixPipelineCompileOptions& operator=(OptixPipelineCompileOptions&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) OptixPipelineLinkOptions final {
		uint32_t m_MaxTraceDepth = 1;

		OptixPipelineLinkOptions() = default;
		~OptixPipelineLinkOptions() = default;

		OptixPipelineLinkOptions(const OptixPipelineLinkOptions&) = default;
		OptixPipelineLinkOptions& operator=(const OptixPipelineLinkOptions&) = default;

		OptixPipelineLinkOptions(OptixPipelineLinkOptions&&) noexcept = default;
		OptixPipelineLinkOptions& operator=(OptixPipelineLinkOptions&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixProgramGroupDesc final {
		OptixProgramGroupKind m_Kind;

		GpuOptixModule m_ModuleRaygen;
		const char*    m_EntryFunctionNameRaygen    = nullptr;

		GpuOptixModule m_ModuleMiss;
		const char*    m_EntryFunctionNameMiss      = nullptr;

		GpuOptixModule m_ModuleException;
		const char*    m_EntryFunctionNameException = nullptr;

		GpuOptixModule m_ModuleCallables;
		const char*    m_EntryFunctionNameDC        = nullptr;
		const char*    m_EntryFunctionNameCC        = nullptr;

		GpuOptixModule m_ModuleHitgroupCH;
		const char*    m_EntryFunctionNameCH        = nullptr;
		GpuOptixModule m_ModuleHitgroupAH;
		const char*    m_EntryFunctionNameAH        = nullptr;
		GpuOptixModule m_ModuleHitgroupIS;
		const char*    m_EntryFunctionNameIS        = nullptr;

		OptixProgramGroupDesc() = default;
		~OptixProgramGroupDesc() = default;

		OptixProgramGroupDesc(const OptixProgramGroupDesc&) = default;
		OptixProgramGroupDesc& operator=(const OptixProgramGroupDesc&) = default;

		OptixProgramGroupDesc(OptixProgramGroupDesc&&) noexcept = default;
		OptixProgramGroupDesc& operator=(OptixProgramGroupDesc&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) OptixStackSizes final {
		uint32_t m_CssRG = 0;
		uint32_t m_CssMS = 0;
		uint32_t m_CssCH = 0;
		uint32_t m_CssAH = 0;
		uint32_t m_CssIS = 0;
		uint32_t m_CssCC = 0;
		uint32_t m_DssDC = 0;

		OptixStackSizes() = default;
		~OptixStackSizes() = default;

		OptixStackSizes(const OptixStackSizes&) = default;
		OptixStackSizes& operator=(const OptixStackSizes&) = default;

		OptixStackSizes(OptixStackSizes&&) noexcept = default;
		OptixStackSizes& operator=(OptixStackSizes&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixBuildInputTriangleArray final {
		const uint64_t* m_VertexBuffers                 = nullptr;
		uint32_t        m_NumVertexBuffers              = 0;
		uint32_t        m_VertexFormat                  = 0;
		uint32_t        m_VertexStrideInBytes           = 0;
		uint32_t        m_NumVertices                   = 0;

		uint64_t        m_IndexBuffer                   = 0;
		uint32_t        m_IndexFormat                   = 0;
		uint32_t        m_IndexStrideInBytes            = 0;
		uint32_t        m_NumIndexTriplets              = 0;

		const uint32_t* m_Flags                         = nullptr;
		uint32_t        m_NumSbtRecords                 = 0;
		uint64_t        m_SbtIndexOffsetBuffer          = 0;
		uint32_t        m_SbtIndexOffsetSizeInBytes     = 0;
		uint32_t        m_SbtIndexOffsetStrideInBytes   = 0;

		OptixBuildInputTriangleArray() = default;
		~OptixBuildInputTriangleArray() = default;

		OptixBuildInputTriangleArray(const OptixBuildInputTriangleArray&) = default;
		OptixBuildInputTriangleArray& operator=(const OptixBuildInputTriangleArray&) = default;

		OptixBuildInputTriangleArray(OptixBuildInputTriangleArray&&) noexcept = default;
		OptixBuildInputTriangleArray& operator=(OptixBuildInputTriangleArray&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixBuildInputInstanceArray final {
		uint64_t m_Instances    = 0;
		uint32_t m_NumInstances = 0;

		OptixBuildInputInstanceArray() = default;
		~OptixBuildInputInstanceArray() = default;

		OptixBuildInputInstanceArray(const OptixBuildInputInstanceArray&) = default;
		OptixBuildInputInstanceArray& operator=(const OptixBuildInputInstanceArray&) = default;

		OptixBuildInputInstanceArray(OptixBuildInputInstanceArray&&) noexcept = default;
		OptixBuildInputInstanceArray& operator=(OptixBuildInputInstanceArray&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixBuildInputDesc final {
		OptixBuildInputType          m_Type;
		OptixBuildInputTriangleArray m_TriangleArray;
		OptixBuildInputInstanceArray m_InstanceArray;

		OptixBuildInputDesc() = default;
		~OptixBuildInputDesc() = default;

		OptixBuildInputDesc(const OptixBuildInputDesc&) = default;
		OptixBuildInputDesc& operator=(const OptixBuildInputDesc&) = default;

		OptixBuildInputDesc(OptixBuildInputDesc&&) noexcept = default;
		OptixBuildInputDesc& operator=(OptixBuildInputDesc&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(4) OptixAccelBuildOptions final {
		uint32_t            m_BuildFlags = 0;
		OptixBuildOperation m_Operation  = OptixBuildOperation::BUILD;

		OptixAccelBuildOptions() = default;
		~OptixAccelBuildOptions() = default;

		OptixAccelBuildOptions(const OptixAccelBuildOptions&) = default;
		OptixAccelBuildOptions& operator=(const OptixAccelBuildOptions&) = default;

		OptixAccelBuildOptions(OptixAccelBuildOptions&&) noexcept = default;
		OptixAccelBuildOptions& operator=(OptixAccelBuildOptions&&) noexcept = default;
	};

	struct SPEC_CUDA_BK_RUNTIME_API SPEC_CUDA_BK_ALIGNAS(8) OptixAccelBufferSizes final {
		size_t m_OutputSizeInBytes      = 0;
		size_t m_TempSizeInBytes        = 0;
		size_t m_TempUpdateSizeInBytes  = 0;

		OptixAccelBufferSizes() = default;
		~OptixAccelBufferSizes() = default;

		OptixAccelBufferSizes(const OptixAccelBufferSizes&) = default;
		OptixAccelBufferSizes& operator=(const OptixAccelBufferSizes&) = default;

		OptixAccelBufferSizes(OptixAccelBufferSizes&&) noexcept = default;
		OptixAccelBufferSizes& operator=(OptixAccelBufferSizes&&) noexcept = default;
	};
}
