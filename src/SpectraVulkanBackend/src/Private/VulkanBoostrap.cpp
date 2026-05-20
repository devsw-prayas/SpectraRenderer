#include "SpectraVulkanBackend.h"
#include "VulkanBootstrap.h"
#include "VulkanState.h"
#include "VulkanInternalHelpers.h"


// VulkanBootstrap.cpp
// Spectra Vulkan Backend - Bootstrap & Initialization
//
// Initialization sequence (must follow this exact order):
//   1. initializeVulkan()   - VkInstance, debug messenger, instance extensions
//   2. selectPhysicalDevice() - GPU selection, feature/extension validation,
//                               capability persistence into DeviceProperties
//   3. createLogicalDevice() - VkDevice, queue retrieval, debug naming
//   4. createAllocator()    - VMA allocator, buffer device address + budget
//   5. createCommandPools() - One pool per unique queue family
//
// Shutdown: shutdownVulkan() - destroys in reverse order


namespace Spectra::Vulkan {

	VulkanInstance g_GlobalInstance;
	bool           g_IsInitialized = false;

	namespace {
		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* data,
			void*) {
			SPEC_VK_BK_UNUSED(data);

			if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
				SPEC_VK_BK_ASSERT(false);
			}

			return VK_FALSE;
		}
	}

	// TODO Upgrade the array to support StormSTL VA Heap
	InstanceHandle VulkanBootstrap::initializeVulkan(const Utils::InitDesc& ro_Desc) {
		Internal::VulkanRegistry::initRegistry();

		// Build the info
		auto info = Internal::vkInit<VkApplicationInfo>();
		info.apiVersion = VK_API_VERSION_1_3;
		info.pEngineName = "Spectra";
		info.pApplicationName = ro_Desc.m_ApplicationName;
		info.engineVersion = 1;
		info.applicationVersion = 1;
		info.pNext = nullptr;

		g_GlobalInstance.m_AppInfo = info;

		auto createInstance = Internal::vkInit<VkInstanceCreateInfo>();
		createInstance.pApplicationInfo = &g_GlobalInstance.m_AppInfo;

		uint32_t extCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		VkExtensionProperties props[Utils::MAX_INSTANCE_EXT];
		SPEC_VK_BK_ASSERT(extCount <= Utils::MAX_INSTANCE_EXT);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, props);

		g_GlobalInstance.s_ExtensionCount = 0;
		for (uint32_t i = 0; i < Internal::VulkanRegistry::INSTANCE_EXTENSIONS; ++i) {
			const auto& ext = Internal::VulkanRegistry::s_Extensions[i];
			bool found = false;
			for (uint32_t j = 0; j < extCount; ++j) {
				if (strcmp(props[j].extensionName, ext.m_Extension) == 0) {
					found = true;
					break;
				}
			}

			// Debug extension gating
			if (ext.m_ExtensionName == Utils::VulkanExtensions::VK_DEBUG_UTILS &&
				!ro_Desc.m_EnableValidation) {
				continue;
			}

			if (ext.m_Requirement == Utils::ExtensionRequirement::REQUIRED && !found) {
				SPEC_VK_BK_ASSERT(false);
			}

			if (found) {
				g_GlobalInstance.m_EnabledExtensions[g_GlobalInstance.s_ExtensionCount++] = ext;
			}
		}

		const char* enabledExts[Utils::VK_SUPPORTED_EXT_COUNT];

		for (uint32_t i = 0; i < g_GlobalInstance.s_ExtensionCount; ++i) {
			enabledExts[i] = g_GlobalInstance.m_EnabledExtensions[i].m_Extension;
		}

		createInstance.enabledExtensionCount = g_GlobalInstance.s_ExtensionCount;
		createInstance.ppEnabledExtensionNames = enabledExts;

		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		VkLayerProperties layerProps[32];
		SPEC_VK_BK_ASSERT(layerCount <= 32);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProps);

		auto isLayerAvailable = [&](const char* name) {
			for (uint32_t i = 0; i < layerCount; ++i) {
				if (strcmp(layerProps[i].layerName, name) == 0)
					return true;
			}
			return false;
			};

		const char* layers[1];
		g_GlobalInstance.s_LayerCount = 0;

		if (ro_Desc.m_EnableValidation) {
			const char* validationLayer = VK_LAYERS_KHRONOS_VALIDATION;

			bool found = isLayerAvailable(validationLayer);
			SPEC_VK_BK_ASSERT(found);

			layers[0] = validationLayer;
			g_GlobalInstance.s_LayerCount = 1;

			g_GlobalInstance.m_EnableLayers[0] =
			{ validationLayer, Utils::VulkanLayers::VK_VALIDATION };
		}

		createInstance.enabledLayerCount = g_GlobalInstance.s_LayerCount;
		createInstance.ppEnabledLayerNames = g_GlobalInstance.s_LayerCount ? layers : nullptr;

		auto debugInfo = Internal::vkInit<VkDebugUtilsMessengerCreateInfoEXT>();
		debugInfo.pNext = nullptr;
		if (ro_Desc.m_EnableValidation) {
			debugInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			debugInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

			debugInfo.pfnUserCallback = debugCallback;

			createInstance.pNext = &debugInfo;
		} else {
			createInstance.pNext = nullptr;
		}

		VkResult res = vkCreateInstance(&createInstance, nullptr, &g_GlobalInstance.m_GlobalInstance);
		SPEC_VK_BK_ASSERT(res == VK_SUCCESS);

		if (ro_Desc.m_EnableValidation) {
			auto raw = vkGetInstanceProcAddr(
				g_GlobalInstance.m_GlobalInstance,
				"vkCreateDebugUtilsMessengerEXT");

			SPEC_VK_BK_ASSERT(raw != nullptr);

			auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(raw);

			SPEC_VK_BK_ASSERT(fn);

			VkResult dbgRes = fn(
				g_GlobalInstance.m_GlobalInstance,
				&debugInfo,
				nullptr,
				&g_GlobalInstance.m_GlobalDebugMessager);

			SPEC_VK_BK_ASSERT(dbgRes == VK_SUCCESS);
		}

		g_IsInitialized = true;
		return &g_GlobalInstance;
	}

	void VulkanBootstrap::selectPhysicalDevice() {
		vkEnumeratePhysicalDevices(g_GlobalInstance.m_GlobalInstance, &g_GlobalInstance.s_DeviceCount, nullptr);
		VkPhysicalDevice devices[VulkanInstance::MAX_DEVICES] = {};
		vkEnumeratePhysicalDevices(g_GlobalInstance.m_GlobalInstance, &g_GlobalInstance.s_DeviceCount, devices);

		SPEC_VK_BK_ASSERT(g_GlobalInstance.s_DeviceCount > 0);
		SPEC_VK_BK_ASSERT(g_GlobalInstance.s_DeviceCount <= VulkanInstance::MAX_DEVICES);

		uint32_t bestScore = 0;

		for (uint32_t d = 0; d < g_GlobalInstance.s_DeviceCount; ++d) {
			auto device = devices[d];

			// --- Properties chain ---
			auto accelProps = Internal::vkInit<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
			auto rtProps = Internal::vkInit<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
			auto props = Internal::vkInit<VkPhysicalDeviceProperties2>();
			props.pNext = &accelProps;
			accelProps.pNext = &rtProps;
			rtProps.pNext = nullptr;
			vkGetPhysicalDeviceProperties2(device, &props);

			// --- Features chain ---
			auto features2 = Internal::vkInit<VkPhysicalDeviceFeatures2>();
			auto vk12 = Internal::vkInit<VkPhysicalDeviceVulkan12Features>();
			auto vk13 = Internal::vkInit<VkPhysicalDeviceVulkan13Features>();
			auto accel = Internal::vkInit<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
			auto rtPipeline = Internal::vkInit<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
			features2.pNext = &vk12;
			vk12.pNext = &vk13;
			vk13.pNext = &accel;
			accel.pNext = &rtPipeline;
			rtPipeline.pNext = nullptr;
			vkGetPhysicalDeviceFeatures2(device, &features2);

			// --- Feature gates ---
			if (!vk12.bufferDeviceAddress)    continue;
			if (!vk12.descriptorIndexing)     continue;
			if (!vk12.timelineSemaphore)      continue;
			if (!vk13.dynamicRendering)       continue;
			if (!vk13.synchronization2)       continue;
			if (!vk13.maintenance4)           continue;
			if (!accel.accelerationStructure) continue;
			if (!rtPipeline.rayTracingPipeline) continue;

			// --- Device extension validation ---
			uint32_t devExtCount = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &devExtCount, nullptr);
			VkExtensionProperties devExtProps[512] = {};
			SPEC_VK_BK_ASSERT(devExtCount <= 512);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &devExtCount, devExtProps);

			Utils::VkExtension enabledDevExts[Utils::VK_SUPPORTED_EXT_COUNT] = {};
			uint32_t enabledDevExtCount = 0;
			bool requiredMissing = false;

			// device + RT - optional extensions are indices 4-26
			for (uint32_t i = Internal::VulkanRegistry::INSTANCE_EXTENSIONS;
				 i < Utils::VK_SUPPORTED_EXT_COUNT; ++i) {
				const auto& ext = Internal::VulkanRegistry::s_Extensions[i];
				bool found = false;
				for (uint32_t j = 0; j < devExtCount; ++j) {
					if (strcmp(devExtProps[j].extensionName, ext.m_Extension) == 0) {
						found = true;
						break;
					}
				}
				if (!found && ext.m_Requirement == Utils::ExtensionRequirement::REQUIRED) {
					requiredMissing = true;
					break;
				}
				if (found) {
					enabledDevExts[enabledDevExtCount++] = ext;
				}
			}
			if (requiredMissing) continue;

			// --- Queue families ---
			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueCount, nullptr);
			VkQueueFamilyProperties2 queueProps[VulkanInstance::MAX_QUEUE_FAMILIES] = {};
			SPEC_VK_BK_ASSERT(queueCount <= VulkanInstance::MAX_QUEUE_FAMILIES);
			for (uint32_t i = 0; i < queueCount; ++i)
				queueProps[i] = Internal::vkInit<VkQueueFamilyProperties2>();
			vkGetPhysicalDeviceQueueFamilyProperties2(device, &queueCount, queueProps);

			uint32_t graphicsFamily = UINT32_MAX, computeFamily = UINT32_MAX, transferFamily = UINT32_MAX;
			for (uint32_t i = 0; i < queueCount; ++i) {
				auto flags = queueProps[i].queueFamilyProperties.queueFlags;
				if ((flags & VK_QUEUE_GRAPHICS_BIT) && graphicsFamily == UINT32_MAX)
					graphicsFamily = i;
				if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && computeFamily == UINT32_MAX)
					computeFamily = i;
				if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) && transferFamily == UINT32_MAX)
					transferFamily = i;
			}
			if (graphicsFamily == UINT32_MAX) continue;

			bool dedicatedCompute = computeFamily != UINT32_MAX;
			bool dedicatedTransfer = transferFamily != UINT32_MAX;
			if (!dedicatedCompute)  computeFamily = graphicsFamily;
			if (!dedicatedTransfer) transferFamily = graphicsFamily;

			// --- Memory properties ---
			auto memProps = Internal::vkInit<VkPhysicalDeviceMemoryProperties2>();
			vkGetPhysicalDeviceMemoryProperties2(device, &memProps);

			uint64_t vramBytes = 0;
			for (uint32_t i = 0; i < memProps.memoryProperties.memoryHeapCount; ++i) {
				auto& heap = memProps.memoryProperties.memoryHeaps[i];
				if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
					vramBytes += heap.size;
			}
			uint32_t vramMB = static_cast<uint32_t>(vramBytes / (1024ull * 1024));

			// Scoring weights:
			//   +1000  discrete GPU
			//   +100   per GB VRAM
			//   +10    per ray recursion depth level
			//   +1     per million max primitives
			//   +50    dedicated async compute family
			//   +25    dedicated DMA transfer family
			// --- Scoring ---
			uint32_t score = 0;
			if (props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
			score += (vramMB / 1024) * 100;
			score += rtProps.maxRayRecursionDepth * 10;
			score += static_cast<uint32_t>(accelProps.maxPrimitiveCount / 1000000);
			if (dedicatedCompute)  score += 50;
			if (dedicatedTransfer) score += 25;

			if (score > bestScore) {
				bestScore = score;

				// --- Persist capabilities ---
				DeviceProperties& dp = g_GlobalInstance.m_DeviceProps;

				dp.m_Properties = props;
				dp.m_RTProperties = rtProps;
				dp.m_AccelProperties = accelProps;

				dp.m_Features = features2;
				dp.m_Vk12Features = vk12;
				dp.m_Vk13Features = vk13;
				dp.m_AccelFeatures = accel;
				dp.m_RTPipelineFeatures = rtPipeline;

				dp.m_MemoryProperties = memProps;

				// clear pNext on stored structs - they'll be re-linked on demand
				dp.m_Properties.pNext = nullptr;
				dp.m_Features.pNext = nullptr;
				dp.m_MemoryProperties.pNext = nullptr;

				for (uint32_t i = 0; i < enabledDevExtCount; ++i)
					dp.m_EnabledExtensions[i] = enabledDevExts[i];
				dp.s_ExtensionCount = enabledDevExtCount;

				strncpy_s(dp.m_DeviceName, props.properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1);
				dp.m_DeviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1] = '\0';
				dp.m_VendorID = props.properties.vendorID;
				dp.m_DeviceID = props.properties.deviceID;

				// --- Finalize handles ---
				g_GlobalInstance.m_Device.m_DeviceHandle = device;
				g_GlobalInstance.m_Device.m_PropertiesHandle = &g_GlobalInstance.m_DeviceProps;
				g_GlobalInstance.m_GraphicsFamily = graphicsFamily;
				g_GlobalInstance.m_ComputeFamily = computeFamily;
				g_GlobalInstance.m_TransferFamily = transferFamily;
			}
		}

		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_Device.m_DeviceHandle != nullptr);
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_Device.m_PropertiesHandle != nullptr);
	}

	void VulkanBootstrap::createLogicalDevice() {
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_Device.m_DeviceHandle != nullptr);

		VkPhysicalDevice physDevice = static_cast<VkPhysicalDevice>(g_GlobalInstance.m_Device.m_DeviceHandle);
		DeviceProperties& dp = g_GlobalInstance.m_DeviceProps;

		// --- Deduplicate queue families ---
		uint32_t uniqueFamilies[3] = {
			g_GlobalInstance.m_GraphicsFamily,
			g_GlobalInstance.m_ComputeFamily,
			g_GlobalInstance.m_TransferFamily
		};

		VkDeviceQueueCreateInfo queueInfos[3] = {};
		uint32_t queueInfoCount = 0;
		float priority = 1.0f;

		for (uint32_t i = 0; i < 3; ++i) {
			bool duplicate = false;
			for (uint32_t j = 0; j < i; ++j) {
				if (uniqueFamilies[i] == uniqueFamilies[j]) {
					duplicate = true;
					break;
				}
			}
			if (duplicate) continue;

			auto qi = Internal::vkInit<VkDeviceQueueCreateInfo>();
			qi.queueFamilyIndex = uniqueFamilies[i];
			qi.queueCount = 1;
			qi.pQueuePriorities = &priority;
			queueInfos[queueInfoCount++] = qi;
		}

		// Re-link the feature pNext chain for VkDeviceCreateInfo.
		// Stored structs have pNext cleared after selectPhysicalDevice() to avoid
		// dangling pointers. Must be re-linked here before passing to vkCreateDevice.
		dp.m_Features.pNext = &dp.m_Vk12Features;
		dp.m_Vk12Features.pNext = &dp.m_Vk13Features;
		dp.m_Vk13Features.pNext = &dp.m_AccelFeatures;
		dp.m_AccelFeatures.pNext = &dp.m_RTPipelineFeatures;
		dp.m_RTPipelineFeatures.pNext = nullptr;

		// --- Extension names ---
		const char* extNames[Utils::VK_SUPPORTED_EXT_COUNT] = {};
		for (uint32_t i = 0; i < dp.s_ExtensionCount; ++i)
			extNames[i] = dp.m_EnabledExtensions[i].m_Extension;

		// --- Device create ---
		auto deviceInfo = Internal::vkInit<VkDeviceCreateInfo>();
		deviceInfo.pNext = &dp.m_Features;
		deviceInfo.queueCreateInfoCount = queueInfoCount;
		deviceInfo.pQueueCreateInfos = queueInfos;
		deviceInfo.enabledExtensionCount = dp.s_ExtensionCount;
		deviceInfo.ppEnabledExtensionNames = extNames;
		deviceInfo.pEnabledFeatures = nullptr;

		VkResult res = vkCreateDevice(physDevice, &deviceInfo, nullptr, &g_GlobalInstance.m_LogicalDevice.m_Device);
		SPEC_VK_BK_ASSERT(res == VK_SUCCESS);

		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;

		// --- Retrieve queues ---
		vkGetDeviceQueue(dev, g_GlobalInstance.m_GraphicsFamily, 0, &g_GlobalInstance.m_LogicalDevice.m_GraphicsQueue);
		vkGetDeviceQueue(dev, g_GlobalInstance.m_ComputeFamily, 0, &g_GlobalInstance.m_LogicalDevice.m_ComputeQueue);
		vkGetDeviceQueue(dev, g_GlobalInstance.m_TransferFamily, 0, &g_GlobalInstance.m_LogicalDevice.m_TransferQueue);

		// --- Debug naming ---
