#pragma once

#include "VulkanConstants.h"

namespace Spectra::Vulkan::Utils {

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) PhysicalDevice final {
		void* m_DeviceHandle;
		void* m_PropertiesHandle;

		bool isValid() const { return m_DeviceHandle != nullptr && m_PropertiesHandle != nullptr; }

		PhysicalDevice() : m_DeviceHandle(nullptr), m_PropertiesHandle(nullptr) {}
		PhysicalDevice(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;
		PhysicalDevice(PhysicalDevice&&) noexcept = default;
		PhysicalDevice& operator=(PhysicalDevice&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) FenceHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		FenceHandle() : m_Handle(nullptr) {}
		FenceHandle(const FenceHandle&) = default;
		FenceHandle& operator=(const FenceHandle&) = default;
		FenceHandle(FenceHandle&&) noexcept = default;
		FenceHandle& operator=(FenceHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SemaphoreHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SemaphoreHandle() : m_Handle(nullptr) {}
		SemaphoreHandle(const SemaphoreHandle&) = default;
		SemaphoreHandle& operator=(const SemaphoreHandle&) = default;
		SemaphoreHandle(SemaphoreHandle&&) noexcept = default;
		SemaphoreHandle& operator=(SemaphoreHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) CommandBufferHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		CommandBufferHandle() : m_Handle(nullptr) {}
		CommandBufferHandle(const CommandBufferHandle&) = default;
		CommandBufferHandle& operator=(const CommandBufferHandle&) = default;
		CommandBufferHandle(CommandBufferHandle&&) noexcept = default;
		CommandBufferHandle& operator=(CommandBufferHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) BufferHandle final {
		void* m_Handle;
		void* m_Allocation;

		bool isValid() const { return m_Handle != nullptr && m_Allocation != nullptr; }

		BufferHandle() : m_Handle(nullptr), m_Allocation(nullptr) {}
		BufferHandle(const BufferHandle&) = default;
		BufferHandle& operator=(const BufferHandle&) = default;
		BufferHandle(BufferHandle&&) noexcept = default;
		BufferHandle& operator=(BufferHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) ImageHandle final {
		void* m_Handle;
		void* m_Allocation;

		bool isValid() const { return m_Handle != nullptr && m_Allocation != nullptr; }

		ImageHandle() : m_Handle(nullptr), m_Allocation(nullptr) {}
		ImageHandle(const ImageHandle&) = default;
		ImageHandle& operator=(const ImageHandle&) = default;
		ImageHandle(ImageHandle&&) noexcept = default;
		ImageHandle& operator=(ImageHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ImageViewHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		ImageViewHandle() : m_Handle(nullptr) {}
		ImageViewHandle(const ImageViewHandle&) = default;
		ImageViewHandle& operator=(const ImageViewHandle&) = default;
		ImageViewHandle(ImageViewHandle&&) noexcept = default;
		ImageViewHandle& operator=(ImageViewHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SamplerHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SamplerHandle() : m_Handle(nullptr) {}
		SamplerHandle(const SamplerHandle&) = default;
		SamplerHandle& operator=(const SamplerHandle&) = default;
		SamplerHandle(SamplerHandle&&) noexcept = default;
		SamplerHandle& operator=(SamplerHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SurfaceHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SurfaceHandle() : m_Handle(nullptr) {}
		SurfaceHandle(const SurfaceHandle&) = default;
		SurfaceHandle& operator=(const SurfaceHandle&) = default;
		SurfaceHandle(SurfaceHandle&&) noexcept = default;
		SurfaceHandle& operator=(SurfaceHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SwapchainHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SwapchainHandle() : m_Handle(nullptr) {}
		SwapchainHandle(const SwapchainHandle&) = default;
		SwapchainHandle& operator=(const SwapchainHandle&) = default;
		SwapchainHandle(SwapchainHandle&&) noexcept = default;
		SwapchainHandle& operator=(SwapchainHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorSetLayoutHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorSetLayoutHandle() : m_Handle(nullptr) {}
		DescriptorSetLayoutHandle(const DescriptorSetLayoutHandle&) = default;
		DescriptorSetLayoutHandle& operator=(const DescriptorSetLayoutHandle&) = default;
		DescriptorSetLayoutHandle(DescriptorSetLayoutHandle&&) noexcept = default;
		DescriptorSetLayoutHandle& operator=(DescriptorSetLayoutHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorPoolHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorPoolHandle() : m_Handle(nullptr) {}
		DescriptorPoolHandle(const DescriptorPoolHandle&) = default;
		DescriptorPoolHandle& operator=(const DescriptorPoolHandle&) = default;
		DescriptorPoolHandle(DescriptorPoolHandle&&) noexcept = default;
		DescriptorPoolHandle& operator=(DescriptorPoolHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorSetHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorSetHandle() : m_Handle(nullptr) {}
		DescriptorSetHandle(const DescriptorSetHandle&) = default;
		DescriptorSetHandle& operator=(const DescriptorSetHandle&) = default;
		DescriptorSetHandle(DescriptorSetHandle&&) noexcept = default;
		DescriptorSetHandle& operator=(DescriptorSetHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineLayoutHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineLayoutHandle() : m_Handle(nullptr) {}
		PipelineLayoutHandle(const PipelineLayoutHandle&) = default;
		PipelineLayoutHandle& operator=(const PipelineLayoutHandle&) = default;
		PipelineLayoutHandle(PipelineLayoutHandle&&) noexcept = default;
		PipelineLayoutHandle& operator=(PipelineLayoutHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineHandle() : m_Handle(nullptr) {}
		PipelineHandle(const PipelineHandle&) = default;
		PipelineHandle& operator=(const PipelineHandle&) = default;
		PipelineHandle(PipelineHandle&&) noexcept = default;
		PipelineHandle& operator=(PipelineHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ShaderModuleHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		ShaderModuleHandle() : m_Handle(nullptr) {}
		ShaderModuleHandle(const ShaderModuleHandle&) = default;
		ShaderModuleHandle& operator=(const ShaderModuleHandle&) = default;
		ShaderModuleHandle(ShaderModuleHandle&&) noexcept = default;
		ShaderModuleHandle& operator=(ShaderModuleHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineCacheHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineCacheHandle() : m_Handle(nullptr) {}
		PipelineCacheHandle(const PipelineCacheHandle&) = default;
		PipelineCacheHandle& operator=(const PipelineCacheHandle&) = default;
		PipelineCacheHandle(PipelineCacheHandle&&) noexcept = default;
		PipelineCacheHandle& operator=(PipelineCacheHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) AccelerationStructureHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		AccelerationStructureHandle() : m_Handle(nullptr) {}
		AccelerationStructureHandle(const AccelerationStructureHandle&) = default;
		AccelerationStructureHandle& operator=(const AccelerationStructureHandle&) = default;
		AccelerationStructureHandle(AccelerationStructureHandle&&) noexcept = default;
		AccelerationStructureHandle& operator=(AccelerationStructureHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) QueryPoolHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		QueryPoolHandle() : m_Handle(nullptr) {}
		QueryPoolHandle(const QueryPoolHandle&) = default;
		QueryPoolHandle& operator=(const QueryPoolHandle&) = default;
		QueryPoolHandle(QueryPoolHandle&&) noexcept = default;
		QueryPoolHandle& operator=(QueryPoolHandle&&) noexcept = default;
	};
}
