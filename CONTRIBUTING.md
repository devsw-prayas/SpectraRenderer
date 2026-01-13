# Commit Guidelines for Spectra

To ensure clarity and traceability across all of Spectra — including modules like Blaze, Iota, Stratum, and Kerbecs — we follow a file-scoped commit format. This focuses on which files or systems are affected (in the title), and why the change was made (in the body), with clean and consistent tagging.

## Commit Format

### Commit Title:
```
[file_or_component] [another_component] ...
```

Use square brackets [] to indicate each file, module, or subsystem affected.
If the commit affects too many areas, just write [multiple].
Do not describe the change in the title — keep it purely scope-based.

### Examples:
[executor] [task_traits] [this_thread]
[iota] [tensor] [allocator]
[scheduler]
[multiple]

Commit Description (Required):
Start with a tag that describes the intent of the change, followed by a concise explanation.
### Format:
```
[Tag] Brief explanation of what changed and why.
```
### Examples:
```
[Fix] Prevented deadlock in recursive task fallback path.
[Add] Introduced TLS allocator routing in all task handles.
[Refactor] Split monolithic executor into modular traits.
[Perf] Reduced spectral texture conversion latency by caching LUTs.
[Docs] Documented lifecycle of thread-local execution handles.
```
### Allowed Tags:
```
[Init] — First commit of a module or system
[Add] — New feature, type, or functionality
[Fix] — Bug fix or logic correction
[Refactor] — Structural change, no behavior change
[Perf] — Performance or memory improvement
[Docs] — Documentation, comments, usage guides
[Test] — Tests, test cases, validation logic
[Remove] — Deleted obsolete logic or files
[Internal] — Developer-only change (builds, tools, CI)
[WIP] — Work-in-progress (must squash before merge)
```
## Best Practices:

Always start the commit description with a valid [Tag]
Use present tense in your message:
✅ [Add] Add fast path for batch scheduling
❌ [Add] Added fast path for batch scheduling


Squash or clean up [WIP] commits before merging
Be consistent, even for personal branches

## Sample Commits:
```
[iota] [tensor]
[Refactor] Separated weight initialization logic from tensor class to improve modularity and reuse.

[blaze] [executor]
[Fix] Fixed race condition in DefaultWorkStealerPool under high contention.

[multiple]
[Init] Established base module structure for Spectra. Includes allocator interface, task engine,
```