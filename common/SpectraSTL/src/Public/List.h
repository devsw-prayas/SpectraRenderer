#pragma once

#include <SpectraSTL.h>
#include <optional>

namespace spectra::stl::abstraction{
    template<typename T, typename A>
    class SPECTRA_STL List{
        using allocator = A;
        using item = T;

        //Retrieval functions
        const item& at(size_t idx) const = 0;
        item& operator[](size_t idx) const = 0;

        const item& front() const = 0;
        const item& back() const = 0;

        //Mutators
        bool pushFront(const item&& element) = 0;
        bool pushBack(const item&& element) = 0;

        std::optional<item> popFromt() = 0;
        std::optional<item> popBack() = 0;

        bool insert(size_t idx, const item& element) = 0;

        template<typenae ...args>
        bool emplace(size_t idx, args constArgs) = 0;

        template<typename ...args>
        bool emplaceBack(args constArgs) = 0;

        std::optinoal<item> remove(size_t idx) = 0;

        bool clear() = 0;

        //Search
        bool find(const item& element) const = 0;
        bool contains(const item& element) const = 0;

        //Capacity
        size_t size() const = 0;
        size_t capacity() const = 0;
        bool reserve(size_t newCapacity) = 0;
        void shrinkToFit() = 0;
        bool resize(size_t newSize) = 0;
        size_t maxSize() const = 0;
        bool empty() const = 0;

        //Iterators
        //TODO Errors...
    }; 
}