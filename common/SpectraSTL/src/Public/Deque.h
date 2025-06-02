#pragma once

#include "SpectraSTL.h"
#include <optional>

namespace spectra::stl::abstraction{
    template<typename T, typename A>
    class SPECTRA_STL Deque{
        using item = T;
        using allocator = A;

    public:
		//Constructors
		Deque() = default;
		virtual ~Deque() = default;

        //Capacity
        [[nodiscard]] virtual size_t capacity() const = 0;
        [[nodiscard]] virtual bool isEmpty() const = 0;
        virtual bool clear() = 0;
        [[nodiscard]] virtual size_t size() const = 0;
        virtual bool reserve(size_t size) = 0;

        //Access
        virtual const item& front() const = 0;
        virtual const item& back() const = 0;
        virtual const item& at(size_t idx) const = 0;
        virtual item& operator[](size_t idx) = 0;

        //Mutators
        virtual bool pushBack(const item& element) = 0;
        virtual bool pushFront(const item& element) = 0;
        virtual std::optional<item> popFront() = 0;
        virtual std::optional<item> popBack() = 0;
        
        template<typename ...Args>
        bool emplaceBack(Args&&... ags) {
            return pushBack(item(std::forward<Args>(ags)...));
        }

        template<typename ...Args>
        bool emplaceFront(Args&&... ags){
            return pushFront(item(std::forward<Args>(ags)...));
        }

        //Iterators
        //TODO...

    };
}