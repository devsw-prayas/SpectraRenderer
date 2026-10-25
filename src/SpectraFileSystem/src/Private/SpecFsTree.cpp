#include "SpecFsTree.h"
#include "SpecFsDiagnostic.h"

#include <SpecMemRegistry.h>
#include <VAUtils.h>

namespace Spectra::FileSystem {
	namespace {
		using Memory::MemoryHandle;
		using Platform::Runtime::Intrinsic::MemoryOrder;

		constexpr uint32_t EDGE_EMPTY = 0;
		constexpr uint32_t EDGE_TOMB = UINT32_MAX;
		constexpr uint32_t INITIAL_EDGES = 4096;
		constexpr uint32_t MAX_SEGMENTS = PATH_CAPACITY / 2 + 1;

		bool has(NodeFlags v_Flags, NodeFlags v_Bit) noexcept { return (uint8_t(v_Flags) & uint8_t(v_Bit)) != 0; }
		void set(NodeFlags& r_Flags, NodeFlags v_Bit) noexcept { r_Flags = NodeFlags(uint8_t(r_Flags) | uint8_t(v_Bit)); }
		void clear(NodeFlags& r_Flags, NodeFlags v_Bit) noexcept { r_Flags = NodeFlags(uint8_t(r_Flags) & ~uint8_t(v_Bit)); }

		uint32_t hashSegment(std::string_view v_Segment) noexcept {
			uint32_t hash = 2166136261u;
			for (const char c : v_Segment) hash = (hash ^ uint8_t(c)) * 16777619u;
			return hash;
		}

		uint32_t edgeHash(uint32_t v_Parent, uint32_t v_SegmentHash) noexcept {
			uint32_t hash = v_SegmentHash ^ (v_Parent * 0x9E3779B1u);
			hash ^= hash >> 16;
			hash *= 0x85EBCA6Bu;
			return hash ^ (hash >> 13);
		}

		std::string_view nameOf(const PathTree& r_Tree, const TreeNode& r_Node) noexcept {
			return { r_Tree.m_Names + r_Node.m_NameOffset, r_Node.m_NameLength };
		}

		bool equalsFolded(std::string_view v_A, std::string_view v_B) noexcept {
			if (v_A.size() != v_B.size()) return false;
			for (size_t i = 0; i < v_A.size(); ++i) {
				char a = v_A[i], b = v_B[i];
				if (a >= 'A' && a <= 'Z') a = char(a + 32);
				if (b >= 'A' && b <= 'Z') b = char(b + 32);
				if (a != b) return false;
			}
			return true;
		}

		bool validHandle(const PathTree& r_Tree, NodeHandle v_Node) noexcept {
			if (v_Node.m_Index >= r_Tree.m_NodeCount) return false;
			const TreeNode& node = r_Tree.m_Nodes[v_Node.m_Index];
			return has(node.m_Flags, NodeFlags::ALIVE) && node.m_Generation == v_Node.m_Generation;
		}

		// The caller picked the tree, so the mount name is not checked: "project:/" lands on whichever project tree.
		bool onThisMount(const PathTree&, const PathBuffer& r_Path) noexcept {
			return r_Path.m_Kind == PathKind::RELATIVE || r_Path.m_Kind == PathKind::EMPTY || r_Path.m_Kind == PathKind::MOUNTED;
		}

		uint32_t findChild(const PathTree& r_Tree, uint32_t v_Parent, std::string_view v_Segment, uint32_t v_SegmentHash) noexcept {
			const uint32_t mask = r_Tree.m_EdgeCapacity - 1;
			const uint32_t hash = edgeHash(v_Parent, v_SegmentHash);
			for (uint32_t slot = hash & mask, probes = 0; probes < r_Tree.m_EdgeCapacity; slot = (slot + 1) & mask, ++probes) {
				const EdgeSlot& edge = r_Tree.m_Edges[slot];
				if (edge.m_Node == EDGE_EMPTY) return NODE_NONE;
				if (edge.m_Node == EDGE_TOMB || edge.m_Hash != hash || edge.m_Node >= r_Tree.m_NodeCount) continue;
				const TreeNode& node = r_Tree.m_Nodes[edge.m_Node];
				if (node.m_Parent == v_Parent && nameOf(r_Tree, node) == v_Segment) return edge.m_Node;
			}
			return NODE_NONE;
		}

