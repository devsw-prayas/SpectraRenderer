# Spectra Design Patterns and Architecture

## Motivation
Spectra, unlike traditional render engines, aggressively utilizes both CPU and GPU resources to achieve its computational goals. The following architectural rules enforce patterns that maximize raw performance without sacrificing clarity or ease of use.

## A) Pimpl Idiom
1. The Pimpl idiom is strictly prohibited within Spectra's internal modules.
2. Spectra mandates direct access to Windows internals wherever possible, provided the system remains within the bounds of user-mode execution. This enables maximum computational performance with minimal abstraction overhead.
3. Spectra favors full exposure of platform-level internals, using carefully crafted function wrappers only where necessary to minimize indirection and performance cost.

## B) RAII for All Resources
1. RAII (Resource Acquisition Is Initialization) is mandatory for managing all owned resources across the entire codebase, ensuring deterministic cleanup and exception safety.

## C) Command / Task Patterns for Deferred Logic
1. Spectra’s deferred execution model is implemented via the Command and Task patterns. For implementation details, refer to Blaze's architectural specification.

## D) Architectural Pattern Usage
1. Patterns such as Observer, Component, State Machine, or Builder are not globally enforced. Instead, they are embedded directly in the documentation of the module they serve.