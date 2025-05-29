	#pragma once

	#include "SpectraSTL.h"
	#include <new>
	#include <memory>
	#include "SpectraInstrumentation.h"
	#include <cstdlib>

	namespace spectra::stl {

		template<typename T>
		constexpr void static_assert_valid_type() {
    		static_assert(!std::is_abstract_v<T>, "Cannot construct abstract type.");
    		static_assert(!std::is_function_v<T>, "Cannot construct function type.");
    		static_assert(!std::is_member_function_pointer_v<T>, "Cannot construct member function pointer.");
    		static_assert(!std::is_pointer_v<T> || !std::is_function_v<std::remove_pointer_t<T>>,
				"Cannot construct function pointer.");
}

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
			size_t paddedSize_ = ((totalSize_ + alignment - 1) / alignment) * alignment;
			void* ptr = nullptr;
	#if defined(_MSC_VER)
			ptr = _aligned_malloc(paddedSize_, alignment);
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
			static_assert_valid_type<T>();
			::new (p) T(std::forward<T>(value));
		}

		template<typename T>
		inline void SPECTRA_STL destroy(T* p) noexcept {
			static_assert(!std::is_function_v<T>, "Cannot destroy function type.");
			if constexpr (!std::is_trivially_destructible_v<T>) p->~T();
		}

		template<typename T>
		inline void SPECTRA_STL copy_construct_range(T* dest, const T* src, size_t count) {
			static_assert_valid_type<T>();
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
			static_assert_valid_type<T>();
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
		inline void SPECTRA_STL default_construct_range(T* ptr, size_t count){
			static_assert_valid_type<T>();
			if constexpr (std::is_trivially_default_constructible_v<T>){
				memset(ptr, 0, sizeof(T) * count);
			}else {
				for(size_t i = 0; i < count; ++i)
					::new (ptr + i) T();
			}
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_fill_range(T* dest, size_t count, const T& value){
			static_assert_valid_type<T>();
			if constexpr (std::is_trivially_copyable_v<T>){
				for(size_t i = 0; i < count; ++i){
					dest[i] = value;
				}
			}else{
				size_t i = 0;
				try{
					for(; i < count; ++i){
						::new (dest + i) T(value);
					}
				}catch(...){
					for(size_t j = 0; j < i; ++j){
						destroy(dest + j);
					}
					//TODO Errors...
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_copy_range(T* dest, const T* src, size_t count){
			copy_construct_range(dest, src, count);
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_move_range(T* dest, const T* src, size_t count){
			move_construct_range(dest, src, count);
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_default_construct_range(T* ptr, size_t count){
			static_assert_valid_type<T>();
			if constexpr (std::is_trivially_default_constructible_v<T>){
				memset(ptr, 0 , count * sizeof(T));
			}else{
				size_t i = 0;
				try{
					for(; i < count; ++i){
						::new (ptr + i) T();
					}
				}catch(...){
					for(size_t j = 0; j < i; ++j){
						destroy(ptr + j);
					}

					//TODO Errors....
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_value_construct_range(T* ptr, size_t count){
			uninitialized_default_construct_range(ptr, count);
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_copy_n(T* dest, const T* src, size_t count){
			static_assert_valid_type<T>();
			if constexpr (std::is_trivially_copyable_v<T>){
				memcpy(dest, src, count * sizeof(T));
			}else{
				size_t i = 0;
				try{
					for(;i < count; ++i){
						::new (dest + i) T(src[i]);
					}
				}catch(...){
					for(size_t j = 0; j < i; ++j){
						destroy(dest + j);
					}

					//TODO Errors...
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL uninitialized_move_n(T* dest, const T* src, size_t count){
			static_assert_valid_type<T>();
			if constexpr (std::is_trivially_move_constructible_v<T>){
				memmove(dest, src, count * sizeof(T));
			}else{
				size_t i = 0;
				try{
					for(; i < count; ++i){
						::new (dest + i) T(std::move(src[i]));
					}
				}catch(...){
					for(size_t j = 0; j < i; j++){
						destroy(dest + j);
					}

					//TODO Errors...
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL fill_range(T* dest, size_t count, const T& value){
			for(size_t i = 0;  i < count; ++i){
				dest[i] = value;
			}
		}

		template<typename T>
		inline void SPECTRA_STL copy_range(T* dest, const T* src, size_t count){
			if constexpr (std::is_trivially_copyable_v<T>){
				memcpy(dest, src, count * sizeof(T));
			}else {
				for(size_t i = 0; i < count; ++i){
					dest[i] = src[i];
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL move_range(T* dest, const T* src, size_t count){
			if constexpr (std::is_trivially_move_assignable_v<T>){
				memmove(dest, src, count * sizeof(T));
			}else{
				for(size_t i = 0; i < count; ++i){
					dest[i] = std::move(src[i]);
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL destroy_n(T* ptr, size_t count){
			if constexpr (!std::is_trivially_destructible_v<T>){
				for(size_t i = 0; i < count; ++i){
					ptr[i].~T();
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL relocate_range(T* dest, T* src, size_t count){
			if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>){
				memmove(dest, src, count * sizeof(T));
			}else{
				for(size_t i = 0; i < count; ++i){
					::new (dest + i) T(std::move(src[i]));
					if constexpr (!std::is_trivially_destructible_v<T>){
						(src + i).~T();
					}
				}
			}
		}

		template<typename T>
		inline void SPECTRA_STL swap(T& a, T& b){
			if constexpr(std::is_trivially_copyable_v<T>){
				alignas(T) char buffer[sizeof(T)];
				memcpy(buffer, &a, sizeof(T));
				memcpy(&a , &b, sizeof(T));
				memcpy(&b, buffer, sizeof(T));
			}else if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>){
				T temp = std::move(a);
				a = std::move(b);
				b = std::move(temp);
			}else{
				static_assert(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>,
					"Type must be move constructible and move assignable to be swapped.");
				T temp = std::move(a);
				a = std::move(b);
				b = std::move(temp);
			}
		}	
	}