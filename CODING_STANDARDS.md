# CODING_STANDARDS.md

Minimal C++/Arduino guidelines for this repo:
- Naming: classes/structs `PascalCase`, functions/methods `lowerCamelCase`, variables/members `lowerCamelCase` (no trailing underscores); constants `SCREAMING_SNAKE_CASE` using `constexpr` when possible.
- Every class should have .h and .cpp files
- There cannot be more than one public clasess in a file
- Braces on the same line as the header (`if (...) {`), always use braces even for single statements.
- Prefer `const`/`constexpr` and references over pointers when `nullptr` is not expected.
- Avoid dynamic allocation; use static/stack buffers sized for the MCU constraints.
- Includes: project/local headers in `""`, platform/stdlib in `<...>`; include only what you need.
- Formatting: 4-space indent, keep lines short (< 100 chars), use spaces after commas and around operators.
- Prefer object-centric APIs over exploded argument lists: pass domain objects (e.g. `Paddle`) instead of `x/w/y` bundles when the callee needs that object’s state.
- Object-specific behavior/config belongs to the object class (e.g. `Ball` launch defaults/bounce rules, `Paddle` movement params), not to `ArkanoidGame`.
- Avoid redundant wrapper/delegate methods and duplicated logic; if a method only forwards state/behavior without adding value, remove it or move logic to the owning class.
- Runtime update methods should update runtime state only; configure immutable/rarely-changing sprite data (`w/h/pixels/scale`) in bind/init methods, not every frame.
- Sprite rendering internals (scale, anchor, bounds/dirty rect computation) belong to `SpriteLayer`/sprite code, not gameplay objects (`Ball`, `Paddle`, scenes).
- When syncing render state from gameplay objects, expose simple intent-level methods (`setPosition`, `translate`, `syncSprite`) instead of leaking render math into game code.
- Keep naming consistent across engine/game lifecycle methods (`onPhysics`, `onProcess`, `delta`) and avoid parallel synonyms for the same concept.
- If a class already satisfies a project interface (e.g. `IRenderTarget`), implement that interface directly instead of creating local adapter structs just to forward the same methods.
- Prefer effect names over vague state names in APIs (`invalidate(...)` over ambiguous names like `setFull(...)` when the method clears and marks redraw state).
- Mutable object state should be private by default (especially position/state fields like `x/y`); expose behavior methods and explicit accessors (e.g. `getPosition()`) instead of public field writes.
- Reuse common math/value types (e.g. `Vector2` for 2D position/size) instead of duplicating one-off `Position`/`Size` structs across classes.
- If object state change has invariants/side effects (e.g. syncing a bound sprite), the state must be changed through the object API (`setPosition`, `setX`, etc.), not by external direct field mutation.
- Do not add central "sync all objects" sweeps to compensate for leaked state mutation. Update dependent state at the point where the owning object state changes.
