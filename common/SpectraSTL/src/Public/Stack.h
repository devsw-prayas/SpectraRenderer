#pragma once

#include<optional>
#include "SpectraSTL.h"

namespace spectra::stl::abstraction{
    template<typename T, typename A>
    class SPECTRA_STL Stack{
        using item = T;
        using allocator = A;

    public:
		Stack() = default;
		virtual ~Stack() = default;

        //Capacity
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual bool isEmpty() const = 0;
        virtual bool clear() = 0;

        //Accessors
        virtual const item& top() const = 0;
        virtual item& tos() const = 0;

        //Mutators
        virtual bool push(const T& item) = 0;
        virtual std::optional<item> pop() = 0;

        template<typename... Args>
        bool emplace(Args&&... args){
            return push(item(std::forward<Args>(args)...));
        }
    };
}