		void placeEdge(PathTree& r_Tree, uint32_t v_Node) noexcept {
			const TreeNode& node = r_Tree.m_Nodes[v_Node];
			const uint32_t mask = r_Tree.m_EdgeCapacity - 1;
			const uint32_t hash = edgeHash(node.m_Parent, node.m_Hash);
			uint32_t slot = hash & mask;
			while (r_Tree.m_Edges[slot].m_Node != EDGE_EMPTY && r_Tree.m_Edges[slot].m_Node != EDGE_TOMB) slot = (slot + 1) & mask;
			if (r_Tree.m_Edges[slot].m_Node == EDGE_EMPTY) ++r_Tree.m_EdgeUsed;
			r_Tree.m_Edges[slot] = { v_Node, hash };
		}

		void dropEdge(PathTree& r_Tree, uint32_t v_Node) noexcept {
			const TreeNode& node = r_Tree.m_Nodes[v_Node];
			const uint32_t mask = r_Tree.m_EdgeCapacity - 1;
			for (uint32_t slot = edgeHash(node.m_Parent, node.m_Hash) & mask; r_Tree.m_Edges[slot].m_Node != EDGE_EMPTY; slot = (slot + 1) & mask) {
				if (r_Tree.m_Edges[slot].m_Node == v_Node) {
					r_Tree.m_Edges[slot].m_Node = EDGE_TOMB;
					return;
				}
			}
		}

		// Edges are derivable from the nodes, so growing is a rebuild, never a copy.
		bool rebuildEdges(PathTree& r_Tree, uint32_t v_Capacity) noexcept {
			if (v_Capacity > r_Tree.m_EdgeMax) return false;
			if (!Memory::commitPageIfNeeded(r_Tree.m_EdgeMemory, size_t(v_Capacity) * sizeof(EdgeSlot) - 1)) return false;
			std::memset(r_Tree.m_Edges, 0, size_t(v_Capacity) * sizeof(EdgeSlot));
			r_Tree.m_EdgeCapacity = v_Capacity;
			r_Tree.m_EdgeUsed = 0;
			for (uint32_t node = 1; node < r_Tree.m_NodeCount; ++node)
				if (has(r_Tree.m_Nodes[node].m_Flags, NodeFlags::ALIVE)) placeEdge(r_Tree, node);
			return true;
		}

		bool reserveEdge(PathTree& r_Tree) noexcept {
			if ((r_Tree.m_EdgeUsed + 1) * 2 <= r_Tree.m_EdgeCapacity) return true;
			uint32_t alive = 0;
			for (uint32_t node = 1; node < r_Tree.m_NodeCount; ++node) alive += has(r_Tree.m_Nodes[node].m_Flags, NodeFlags::ALIVE);
			// Mostly tombstones: rebuild in place instead of doubling.
			const uint32_t capacity = (alive + 1) * 4 <= r_Tree.m_EdgeCapacity ? r_Tree.m_EdgeCapacity : r_Tree.m_EdgeCapacity * 2;
			return rebuildEdges(r_Tree, capacity);  // exec TreeFull when it can't grow
		}

