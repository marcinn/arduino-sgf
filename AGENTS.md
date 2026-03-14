# AGENTS.md

## Local collaboration rules
- Change only what the user asks for; do not touch unrelated user edits.
- No side refactors/cleanups beyond the requested scope.
- If a change spans multiple areas of a file, briefly state what will be altered before editing.
- Keep the project’s style/structure; do not impose your own organization unasked.
- Follow `CODING_STANDARDS.md` for any C++/Arduino code changes.
- Obey every single rule in `CODING_STANDARDS.md` without exception; treat each point as mandatory, not advisory.
- If the user gives a hard requirement to replace bad API/design, remove the bad API instead of keeping compatibility overloads, adapters, or transitional wrappers.
- Core structure is described in `README.md`; consult it when touching shared components.
- Before adding ad-hoc math helpers or inline math logic, check `src/SGF/Math.h` and reuse existing functions when they fit.
- Prefer small, precise patches over sweeping rewrites.
- Ask when something is unclear instead of guessing.
- No false statements and no lies: if unsure, say so; verify compliance before stating it and double-check work instead of assuming.
