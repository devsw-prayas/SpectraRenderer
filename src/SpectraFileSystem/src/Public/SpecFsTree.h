#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"
#include "SpecFsPath.h"

#include <SpecCoreThreading.h>
#include <SpectraAllocators.h>
#include <PlatformAtomics.h>

namespace Spectra::FileSystem {
	constexpr uint32_t NODE_NONE = UINT32_MAX;

	struct NodeHandle {
		uint32_t m_Index = NODE_NONE;
		uint32_t m_Generation = 0;
	};

	enum class NodeFlags : uint8_t {
		NONE = 0,
		ALIVE = 1 << 0,
		CASE_COLLISION = 1 << 1,
		FREE = 1 << 2
	};

	struct alignas(32) TreeNode {
		uint32_t m_Parent;
		uint32_t m_NameOffset;
		uint16_t m_NameLength;
		NodeFlags m_Flags;
		uint8_t m_Pad;
		uint32_t m_Hash;  // segment only; edges mix in the parent
		uint32_t m_Generation;
		Platform::Runtime::Atomic::AtomicValue32<uint32_t> m_OpenCount;
		uint32_t m_FirstChild;   // doubles as the free-list link once FREE
		uint32_t m_NextSibling;
	};
	SPEC_FS_STATIC_ASSERT(sizeof(TreeNode) == 32, "TreeNode must be 32 bytes");

	struct EdgeSlot {
		uint32_t m_Node;  // 0 empty (root is never a child), UINT32_MAX tombstone
		uint32_t m_Hash;
	};

	// One per project (or mount). Lives in one BlockedAllocator run: nodes 1/4, names 1/2, edges 1/4.
	struct PathTree {
		char m_Mount[32];
		uint8_t m_MountLength;

		void* m_Block;
		size_t m_BlockBytes;
		size_t m_NumaNode;

		Memory::Allocators::ReclaimableStackArena m_NodeArena;
		Memory::Allocators::ReclaimableStackArena m_NameArena;
		Memory::MemoryHandle m_EdgeMemory;

		TreeNode* m_Nodes;
		uint32_t m_NodeCount;
		uint32_t m_FreeHead;
		char* m_Names;

		EdgeSlot* m_Edges;
		uint32_t m_EdgeCapacity;
		uint32_t m_EdgeMax;
		uint32_t m_EdgeUsed;  // tombstones included

		Core::Threads::ReadWriteLock m_Lock;
	};

	// Failures return false and push an exec (module 0x07).
	class SPEC_FS_RUNTIME_API Trees final {
	public:
		Trees() = delete;

		static constexpr uint32_t DEFAULT_BLOCKS = 16;

		// v_Mount: the mount name the tree's paths carry ("project", "engine"). Blocks must be a multiple of 4.
		SPEC_FS_NODISCARD static bool create(PathTree& r_Tree, std::string_view v_Mount, uint32_t v_Blocks, size_t v_NumaNode) noexcept;
		static void destroy(PathTree& r_Tree) noexcept;

		// Paths are relative or mounted; the mount name is not checked. buildPath writes the tree's own mount name.
		SPEC_FS_NODISCARD static NodeHandle lookup(PathTree& r_Tree, const PathBuffer& r_Path) noexcept;
		SPEC_FS_NODISCARD static bool insert(PathTree& r_Tree, const PathBuffer& r_Path, NodeHandle& r_Out) noexcept;
		// Handles follow the node; missing parents of the new path are created.
		SPEC_FS_NODISCARD static bool rename(PathTree& r_Tree, NodeHandle v_Node, const PathBuffer& r_NewPath) noexcept;
		// Removes the subtree. Pinned nodes stay dead until their last unpin.
		SPEC_FS_NODISCARD static bool remove(PathTree& r_Tree, NodeHandle v_Node) noexcept;

		SPEC_FS_NODISCARD static bool pin(PathTree& r_Tree, NodeHandle v_Node) noexcept;
		static void unpin(PathTree& r_Tree, NodeHandle v_Node) noexcept;

		SPEC_FS_NODISCARD static bool buildPath(PathTree& r_Tree, NodeHandle v_Node, PathBuffer& r_Out) noexcept;
		SPEC_FS_NODISCARD static bool isAlive(PathTree& r_Tree, NodeHandle v_Node) noexcept;
		SPEC_FS_NODISCARD static bool hasCaseCollision(PathTree& r_Tree, NodeHandle v_Node) noexcept;
	};
}