		void linkChild(PathTree& r_Tree, uint32_t v_Parent, uint32_t v_Node) noexcept {
			TreeNode& node = r_Tree.m_Nodes[v_Node];
			TreeNode& parent = r_Tree.m_Nodes[v_Parent];
			const std::string_view name = nameOf(r_Tree, node);
			for (uint32_t sibling = parent.m_FirstChild; sibling != NODE_NONE; sibling = r_Tree.m_Nodes[sibling].m_NextSibling) {
				if (equalsFolded(nameOf(r_Tree, r_Tree.m_Nodes[sibling]), name)) {
					set(r_Tree.m_Nodes[sibling].m_Flags, NodeFlags::CASE_COLLISION);
					set(node.m_Flags, NodeFlags::CASE_COLLISION);
				}
			}
			node.m_Parent = v_Parent;
			node.m_NextSibling = parent.m_FirstChild;
			parent.m_FirstChild = v_Node;
		}

		void unlinkChild(PathTree& r_Tree, uint32_t v_Node) noexcept {
			TreeNode& parent = r_Tree.m_Nodes[r_Tree.m_Nodes[v_Node].m_Parent];
			uint32_t* link = &parent.m_FirstChild;
			while (*link != v_Node) link = &r_Tree.m_Nodes[*link].m_NextSibling;
			*link = r_Tree.m_Nodes[v_Node].m_NextSibling;
		}

		bool storeName(PathTree& r_Tree, std::string_view v_Name, uint32_t& r_Offset) noexcept {
			char* dst = static_cast<char*>(r_Tree.m_NameArena.allocate(v_Name.size()));
			if (dst == nullptr) return false;  // exec TreeFull
			std::memcpy(dst, v_Name.data(), v_Name.size());
			r_Offset = uint32_t(dst - r_Tree.m_Names);
			return true;
		}

		uint32_t takeNode(PathTree& r_Tree) noexcept {
			if (r_Tree.m_FreeHead != NODE_NONE) {
				const uint32_t index = r_Tree.m_FreeHead;
				r_Tree.m_FreeHead = r_Tree.m_Nodes[index].m_FirstChild;
				return index;
			}
			void* slot = r_Tree.m_NodeArena.allocate(sizeof(TreeNode), alignof(TreeNode));
			if (slot == nullptr) return NODE_NONE;  // exec TreeFull
			return r_Tree.m_NodeCount++;
		}

		void freeNode(PathTree& r_Tree, uint32_t v_Node) noexcept {
			TreeNode& node = r_Tree.m_Nodes[v_Node];
			node.m_Flags = NodeFlags::FREE;
			node.m_FirstChild = r_Tree.m_FreeHead;
			r_Tree.m_FreeHead = v_Node;
		}

		uint32_t addChild(PathTree& r_Tree, uint32_t v_Parent, std::string_view v_Segment, uint32_t v_Hash) noexcept {
			if (!reserveEdge(r_Tree)) return NODE_NONE;
			uint32_t nameOffset = 0;
			if (!storeName(r_Tree, v_Segment, nameOffset)) return NODE_NONE;
			const uint32_t index = takeNode(r_Tree);
			if (index == NODE_NONE) return NODE_NONE;

			TreeNode& node = r_Tree.m_Nodes[index];
			const uint32_t generation = node.m_Generation;
			node.m_NameOffset = nameOffset;
			node.m_NameLength = uint16_t(v_Segment.size());
			node.m_Flags = NodeFlags::ALIVE;
			node.m_Hash = v_Hash;
			node.m_Generation = generation;
			node.m_OpenCount.store(0, MemoryOrder::RELAXED);
			node.m_FirstChild = NODE_NONE;
			linkChild(r_Tree, v_Parent, index);
			placeEdge(r_Tree, index);
			return index;
		}

		// Kills the subtree: dead, generation bumped, edge dropped. Unpinned nodes go straight to the free list.
		void killSubtree(PathTree& r_Tree, uint32_t v_Top) noexcept {
			uint32_t current = v_Top;
			while (true) {
				TreeNode& node = r_Tree.m_Nodes[current];
				const uint32_t firstChild = node.m_FirstChild;
				dropEdge(r_Tree, current);
				clear(node.m_Flags, NodeFlags::ALIVE);
				++node.m_Generation;
				if (node.m_OpenCount.load(MemoryOrder::ACQUIRE) == 0) freeNode(r_Tree, current);

				if (firstChild != NODE_NONE) {
					current = firstChild;
					continue;
				}
				while (current != v_Top && r_Tree.m_Nodes[current].m_NextSibling == NODE_NONE) current = r_Tree.m_Nodes[current].m_Parent;
				if (current == v_Top) return;
				current = r_Tree.m_Nodes[current].m_NextSibling;
			}
		}

