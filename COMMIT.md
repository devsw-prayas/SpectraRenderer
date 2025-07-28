# Commit Guidelines
To maintain clarity, consistency, and traceability throughout Blaze's development, we follow a structured commit format. This format not only makes the project easier to navigate and audit, but also aligns well with Blaze’s principles of precision, modularity, and performance-driven engineering. Each commit should clearly state its intent and affected subsystem, ensuring that every change — whether experimental, corrective, or architectural — is both understandable and reproducible.

| Prefix   | Scope       | Description                                    | Example                                              |
|----------|-------------|------------------------------------------------|------------------------------------------------------|
| init     | core        | Initial setup of core logic                    | `init(core): Set up main event loop and config`      |
| impl     | threadpool  | Added/implemented new feature                  | `impl(threadpool): Add dynamic thread scaling`       |
| fix      | scheduler   | Bugfix in scheduler logic                      | `fix(scheduler): Resolve deadlock in task queue`     |
| refactor | alloc       | Code restructure without changing behavior     | `refactor(alloc): Simplify memory pool logic`        |
| opt      | dispatch    | Performance or memory optimization             | `opt(dispatch): Reduce lock contention in queue`     |
| docs     | api         | Documentation for APIs or systems              | `docs(api): Add REST endpoint usage guide`           |
| test     | worksteal   | Add or update tests                            | `test(worksteal): Add unit tests for task stealing`  |
| wip      | bulkexec    | Work in progress, squash later                 | `wip(bulkexec): Partial impl of batch processing`    |
| hook     | instrument  | Instrumentation or Stratum-related hook added  | `hook(instrument): Add tracing for task latency`     |
| meta     | build       | Build system, compiler flags, or CI changes    | `meta(build): Update CI to use GCC 14`               |

| Tag | Description | Example Commit Message |
|-----|-------------|-----------------------|
| [Init] | Initial setup for a module, component, or system | [Init] Core interfaces and base infrastructure |
| [Add] | Added a new feature, function, or type | [Add] Data serializer and format registry |
| [Fix] | Fixed a bug or logic error | [Fix] Boundary condition in range checks |
| [Refactor] | Code restructuring without changing behavior | [Refactor] Split utility functions into separate file |
| [Internal] | Developer-facing changes like logging, build, or config | [Internal] Updated build script for platform compatibility |
| [Perf] | Performance-related improvements | [Perf] Reduced allocations in loop using preallocation |
| [Docs] | Documentation changes, comment updates, or markdown edits | [Docs] Improved inline comments and updated usage guide |
| [Test] | Added or modified tests, testing tools, or test data | [Test] Added unit tests for parsing logic |
| [Remove] | Removed obsolete code, files, or unused features | [Remove] Deprecated legacy config loader |