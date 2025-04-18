#pragma once
#include "SpectraSTL.h"
#include <new>

namespace spectra::stl {
	template<typename T>
	class SPECTRA_STL Allocator {
		using value_type = T;
	public:
		Allocator() noexcept = default;
		template<typename U>
		Allocator(const Allocator<U>&) noexcept {}

		virtual T* allocate(size_t n) {
			void* p = ::operator new(n * sizeof(T), std::align_val_t{ 32 });
			return static_cast<T*>(p);
		}

		virtual void deallocate(T* p, size_t n) noexcept {
			::operator delete(p, std::align_val_t{ 32 });
		}

		template<typename... Args>
		void construct(T* p, Args&&... args) {
			::new (p) T(std::forward<Args>(args)...);
		}

		virtual void destroy(T* p) noexcept {
			p->~T();
		}

		template<typename U>
		struct rebind {
			using other = Allocator<U>;
		};

#ifdef SPECTRA_DEBUG
		size_t allocations = 0;
		size_t deallocations = 0;
#endif
	};

	template <typename T>
	class SPECTRA_STL BlockAllocator : public Allocator<T> {
		static constexpr size_t BLOCK_SIZE = 64;
		static constexpr size_t BLOCKS_PER_SLAB = 64;
		struct Block {
			union {
				T data;
				Block* next;
			};
		};

		struct Slab {
			Block* blocks[BLOCKS_PER_SLAB];
			Slab* next;
		};

		Slab* slabs = nullptr;
		Block* free_list = nullptr;

		void allocateSlab() {
			Slab* new_slab = static_cast<Slab*>(::operator new(sizeof(Slab), std::align_val_t{ 32 }));
			new_slab->next = slabs;
			slabs = new_slab;

			for (size_t i = 0; i < BLOCKS_PER_SLAB - 1; ++i) {
				new_slab->blocks[i].next = &new_slab->blocks[i + 1];
			}
			new_slab->blocks[BLOCKS_PER_SLAB - 1].next = free_list;
			free_list = &new_slab->blocks[0];
		}

	public:
		BlockAllocator() noexcept {
			allocateSlab();
		}

		~BlockAllocator() noexcept {
			while (slabs) {
				Slab* next = slabs->next;
				::operator delete(slabs, std::align_val_t{ 32 });
				slabs = next;
			}
		}

		T* allocate(size_t n) override{
			if (n != 1) {
				//TODO throwing error
			}
			if (!free_list) {
				allocateSlab();
			}
			Block* block = free_list;
			free_list = free_list->next;
#ifdef SPECTRA_DEBYG
			this->allocations++;
#endif
			return &block->data;
		}

		void deallocate(T* p, size_t n) noexcept override{
			if (n != 1) {
				//TODO throwing error
			}
			Block* block = reinterpret_cast<Block*>(p);
			block->next = free_list;
			free_list = block;
#ifdef SPECTRA_DEBUG
			this->deallocations++;
#endif
		}
	};

	template <typename T>
	class StackAllocator : public Allocator<T> {
	private:
		static constexpr size_t DEFAULT_CAPACITY = 1024;
		uint8_t* buffer = nullptr;
		uint8_t* top = nullptr;
		size_t capacity_bytes = DEFAULT_CAPACITY * sizeof(T);

	public:
		StackAllocator() noexcept {
			buffer = static_cast<uint8_t*>(::operator new(capacity_bytes, std::align_val_t{ 32 }));
			top = buffer;
		}

		~StackAllocator() noexcept {
			::operator delete(buffer, std::align_val_t{ 32 });
		}

		T* allocate(size_t n) override {
			size_t bytes = n * sizeof(T);
			size_t aligned_bytes = (bytes + 31) & ~31; // 32B alignment
			if (top + aligned_bytes > buffer + capacity_bytes) {
				throw std::bad_alloc();
			}
			T* result = reinterpret_cast<T*>(top);
			top += aligned_bytes;
#ifdef SPECTRA_DEBUG
			this->allocations++;
#endif
			return result;
		}

		void deallocate(T* p, size_t n) noexcept override {
			// Nope, use reset() instead
		}

		void reset() noexcept {
			top = buffer;
#ifdef SPECTRA_DEBUG
			this->deallocations += this->allocations;
			this->allocations = 0;
#endif
		}

		void* get_marker() const noexcept { return top; }
		void rewind(void* marker) noexcept { top = static_cast<uint8_t*>(marker); }
	};

	template <typename T>
	class PoolAllocator : public Allocator<T> {
	private:
		static constexpr size_t POOL_SIZE = 1024;
		struct Slot {
			union {
				T data; // When allocated
				Slot* next; // When free
			};
		};
		Slot* pool = nullptr;
		Slot* free_list = nullptr;

		void init_pool() {
			pool = static_cast<Slot*>(::operator new(sizeof(Slot) * POOL_SIZE, std::align_val_t{ 32 }));
			free_list = pool;
			for (size_t i = 0; i < POOL_SIZE - 1; ++i) {
				pool[i].next = &pool[i + 1];
			}
			pool[POOL_SIZE - 1].next = nullptr;
		}

	public:
		PoolAllocator() noexcept { init_pool(); }
		~PoolAllocator() noexcept {
			::operator delete(pool, std::align_val_t{ 32 });
		}

		T* allocate(size_t n) override {
			if (n != 1) throw std::bad_alloc();
			if (!free_list) throw std::bad_alloc();
			Slot* slot = free_list;
			free_list = slot->next;
#ifdef SPECTRA_DEBUG
			this->allocations++;
#endif
			return &slot->data;
		}

		void deallocate(T* p, size_t n) noexcept override {
			if (n != 1) return;
			Slot* slot = reinterpret_cast<Slot*>(p);
			slot->next = free_list;
			free_list = slot;
#ifdef SPECTRA_DEBUG
			this->deallocations++;
#endif
		}
	};
}