		// Walks or creates every segment. Rolls back what it created on failure.
		bool insertLocked(PathTree& r_Tree, const PathBuffer& r_Path, uint32_t& r_Out) noexcept {
			uint32_t created[MAX_SEGMENTS];
			uint32_t createdCount = 0;
			const auto nameMark = r_Tree.m_NameArena.mark();

			uint32_t current = 0;
			uint16_t offset = 0;
			std::string_view segment;
			while (Paths::nextSegment(r_Path, offset, segment)) {
				const uint32_t hash = hashSegment(segment);
				uint32_t child = findChild(r_Tree, current, segment, hash);
				if (child == NODE_NONE) {
					child = addChild(r_Tree, current, segment, hash);
					if (child == NODE_NONE) {
						while (createdCount > 0) {
							const uint32_t node = created[--createdCount];
							dropEdge(r_Tree, node);
							unlinkChild(r_Tree, node);
							freeNode(r_Tree, node);
						}
						r_Tree.m_NameArena.unwind(nameMark);
						return false;
					}
					created[createdCount++] = child;
				}
				current = child;
			}
			r_Out = current;
			return true;
		}

		NodeHandle handleOf(const PathTree& r_Tree, uint32_t v_Node) noexcept {
			return { v_Node, r_Tree.m_Nodes[v_Node].m_Generation };
		}
	}

	bool Trees::create(PathTree& r_Tree, std::string_view v_Mount, uint32_t v_Blocks, size_t v_NumaNode) noexcept {
		if (v_Mount.size() >= sizeof(r_Tree.m_Mount) || v_Blocks == 0 || v_Blocks % 4 != 0) return false;  // exec InvalidPath
		Memory::Allocators::BlockedAllocator* blocks = Memory::AtomicAllocators::instance().fileSystemBlocks(v_NumaNode);
		const size_t bytes = size_t(v_Blocks) * Memory::Allocators::BLOCK_SIZE;
		void* block = blocks->allocate(bytes);
		if (block == nullptr) return false;  // exec TreeFull

		std::memcpy(r_Tree.m_Mount, v_Mount.data(), v_Mount.size());
		r_Tree.m_MountLength = uint8_t(v_Mount.size());
		r_Tree.m_Block = block;
		r_Tree.m_BlockBytes = bytes;
		r_Tree.m_NumaNode = v_NumaNode;

		uint8_t* base = static_cast<uint8_t*>(block);
		const auto area = [&](size_t v_Offset, size_t v_Size) {
			MemoryHandle handle = Memory::Allocators::BlockedAllocator::handleFor(base + v_Offset, v_Size, v_NumaNode);
			handle.m_Memory.m_TotalSize = v_Size;
			return handle;
		};
		MemoryHandle nodes = area(0, bytes / 4);
		MemoryHandle names = area(bytes / 4, bytes / 2);
		r_Tree.m_NodeArena.init(&nodes);
		r_Tree.m_NameArena.init(&names);
		r_Tree.m_EdgeMemory = area(bytes / 4 * 3, bytes / 4);

		r_Tree.m_Nodes = reinterpret_cast<TreeNode*>(base);
		r_Tree.m_Names = reinterpret_cast<char*>(base + bytes / 4);
		r_Tree.m_Edges = reinterpret_cast<EdgeSlot*>(base + bytes / 4 * 3);
		r_Tree.m_EdgeMax = uint32_t((bytes / 4) / sizeof(EdgeSlot));
		r_Tree.m_NodeCount = 0;
		r_Tree.m_FreeHead = NODE_NONE;

		const uint32_t root = takeNode(r_Tree);
		TreeNode& node = r_Tree.m_Nodes[root];
		node = {};
		node.m_Parent = NODE_NONE;
		node.m_Flags = NodeFlags::ALIVE;
		node.m_FirstChild = NODE_NONE;
		node.m_NextSibling = NODE_NONE;
		return rebuildEdges(r_Tree, INITIAL_EDGES);
	}