#if SPEC_VK_BK_BUILD_DEBUG
		auto vkSetDebugName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
			vkGetDeviceProcAddr(dev, "vkSetDebugUtilsObjectNameEXT"));

		if (vkSetDebugName) {
			auto nameObj = [&](VkObjectType type, uint64_t handle, const char* name) {
				auto info = Internal::vkInit<VkDebugUtilsObjectNameInfoEXT>();
				info.objectType = type;
				info.objectHandle = handle;
				info.pObjectName = name;
				vkSetDebugName(dev, &info);
				};

			nameObj(VK_OBJECT_TYPE_QUEUE, reinterpret_cast<uint64_t>(g_GlobalInstance.m_LogicalDevice.m_GraphicsQueue), "Queue::Graphics");
			nameObj(VK_OBJECT_TYPE_QUEUE, reinterpret_cast<uint64_t>(g_GlobalInstance.m_LogicalDevice.m_ComputeQueue), "Queue::Compute");
			nameObj(VK_OBJECT_TYPE_QUEUE, reinterpret_cast<uint64_t>(g_GlobalInstance.m_LogicalDevice.m_TransferQueue), "Queue::Transfer");
		}
#endif

		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_LogicalDevice.m_Device != VK_NULL_HANDLE);
	}

	void VulkanBootstrap::createAllocator() {
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_Device.m_DeviceHandle != nullptr);
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_LogicalDevice.m_Device != VK_NULL_HANDLE);

		VkPhysicalDevice physDevice = static_cast<VkPhysicalDevice>(g_GlobalInstance.m_Device.m_DeviceHandle);
		DeviceProperties& dp = g_GlobalInstance.m_DeviceProps;

		// --- Vulkan function pointers --- required for Vulkan 1.3
		VmaVulkanFunctions vmaFuncs{};
		vmaFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vmaFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		// --- Allocator flags ---
		VmaAllocatorCreateFlags flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		// VMA_DYNAMIC_VULKAN_FUNCTIONS is implied by passing pVulkanFunctions with
		// only vkGetInstanceProcAddr and vkGetDeviceProcAddr filled. VMA resolves
		// all other function pointers itself at runtime. Required for Vulkan 1.3.
		for (uint32_t i = 0; i < dp.s_ExtensionCount; ++i) {
			if (dp.m_EnabledExtensions[i].m_ExtensionName == Utils::VulkanExtensions::VK_MEMORY_BUDGET) {
				flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
				break;
			}
		}

		VmaAllocatorCreateInfo allocInfo{};
		allocInfo.flags = flags;
		allocInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocInfo.instance = g_GlobalInstance.m_GlobalInstance;
		allocInfo.physicalDevice = physDevice;
		allocInfo.device = g_GlobalInstance.m_LogicalDevice.m_Device;
		allocInfo.pVulkanFunctions = &vmaFuncs;

		VkResult res = vmaCreateAllocator(&allocInfo, &g_GlobalInstance.m_Allocator.m_Allocator);
		SPEC_VK_BK_ASSERT(res == VK_SUCCESS);
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_Allocator.isValid());
	}

	void VulkanBootstrap::createCommandPools() {
		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_LogicalDevice.m_Device != VK_NULL_HANDLE);

		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;

		auto createPool = [&](uint32_t familyIndex) -> VkCommandPool {
			auto poolInfo = Internal::vkInit<VkCommandPoolCreateInfo>();
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = familyIndex;

			VkCommandPool pool = VK_NULL_HANDLE;
			VkResult res = vkCreateCommandPool(dev, &poolInfo, nullptr, &pool);
			SPEC_VK_BK_ASSERT(res == VK_SUCCESS);
			return pool;
			};

		g_GlobalInstance.m_CommandPools.m_GraphicsPool = createPool(g_GlobalInstance.m_GraphicsFamily);

		// deduplicate - only create separate pools for distinct families
		if (g_GlobalInstance.m_ComputeFamily != g_GlobalInstance.m_GraphicsFamily)
			g_GlobalInstance.m_CommandPools.m_ComputePool = createPool(g_GlobalInstance.m_ComputeFamily);
		else
			g_GlobalInstance.m_CommandPools.m_ComputePool = g_GlobalInstance.m_CommandPools.m_GraphicsPool;

		if (g_GlobalInstance.m_TransferFamily != g_GlobalInstance.m_GraphicsFamily &&
			g_GlobalInstance.m_TransferFamily != g_GlobalInstance.m_ComputeFamily)
			g_GlobalInstance.m_CommandPools.m_TransferPool = createPool(g_GlobalInstance.m_TransferFamily);
		else
			g_GlobalInstance.m_CommandPools.m_TransferPool = g_GlobalInstance.m_CommandPools.m_GraphicsPool;

		SPEC_VK_BK_ASSERT(g_GlobalInstance.m_CommandPools.isValid());

