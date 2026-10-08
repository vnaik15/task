# Repository Engineering Instructions

Applies to this entire repository. Follow these instructions for all changes, including AI-assisted work.

## Language and style
- Use the official or established language-specific style guide appropriate to the actual source files.
- Python: follow PEP 8 and PEP 257, use modern annotations (\`list[T]\`, \`dict[K, V]\`, union types), and validate with Ruff or equivalent.
- JavaScript/TypeScript: use StandardJS for JS unless existing configuration specifies a different established standard; TypeScript should use strict mode, no implicit \`any\`, and ESLint/Prettier where supported.
- HTML/CSS: semantic accessible markup, valid HTML, keyboard support, responsive layouts, consistent formatting.
- Respect .editorconfig and existing toolchain over arbitrary formatting churn.

## Architecture
- Keep functions focused, modules cohesive, dependencies explicit, and business logic separate from I/O and presentation.
- Make changes in small, reviewable, testable units; do not replace an entire file to implement a minor change.
- Prefer dependency injection and pure functions where practical. Avoid unexplained global mutable state and duplication.
- Firmware: document ISR and task ownership, concurrency, timing, memory constraints, peripheral setup, and hardware-specific assumptions; no dynamic allocation in interrupt contexts.
- Preserve backwards compatibility unless an API break or migration is explicitly requested.

## Safety and correctness
- Validate external input, ranges, buffers, encodings, units, and optional/null values.
- Handle invalid states and error paths explicitly; do not silently swallow exceptions or invent fallback data.
- Use parameterized database operations, safe output encoding, least privilege and environment-provided secrets. Never commit tokens, credentials, customer data or keys.
- Consider overflow, signedness, alignment, lifetime, races, timeout/retry behavior and resource cleanup when relevant.
- Add meaningful tests for nominal behavior, boundaries, failures, regressions and security-sensitive paths.

## Documentation and delivery
- Write concise documentation for public APIs and non-obvious design decisions; comments explain **why**, invariants and tradeoffs, not line-by-line **what**.
- When changing a feature, update associated tests and documentation in the same PR.
- Describe modified files, behavior, assumptions, tests run, and unverified limitations.
- When sharing implementation in chat, **always provide the complete updated file or component code** (not scattered fragments or partial patches) if code is requested. For repository commits, preserve concise diffs rather than duplicating unchanged source in comments.
- Never claim tests, builds, performance, security audits or deployments passed unless actually verified.
- Protect default branches with PR review; do not merge failing or unverified work.