	void Trees::destroy(PathTree& r_Tree) noexcept {
		if (r_Tree.m_Block == nullptr) return;
		// The run is decommitted whole; the arenas only need their cursors reset.
		Memory::AtomicAllocators::instance().fileSystemBlocks(r_Tree.m_NumaNode)->deallocate(r_Tree.m_Block, r_Tree.m_BlockBytes);
		r_Tree.m_Block = nullptr;
		r_Tree.m_NodeCount = 0;
	}

	NodeHandle Trees::lookup(PathTree& r_Tree, const PathBuffer& r_Path) noexcept {
		if (!onThisMount(r_Tree, r_Path)) return {};  // exec OutsideMount
		r_Tree.m_Lock.lockRead();
		uint32_t current = 0;
		uint16_t offset = 0;
		std::string_view segment;
		while (current != NODE_NONE && Paths::nextSegment(r_Path, offset, segment))
			current = findChild(r_Tree, current, segment, hashSegment(segment));
		const NodeHandle result = current == NODE_NONE ? NodeHandle{} : handleOf(r_Tree, current);
		r_Tree.m_Lock.unlockRead();
		return result;
	}

	bool Trees::insert(PathTree& r_Tree, const PathBuffer& r_Path, NodeHandle& r_Out) noexcept {
		if (!onThisMount(r_Tree, r_Path)) return false;  // exec OutsideMount
		r_Tree.m_Lock.lockWrite();
		uint32_t node = NODE_NONE;
		const bool ok = insertLocked(r_Tree, r_Path, node);
		if (ok) r_Out = handleOf(r_Tree, node);
		r_Tree.m_Lock.unlockWrite();
		return ok;
	}

	bool Trees::rename(PathTree& r_Tree, NodeHandle v_Node, const PathBuffer& r_NewPath) noexcept {
		if (!onThisMount(r_Tree, r_NewPath) || Paths::fileName(r_NewPath).empty()) return false;  // exec InvalidPath
		PathBuffer parentPath = r_NewPath;
		Paths::toParent(parentPath);
		const std::string_view newName = Paths::fileName(r_NewPath);
		const uint32_t newHash = hashSegment(newName);

		r_Tree.m_Lock.lockWrite();
		bool ok = validHandle(r_Tree, v_Node) && v_Node.m_Index != 0;  // exec NotFound
		uint32_t newParent = NODE_NONE;
		if (ok) ok = insertLocked(r_Tree, parentPath, newParent);
		for (uint32_t up = newParent; ok && up != NODE_NONE; up = r_Tree.m_Nodes[up].m_Parent)
			ok = up != v_Node.m_Index;  // exec InvalidPath: into its own subtree
		if (ok) ok = findChild(r_Tree, newParent, newName, newHash) == NODE_NONE;  // exec AlreadyExists
		uint32_t nameOffset = 0;
		if (ok) ok = reserveEdge(r_Tree) && storeName(r_Tree, newName, nameOffset);

		if (ok) {
			TreeNode& node = r_Tree.m_Nodes[v_Node.m_Index];
			dropEdge(r_Tree, v_Node.m_Index);
			unlinkChild(r_Tree, v_Node.m_Index);
			node.m_NameOffset = nameOffset;
			node.m_NameLength = uint16_t(newName.size());
			node.m_Hash = newHash;
			clear(node.m_Flags, NodeFlags::CASE_COLLISION);
			linkChild(r_Tree, newParent, v_Node.m_Index);
			placeEdge(r_Tree, v_Node.m_Index);
		}
		r_Tree.m_Lock.unlockWrite();
		return ok;
	}

