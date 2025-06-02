#pragma once

#include <SpectraSTL.h>
#include <optional>

namespace spectra::stl::abstraction{
    template<typename T, typename A>
    class SPECTRA_STL List{
        using allocator = A;
        using item = T;
			
    public:
        //Constructors
        List() = default;
        virtual ~List() = default;

        //Retrieval functions
        virtual const item& at(size_t idx) const = 0;
        virtual item& operator[](size_t idx) const = 0;

        virtual const item& front() const = 0;
        virtual const item& back() const = 0;

        //Mutators
        virtual bool pushFront(const item&& element) = 0;
        virtual bool pushBack(const item&& element) = 0;

        virtual std::optional<item> popFront() = 0;
        virtual std::optional<item> popBack() = 0;

        virtual bool insert(size_t idx, const item& element) = 0;

        template<typename ...Args>
        bool emplace(size_t idx, Args&&... ags){
            return insert(idx, item(std::forward<Args>(ags)...));
        }

        template<typename ...Args>
        bool emplaceFront(Args&&... ags){
            return pushFront(item(std::forward<Args>(ags)...));
        }

        template<typename ...Args>
        bool emplaceBack(Args&&... ags){
            return pushBack(item(std::forward<Args>(ags)...));
        }

        virtual std::optional<item> remove(size_t idx) = 0;

        virtual bool clear() = 0;

        //Search
        virtual bool find(const item& element) const = 0;
        virtual bool contains(const item& element) const = 0;

        //Capacity
        [[nodiscard]] virtual size_t size() const = 0;
        [[nodiscard]] virtual size_t capacity() const = 0;
        virtual bool reserve(size_t newCapacity) = 0;
        virtual void shrinkToFit() = 0;
        virtual bool resize(size_t newSize) = 0;
        [[nodiscard]] virtual size_t maxSize() const = 0;
        [[nodiscard]] virtual bool empty() const = 0;

        //Iterators
        //TODO...
    }; 
}