#if SPEC_VK_BK_BUILD_DEBUG
		auto vkSetDebugName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
			vkGetDeviceProcAddr(dev, "vkSetDebugUtilsObjectNameEXT"));

		if (vkSetDebugName) {
			auto nameObj = [&](VkObjectType type, uint64_t handle, const char* name) {
				auto info = Internal::vkInit<VkDebugUtilsObjectNameInfoEXT>();
				info.objectType = type;
				info.objectHandle = handle;
				info.pObjectName = name;
				vkSetDebugName(dev, &info);
				};

			nameObj(VK_OBJECT_TYPE_COMMAND_POOL, reinterpret_cast<uint64_t>(g_GlobalInstance.m_CommandPools.m_GraphicsPool), "CommandPool::Graphics");
			if (g_GlobalInstance.m_CommandPools.m_ComputePool != g_GlobalInstance.m_CommandPools.m_GraphicsPool)
				nameObj(VK_OBJECT_TYPE_COMMAND_POOL, reinterpret_cast<uint64_t>(g_GlobalInstance.m_CommandPools.m_ComputePool), "CommandPool::Compute");
			if (g_GlobalInstance.m_CommandPools.m_TransferPool != g_GlobalInstance.m_CommandPools.m_GraphicsPool)
				nameObj(VK_OBJECT_TYPE_COMMAND_POOL, reinterpret_cast<uint64_t>(g_GlobalInstance.m_CommandPools.m_TransferPool), "CommandPool::Transfer");
		}