	bool Trees::remove(PathTree& r_Tree, NodeHandle v_Node) noexcept {
		r_Tree.m_Lock.lockWrite();
		const bool ok = validHandle(r_Tree, v_Node) && v_Node.m_Index != 0;  // exec NotFound
		if (ok) {
			unlinkChild(r_Tree, v_Node.m_Index);
			killSubtree(r_Tree, v_Node.m_Index);
		}
		r_Tree.m_Lock.unlockWrite();
		return ok;
	}

	bool Trees::pin(PathTree& r_Tree, NodeHandle v_Node) noexcept {
		r_Tree.m_Lock.lockRead();
		const bool ok = validHandle(r_Tree, v_Node);  // exec NotFound
		if (ok) r_Tree.m_Nodes[v_Node.m_Index].m_OpenCount.fetchAdd(1, MemoryOrder::ACQ_REL);
		r_Tree.m_Lock.unlockRead();
		return ok;
	}

	void Trees::unpin(PathTree& r_Tree, NodeHandle v_Node) noexcept {
		if (v_Node.m_Index >= r_Tree.m_NodeCount) return;
		TreeNode& node = r_Tree.m_Nodes[v_Node.m_Index];
		const uint32_t previous = node.m_OpenCount.fetchAdd(UINT32_MAX, MemoryOrder::ACQ_REL);
		SPEC_FS_ASSERT(previous != 0);
		if (previous != 1) return;

		r_Tree.m_Lock.lockWrite();
		if (!has(node.m_Flags, NodeFlags::ALIVE) && !has(node.m_Flags, NodeFlags::FREE) && node.m_OpenCount.load(MemoryOrder::ACQUIRE) == 0)
			freeNode(r_Tree, v_Node.m_Index);
		r_Tree.m_Lock.unlockWrite();
	}

	bool Trees::buildPath(PathTree& r_Tree, NodeHandle v_Node, PathBuffer& r_Out) noexcept {
		r_Tree.m_Lock.lockRead();
		bool ok = validHandle(r_Tree, v_Node);  // exec NotFound
		uint32_t chain[MAX_SEGMENTS];
		uint32_t depth = 0;
		for (uint32_t up = v_Node.m_Index; ok && up != 0; up = r_Tree.m_Nodes[up].m_Parent) {
			ok = depth < MAX_SEGMENTS;
			if (ok) chain[depth++] = up;
		}

		char text[PATH_CAPACITY + 1];
		size_t length = 0;
		const auto append = [&](std::string_view v_Text) {
			ok = ok && length + v_Text.size() <= PATH_CAPACITY;
			if (ok) std::memcpy(text + length, v_Text.data(), v_Text.size());
			if (ok) length += v_Text.size();
		};
		append({ r_Tree.m_Mount, r_Tree.m_MountLength });
		append(":/");
		for (uint32_t i = depth; ok && i > 0; --i) {
			append(nameOf(r_Tree, r_Tree.m_Nodes[chain[i - 1]]));
			if (i > 1) append("/");
		}
		r_Tree.m_Lock.unlockRead();
		return ok && Paths::make(r_Out, { text, length });
	}

	bool Trees::isAlive(PathTree& r_Tree, NodeHandle v_Node) noexcept {
		r_Tree.m_Lock.lockRead();
		const bool alive = validHandle(r_Tree, v_Node);
		r_Tree.m_Lock.unlockRead();
		return alive;
	}

	bool Trees::hasCaseCollision(PathTree& r_Tree, NodeHandle v_Node) noexcept {
		r_Tree.m_Lock.lockRead();
		const bool collision = validHandle(r_Tree, v_Node) && has(r_Tree.m_Nodes[v_Node.m_Index].m_Flags, NodeFlags::CASE_COLLISION);
		r_Tree.m_Lock.unlockRead();
		return collision;
	}
}
