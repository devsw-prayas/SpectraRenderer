#pragma once 

#include "SpectraSTL.h"
#include <iterator>
#include <type_traits>

namespace spectra::stl{

    template<typename Derived, typename T, typename Tag>
    class Iterator{
        public: 
        using value_type            = T;
        using difference_type       = std::ptrdiff_t;
        using pointer               = T*;
        using reference             = T&;
        using iterator_category     = Tag;

        reference operator*() const{
            return &static_cast<const Derived*>(this)->dereference();
        }

        pointer operator->() const{
            return &static_cast<const Derived*>(this)->dereference();
        }

        bool operator==(const Derived& other) const{
            return static_cast<const Derived*>(this)->equals(other);
        }

        bool operator!=(const Derived& other) const{
            return !(*this == other);
        }

        Derived& operator++() {
            static_cast<Derived*>(this)->increment();
            return *static_cast<Derived*>(this);
        }

        Derived operator++(int) {
            Derived tmp = *static_cast<Derived*>(this);
            ++(*this);
            return tmp;
        }

        Derived& operator--() requires std::is_base_of_v<std::bidirectional_iterator_tag, Tag> {
            static_cast<Derived*>(this)->decrement();
            return *static_cast<Derived*>(this);
        }

        Derived operator--(int) requires std::is_base_of_v<std::bidirectional_iterator_tag, Tag> {
            Derived tmp = *static_cast<Derived*>(this);
            --(*this);
            return tmp;
        }
    };
}