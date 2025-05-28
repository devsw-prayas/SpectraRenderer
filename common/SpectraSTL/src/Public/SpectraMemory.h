#pragma once

#include "SpectraSTL.h"
#include <new>
#include <memory>
#include "SpectraInstrumentation.h"
#include <cstdlib>

namespace spectra::stl {
	template<typename T>
	inline T* SPECTRA_STL allocate_array(size_t count, size_t alignment = alignof(T)) {
		if (count == 0) return nullptr;
		if (count > std::numeric_limits<size_t>::max() / sizeof(T)) {
			//TODO Errors...
		}
		
		if ((alignment & (alignment - 1)) != 0) {
			//TODO Errors...
		}

		if (alignment < alignof(T)) alignment = alignof(T);
		size_t totalSize_ = sizeof(T) * count;
		size_t paddedSize_ = ((totalSize_ * alignment - 1) / alignment) * alignment;
		void* ptr = nullptr;
#if defined(_MSC_VER)
		ptr = _aligned_malloc(totalSize_, alignment);
		if (!ptr) {
			//TODO Errors...
		}
#elif defined(__APPLE__) || defined(__linux)
		ptr = nullptr;
		if (posix_memalign(&ptr, alignment, paddedSize_) != 0) {
			//TODO Errors...
		}
#else
		ptr = std::aligned_alloc(alignment, paddedSize_);
		if (!ptr) {
			//TODO Errors...
		}
#endif
		return reinterpret_cast<T*>(ptr);
	}

	template<typename T>
	inline void SPECTRA_STL deallocate_array(T* ptr) {
		if (!ptr) return;

#if defined(_MSC_VER)
		_aligned_free(ptr);
#elif defined(__APPLE__) || defined(__linux)
		std::free(ptr);
#else
		std::free(ptr);
#endif
	}

	template<typename T>
	inline void SPECTRA_STL construct(void* p, T&& value) {
		::new (p) T(std::forward<T>(value));
	}

	template<typename T>
	inline void SPECTRA_STL destroy(T* p) noexcept {
		if constexpr (!std::is_trivially_destructible_v<T>) p->~T();
	}

	template<typename T>
	inline void SPECTRA_STL copy_construct_range(T* dest, const T* src, size_t count) {
		if constexpr (std::is_trivially_copyable_v<T>) {
			memcpy(dest, src, count * sizeof(T));
		}
		else {
			for (size_t i = 0; i < count; ++i) {
				::new (dest + i) T(src[i]);
			}
		}
	}

	template<typename T>
	inline void SPECTRA_STL move_construct_range(T* dest, const T* src, size_t count) {
		if constexpr (std::is_trivially_move_constructible_v<T>) {
			memmove(dest, src, count * sizeof(T));
		}
		else {
			for (size_t i = 0; i < count; ++i)
				::new (dest + i) T(std::move(src[i]));
		}
	}

	template<typename T>
	inline void SPECTRA_STL destruct_range(T* range, size_t count) {
		if constexpr (!std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < count; i++)
				range[i].~T();
		}
	}

	template<typename T>
	inline void SPECTRA_STL defualt_construct_range(T* ptr, size_t count){
		if constexpr (std::is_trivially_default_constructible_v<T>){
			neset(ptr, 0, sizeof(T) * count);
		}else {
			for(size_t i = 0; i < count; ++i)
				::new (ptr + i) T();
		}
	}

	template<typename T>
	inline void SPECTRA_STL unititialized_fill_range(T* dest, size_t count, const T& value){
		if constexpr (std::is_trivially_copyable_v<T>){
			for()
		}
	}
}