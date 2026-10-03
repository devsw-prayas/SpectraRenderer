#pragma once

// Defaults on: a TU missing the CMake define should still trace, not silently compile out.
#ifndef SPECTRA_INSTRUMENTATION_ENABLED
#define SPECTRA_INSTRUMENTATION_ENABLED 1
#endif

#define SPEC_INST_CONCAT_IMPL(a, b) a##b
#define SPEC_INST_CONCAT(a, b) SPEC_INST_CONCAT_IMPL(a, b)

// Per-line guard names so nested scopes don't shadow each other (C4456).
#if SPECTRA_INSTRUMENTATION_ENABLED
#define SPEC_INST_SCOPE_BEGIN(ScopeObject, ...) { ScopeObject SPEC_INST_CONCAT(l_InstScope, __LINE__)(__VA_ARGS__);
#define SPEC_INST_SCOPE_END() }
#else
#define SPEC_INST_SCOPE_BEGIN(ScopeObject, ...) {
#define SPEC_INST_SCOPE_END() }
#endif