#endif
	}

	InstanceHandle VulkanBootstrap::getVulkanInstance() {
		return &g_GlobalInstance;
	}

	void VulkanBootstrap::shutdownVulkan(InstanceHandle p_Handle) {
		SPEC_VK_BK_ASSERT(p_Handle != nullptr);

		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;

		// Destroy in strict reverse-init order:
		//   CommandPools -> VMA -> LogicalDevice -> DebugMessenger -> Instance
		// Aliased pool handles (compute/transfer sharing graphics pool) must not be
		// destroyed twice - guard via handle comparison before each destroy call.
		if (g_GlobalInstance.m_CommandPools.m_TransferPool != VK_NULL_HANDLE &&
			g_GlobalInstance.m_CommandPools.m_TransferPool != g_GlobalInstance.m_CommandPools.m_GraphicsPool &&
			g_GlobalInstance.m_CommandPools.m_TransferPool != g_GlobalInstance.m_CommandPools.m_ComputePool)
			vkDestroyCommandPool(dev, g_GlobalInstance.m_CommandPools.m_TransferPool, nullptr);

		if (g_GlobalInstance.m_CommandPools.m_ComputePool != VK_NULL_HANDLE &&
			g_GlobalInstance.m_CommandPools.m_ComputePool != g_GlobalInstance.m_CommandPools.m_GraphicsPool)
			vkDestroyCommandPool(dev, g_GlobalInstance.m_CommandPools.m_ComputePool, nullptr);

		if (g_GlobalInstance.m_CommandPools.m_GraphicsPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(dev, g_GlobalInstance.m_CommandPools.m_GraphicsPool, nullptr);

		g_GlobalInstance.m_CommandPools.m_GraphicsPool = VK_NULL_HANDLE;
		g_GlobalInstance.m_CommandPools.m_ComputePool = VK_NULL_HANDLE;
		g_GlobalInstance.m_CommandPools.m_TransferPool = VK_NULL_HANDLE;

		// --- VMA ---
		if (g_GlobalInstance.m_Allocator.isValid()) {
			vmaDestroyAllocator(g_GlobalInstance.m_Allocator.m_Allocator);
			g_GlobalInstance.m_Allocator.m_Allocator = VK_NULL_HANDLE;
		}

		// --- Logical Device ---
		if (dev != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(dev);
			vkDestroyDevice(dev, nullptr);
			g_GlobalInstance.m_LogicalDevice.m_Device = VK_NULL_HANDLE;
			g_GlobalInstance.m_LogicalDevice.m_GraphicsQueue = VK_NULL_HANDLE;
			g_GlobalInstance.m_LogicalDevice.m_ComputeQueue = VK_NULL_HANDLE;
			g_GlobalInstance.m_LogicalDevice.m_TransferQueue = VK_NULL_HANDLE;
		}

		// --- Debug Messenger ---
#if SPEC_VK_BK_BUILD_DEBUG
		if (g_GlobalInstance.m_GlobalDebugMessager != VK_NULL_HANDLE) {
			auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(g_GlobalInstance.m_GlobalInstance, "vkDestroyDebugUtilsMessengerEXT"));
			if (fn)
				fn(g_GlobalInstance.m_GlobalInstance, g_GlobalInstance.m_GlobalDebugMessager, nullptr);
			g_GlobalInstance.m_GlobalDebugMessager = VK_NULL_HANDLE;
		}
#endif

		// --- Instance ---
		if (g_GlobalInstance.m_GlobalInstance != VK_NULL_HANDLE) {
			vkDestroyInstance(g_GlobalInstance.m_GlobalInstance, nullptr);
			g_GlobalInstance.m_GlobalInstance = VK_NULL_HANDLE;
		}

		g_IsInitialized = false;
	}
}