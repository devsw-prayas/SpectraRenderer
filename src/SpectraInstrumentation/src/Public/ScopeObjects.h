#pragma once

#include "SpectraInstrumentation.h"

namespace Spectra::Instrumentation {

    template<auto Code, typename Fn>
    struct SwitchCase {
        Fn m_Fn;
    };

    template<auto Code, typename Fn>
    constexpr auto caseOf(Fn&& fn) {
        return SwitchCase<Code, std::decay_t<Fn>>{std::forward<Fn>(fn)};
    }

    template<typename Fn>
    struct DefaultCase {
        Fn m_Fn;
    };

    template<typename Fn>
    constexpr auto otherwise(Fn&& fn) {
        return DefaultCase<std::decay_t<Fn>>{std::forward<Fn>(fn)};
    }

    template<typename T, auto Code, typename Fn>
    bool tryCase(T value, const SwitchCase<Code, Fn>& c) {
        if (value == Code) { c.m_Fn(); return true; }
        return false;
    }

    template<typename T, typename Fn>
    bool tryCase(T value, const DefaultCase<Fn>& c) {
        c.m_Fn(value);
        return true;
    }

    template<typename T, typename... Cases>
    void staticSwitch(T value, Cases&&... cases) {
        (tryCase(value, cases) || ...);
    }

}
