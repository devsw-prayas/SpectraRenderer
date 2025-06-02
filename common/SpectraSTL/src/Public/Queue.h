#pragma once

#include "SpectraSTL.h"
#include <optional>

namespace spectra::stl::abstraction{
    template<typename T, typename A>
    class SPECTRA_STL Queue{
        using item = T;
        using allocator = A;

    public:
		Queue() = default;
		virtual ~Queue() = default;

        //Capacity
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual bool isEmpty() const = 0;
        virtual bool clear() = 0;

        //Accessors
        virtual const item& front() const = 0;
        virtual item& front() = 0;

        virtual const item& back() const = 0;
        virtual item& back() = 0;

        //Mutators
        virtual bool push(const item& element) = 0;
        virtual std::optional<item> pop() = 0;

        template<typename... Args>
        bool emplace(Args&&... args){
            return push(item(std::forward<Args>(args)...));
        }
    };
}
