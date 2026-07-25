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

		// AllocShape - allocator-shape category. Renamed from the original allocator
		// design addendum's `MemType` enum (Raw/Typed/Header) specifically to avoid
		// colliding with the substrate MemType<T> tag hierarchy above, which claimed
		// the `MemType` name first in this file. Descriptive metadata only - doesn't
		// change that construction still lives on the Typed tier only.

		enum class AllocShape : std::uint8_t {
		Raw,
		Typed,
		Header
	};

	// ArenaMetadata<Arena> - Tier 0 descriptive metadata. Primary template + macro
	// only live here; specializations live next to each real Arena type (e.g.
	// LinearArena.h), invoked locally via SPEC_ARENA_METADATA. Substrate-blind -
	// carries no MemType<T> field, no cross-check against it.

	template<typename Arena>
	struct ArenaMetadata final {
		constexpr static auto  arenaDesc = "unknown";
		constexpr static short arenaHash = INT16_MAX;   // sentinel = "unspecialized"
		constexpr static auto  arenaNameStr = "unknown";
	};

	// Intentionally NOT #undef'd - must stay visible so every Arena header that
	// includes MemBehavior.h can invoke it locally, next to the class it describes.
#define SPEC_ARENA_METADATA(ArenaType, Description, Hash, NameStr) \
	template<> struct ArenaMetadata<ArenaType> final { \
		constexpr static auto  arenaDesc    = Description; \
		constexpr static short arenaHash    = Hash; \
		constexpr static auto  arenaNameStr = NameStr; \
	};

	// AllocatorMetadata<T, Arena> - Tier 1 descriptive metadata. Same locality
	// principle as ArenaMetadata. No `mechanism` field - explicitly dropped;
	// the original addendum's tooling-only Mechanism tag is not part of this.

	template<typename T, typename Arena>
	struct AllocatorMetadata final {
		constexpr static auto       allocDesc = "unknown";
		constexpr static short      allocHash = INT16_MAX;   // sentinel = "unspecialized"
		constexpr static auto       allocNameStr = "unknown";
		constexpr static AllocShape allocShape = AllocShape::Raw;
	};

	// Intentionally NOT #undef'd, same reasoning as SPEC_ARENA_METADATA.
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

	// Seed list - extend as needed per subsystem. No central registry file requirement;
	// each subsystem may declare its own tag(s) via SPEC_SUBSYSTEM in its own headers.
	SPEC_SUBSYSTEM(SpectraCudaTag, "SpectraCudaBackend", "CUDA backend allocations", 0)
		SPEC_SUBSYSTEM(SpectraFileTag, "SpectraFileSystem", "File I/O staging", 1)
		SPEC_SUBSYSTEM(LumosSceneTag, "LumosScene", "Scene graph allocations", 2)

#undef SPEC_SUBSYSTEM

		// Friendly aliases -- callers spell the real subsystem name, not its tag.
		using SpectraCudaBackend = SpectraCudaTag;
	using SpectraFileSystem = SpectraFileTag;
	using LumosScene = LumosSceneTag;
}
