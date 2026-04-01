# AGENTS.md

## Local collaboration rules
- Change only what the user asks for; do not touch unrelated user edits.
- No side refactors/cleanups beyond the requested scope.
- If a change spans multiple areas of a file, briefly state what will be altered before editing.
- Keep the project’s style/structure; do not impose your own organization unasked.
- Stop generating fucking bloat code.
- Do not cast without a reason.
- If a cast is truly necessary, use the shortest correct idiom (`std::move`, a compact `static_cast`, or a tiny typed helper) instead of bloated conversion scaffolding.
- Do not replace one cast style mechanically with another.
- Keep code compact; do not expand simple logic into multiple temporaries unless that materially improves correctness or readability.
- If a knob is expected to be tuned from build defines, do not hide it in `static constexpr` or file-local tuning constants. Keep it overrideable from the build.
- Follow `CODING_STANDARDS.md` for any C++/Arduino code changes.
- Obey every single rule in `CODING_STANDARDS.md` without exception; treat each point as mandatory, not advisory.
- If the user gives a hard requirement to replace bad API/design, remove the bad API instead of keeping compatibility overloads, adapters, or transitional wrappers.
- Core structure is described in `README.md`; consult it when touching shared components.
- Before adding ad-hoc math helpers or inline math logic, check `src/SGF/Math.h` and reuse existing functions when they fit.
- Prefer small, precise patches over sweeping rewrites.
- Ask when something is unclear instead of guessing.
- No false statements and no lies: if unsure, say so; verify compliance before stating it and double-check work instead of assuming.
