#pragma once

#include "SpectraInstrumentation.h"
#include "SpectraSTL.h"
#include "Traits.h"
#include "Allocators.h"

namespace spectra::stl {
    template <typename T>
    class SPECTRA_STL AlignedVector {
        static_assert(is_numeric_v<T>, "AlignedVector only supports floating point and integer types");

    public:
        // Constructors
        AlignedVector() :data_(nullptr), size_(0), capacity_(0), allocator_() {}
        explicit AlignedVector(size_t n, T value = T()) : data_(nullptr), size_(0), capacity_(0) {
            resize(n, value);
        }

        // Copy constructor
        AlignedVector(const AlignedVector& other) : data_(nullptr), size_(0), capacity_(0) {
            reserve(other.size_);
            size_ = other.size_;
            memcpy(data_, other.data_, size_ * sizeof(T));
        }

        // Copy assignment
        AlignedVector& operator=(const AlignedVector& other) {
            if (this != &other) {
				if (data_) allocator_.deallocate(data_, capacity_);
                data_ = nullptr;
                size_ = 0;
                capacity_ = 0;
                reserve(other.size_);
                size_ = other.size_;
                memcpy(data_, other.data_, size_ * sizeof(T));
            }
            return *this;
        }

        // Move constructor
        AlignedVector(AlignedVector&& other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        // Move assignment
        AlignedVector& operator=(AlignedVector&& other) noexcept {
            if (this != &other) {
				if (data_) allocator_.deallocate(data_, capacity_);
                data_ = other.data_;
                size_ = other.size_;
                capacity_ = other.capacity_;
                other.data_ = nullptr;
                other.size_ = 0;
                other.capacity_ = 0;
            }
            return *this;
        }

        // Destructor
        ~AlignedVector() {
			if (data_) allocator_.deallocate(data_, capacity_);
        }

        // Accessors
        T* begin() { return data_; }
        const T* begin() const { return data_; }
        T* end() { return data_ + size_; }
        const T* end() const { return data_ + size_; }
        size_t get_size() const { return size_; }
        size_t get_capacity() const { return capacity_; }

        // Element access
        T& operator[](size_t i) { return data_[i]; } // No bounds live dangerously!
        const T& operator[](size_t i) const { return data_[i]; }
        T& at(size_t i) {
            if (i >= size_)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "AlignedVector",
					"Index out of bounds",instrumentation::E_LogComponent::CORE, LOCATION);
            return data_[i];
        }
        const T& at(size_t i) const {
            if (i >= size_)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "AlignedVector",
					"Index out of bounds", instrumentation::E_LogComponent::CORE, LOCATION);
            return data_[i];
        }

        // Modifiers
        void push_back(T value) {
            if (size_ == capacity_) {
                size_t new_capacity = capacity_ ? capacity_ * 2 : 16; // Start bigger for AVX
                reserve(new_capacity);
            }
            data_[size_++] = value;
        }

        void resize(size_t new_size, T value = T()) {
            if (new_size > capacity_) {
                size_t new_capacity = (new_size + 15) & ~15; // Round up to 16 for AVX
                reserve(new_capacity);
            }
            if (new_size > size_) {
                for (size_t i = size_; i < new_size; ++i) data_[i] = value;
            }
            size_ = new_size;
        }

        void reserve(size_t new_capacity) {
            if (new_capacity <= capacity_) return;
            T* new_data = static_cast<T*>(allocator_.allocate(32, new_capacity * sizeof(T)));
            if (!new_data)
				instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::stl", "AlignedVector",
					"Failed to allocate memory", instrumentation::E_LogComponent::STL, LOCATION);
            if (data_) {
                memcpy(new_data, data_, size_ * sizeof(T));
                aligned_free(data_);
            }
            data_ = new_data;
            capacity_ = new_capacity;
        }

        void clear() {
            size_ = 0;
        }

        // AVX-friendly size helper
        size_t aligned_size() const {
            return (size_ + avx512_lanes_v<T> -1) & ~(avx512_lanes_v<T> -1); // Round up to lanes
        }

    private:
        T* data_;
        size_t size_;
        size_t capacity_;
		StackAllocator<T> allocator_;
    };
}

