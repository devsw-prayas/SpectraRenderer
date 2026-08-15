#pragma once
#include <cstdint>

namespace Spectra::Memory::Allocators {
	template<typename T>
	struct MemType final {
		constexpr static auto name = "unknown";
		constexpr static auto desc = "unknown underlying memory type";
		constexpr static short id = INT16_MAX;
	};

	template<typename T>
	constexpr const char* getMemTypeName() noexcept {
		return MemType<T>::name;
	}

	template<typename T>
	constexpr const char* getMemTypeDesc() noexcept {
		return MemType<T>::desc;
	}

	template<typename T>
	constexpr short getMemTypeId() noexcept {
		return MemType<T>::id;
	}

#ifndef SPEC_MEM_TYPE
#define SPEC_MEM_TYPE(TagName, DisplayName, Description, Code) \
	struct TagName; \
	template<> struct MemType<TagName> final { \
		constexpr static auto name = DisplayName; \
		constexpr static auto desc = Description; \
		constexpr static short id  = Code; \
	};
#endif

	SPEC_MEM_TYPE(CPU, "CPU", "Host (CPU) paged/committed VA", 0)
	SPEC_MEM_TYPE(GPU, "GPU", "GPU-mapped device memory", 1)
	SPEC_MEM_TYPE(MemMapped, "MemMapped", "Memory-mapped file", 2)

#undef SPEC_MEM_TYPE
	// Allocator construction shape.

	enum class AllocShape : std::uint8_t {
		Raw,
		Typed,
		Header
	};
	// Tier 0 metadata for an arena type.
	template<typename Arena>
	struct ArenaMetadata final {
		constexpr static auto  arenaDesc = "unknown";
		constexpr static short arenaHash = INT16_MAX;   // sentinel = "unspecialized"
		constexpr static auto  arenaNameStr = "unknown";
	};
	// Kept visible for use by individual arena headers.
#define SPEC_ARENA_METADATA(ArenaType, Description, Hash, NameStr) \
	template<> struct ArenaMetadata<ArenaType> final { \
		constexpr static auto  arenaDesc    = Description; \
		constexpr static short arenaHash    = Hash; \
		constexpr static auto  arenaNameStr = NameStr; \
	};
	// Tier 1 metadata for an allocator and its arena.
	template<typename T, typename Arena>
	struct AllocatorMetadata final {
		constexpr static auto       allocDesc = "unknown";
		constexpr static short      allocHash = INT16_MAX;   // sentinel = "unspecialized"
		constexpr static auto       allocNameStr = "unknown";
		constexpr static AllocShape allocShape = AllocShape::Raw;
	};
	// Kept visible for use by allocator headers.
#define SPEC_ALLOCATOR_METADATA(TType, ArenaType, Description, Hash, NameStr, Shape) \
	template<> struct AllocatorMetadata<TType, ArenaType> final { \
		constexpr static auto       allocDesc    = Description; \
		constexpr static short      allocHash    = Hash; \
		constexpr static auto       allocNameStr = NameStr; \
		constexpr static AllocShape allocShape   = Shape; \
	};
}

namespace Spectra::Memory {
	template<typename U>
	struct Subsystem final {
		constexpr static auto subsystemName = "unknown";
		constexpr static auto subsystemDesc = "unknown subsystem type";
		constexpr static short subsystemId = INT16_MAX;   // sentinel = "unspecialized"
	};

	template<typename U>
	constexpr const char* getSubsystemName() noexcept {
		return Subsystem<U>::subsystemName;
	}

	template<typename U>
	constexpr const char* getSubsystemDesc() noexcept {
		return Subsystem<U>::subsystemDesc;
	}

	template<typename U>
	constexpr short getSubsystemId() noexcept {
		return Subsystem<U>::subsystemId;
	}

#ifndef SPEC_SUBSYSTEM
#define SPEC_SUBSYSTEM(TagName, DisplayName, Description, Code) \
	struct TagName; \
	template<> struct Subsystem<TagName> final { \
		constexpr static auto subsystemName = DisplayName; \
		constexpr static auto subsystemDesc = Description; \
		constexpr static short subsystemId  = Code; \
	};
#endif
	// Built-in subsystem tags.
	SPEC_SUBSYSTEM(SpectraCudaTag, "SpectraCudaBackend", "CUDA backend allocations", 0)
	SPEC_SUBSYSTEM(SpectraFileTag, "SpectraFileSystem", "File I/O staging", 1)
	SPEC_SUBSYSTEM(LumosSceneTag, "LumosScene", "Scene graph allocations", 2)

#undef SPEC_SUBSYSTEM
	// Public subsystem aliases.
	using SpectraCudaBackend = SpectraCudaTag;
	using SpectraFileSystem = SpectraFileTag;
	using LumosScene = LumosSceneTag;
}
