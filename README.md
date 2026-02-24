# Simple Games Factory (SGF)

SGF is a lightweight C++ support library for small embedded games. It provides timing, rendering, and utility building blocks without imposing a specific engine architecture. All headers are included with the `SGF/` prefix (e.g., `#include "SGF/TileFlusher.h"`).

## Components
- **Game**: Base loop with an internal frame clock. Exposes `start()`, `loop()`, and `resetClock()`. Derive from it and implement `onSetup()`, `onPhysics(float dtSec)`, and `onProcess(float dtSec)` to integrate your game logic and rendering.
- **IRenderTarget**: Minimal interface for render targets (`width()`, `height()`, `blit565(...)`) to decouple flushing from concrete display drivers.
- **TileFlusher**: Tile-based dirty-rect flusher. Takes `DirtyRects`, an `IRenderTarget`, and a tile render callback to repaint only modified regions in bounded tiles.
- **Sprites**: Software sprite layer with fixed slots (sprites + missiles), transparent key, and simple horizontal scaling modes; intended to be composed over a background buffer.
- **DirtyRects**: Simple registry of rectangles to refresh, with clip/merge helpers to reduce overdraw.
- **Collision**: Collision helpers, including circle-rectangle intersection.
- **FastILI9341**: Display driver for ILI9341 (blitting, backlight control, rotation) plus color helpers (`rgb565`, `lighten565`, `darken565`).
- **RectFlashAnim**: Utility for animating flashing rectangles, built on `DirtyRects`.
- **Font5x7**: Fixed 5x7 bitmap font routines (width calculation, pixel sampling, drawing).

## Typical use
- Derive your game class from `Game`, override the three lifecycle hooks, and hold your state there.
- For rendering, adapt your display to `IRenderTarget` (or use a thin adapter) and use `TileFlusher` with a game-provided region renderer to redraw dirty areas efficiently.
- Leverage `DirtyRects` to mark updates, `Collision` for basic geometry tests, and `FastILI9341` utilities for color and display control when targeting that controller.
