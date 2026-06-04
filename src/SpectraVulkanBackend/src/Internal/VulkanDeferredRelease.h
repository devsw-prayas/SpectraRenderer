#pragma once
#include "VulkanUtils.h"
#define ALLOW_SYSCALL
#include "SpecVulkanSyscalls.h"

namespace Spectra::Vulkan::Internal {

	// Ring-slotted deferred destruction queue. Resources pushed in frame N are destroyed
	// when advance() is called for the slot that wraps back to N after FRAMES_IN_FLIGHT
	// frames, guaranteeing the GPU has finished with them.
	struct DeferReleaseQueue final {
		static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
		static constexpr uint32_t MAX_FENCES              = 64;
		static constexpr uint32_t MAX_SEMAPHORES          = 64;
		static constexpr uint32_t MAX_BUFFERS             = 64;
		static constexpr uint32_t MAX_IMAGES              = 32;
		static constexpr uint32_t MAX_IMAGE_VIEWS         = 64;
		static constexpr uint32_t MAX_SAMPLERS            = 32;
		static constexpr uint32_t MAX_ACCEL_STRUCTURES    = 16;

		struct FrameSlot {
			Utils::FenceHandle                m_Fences           [MAX_FENCES];
			Utils::SemaphoreHandle            m_Semaphores       [MAX_SEMAPHORES];
			Utils::BufferHandle               m_Buffers          [MAX_BUFFERS];
			Utils::ImageHandle                m_Images           [MAX_IMAGES];
			Utils::ImageViewHandle            m_ImageViews       [MAX_IMAGE_VIEWS];
			Utils::SamplerHandle              m_Samplers         [MAX_SAMPLERS];
			Utils::AccelerationStructureHandle m_AccelStructures [MAX_ACCEL_STRUCTURES];

			uint32_t m_FenceCount;
			uint32_t m_SemaphoreCount;
			uint32_t m_BufferCount;
			uint32_t m_ImageCount;
			uint32_t m_ImageViewCount;
			uint32_t m_SamplerCount;
			uint32_t m_AccelStructureCount;

			void reset() {
				m_FenceCount          = 0;
				m_SemaphoreCount      = 0;
				m_BufferCount         = 0;
				m_ImageCount          = 0;
				m_ImageViewCount      = 0;
				m_SamplerCount        = 0;
				m_AccelStructureCount = 0;
			}
		};

		FrameSlot m_Slots[FRAMES_IN_FLIGHT];
		uint32_t  m_CurrentSlot;

		DeferReleaseQueue();

		void deferFence          (const Utils::FenceHandle&                r_Handle);
		void deferSemaphore      (const Utils::SemaphoreHandle&            r_Handle);
		void deferBuffer         (const Utils::BufferHandle&               r_Handle);
		void deferImage          (const Utils::ImageHandle&                r_Handle);
		void deferImageView      (const Utils::ImageViewHandle&            r_Handle);
		void deferSampler        (const Utils::SamplerHandle&              r_Handle);
		void deferAccelStructure (const Utils::AccelerationStructureHandle& r_Handle);

		// Advances to the next slot, flushes it (destroying anything FRAMES_IN_FLIGHT
		// frames old), then exposes it for the current frame's deferred resources.
		// Call once per frame before recording new work.
		void advance();
	};

	extern DeferReleaseQueue g_DeferRelease;
}
