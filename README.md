# Simple Games Factory (SGF)

SGF is a lightweight C++ support library for small embedded games. It provides timing, rendering, and utility building blocks without imposing a specific engine architecture. All headers are included with the `SGF/` prefix (e.g., `#include "SGF/TileFlusher.h"`).

## Components
- **Game**: Base loop with an internal frame clock. Exposes `start()`, `loop()`, and `resetClock()`. Derive from it and implement `onSetup()`, `onPhysics(float delta)`, and `onProcess(float delta)` to integrate your game logic and rendering.
- **Scene** / **SceneSwitcher**: Lightweight scene interface and dispatcher for title/gameplay/game-over style flows without dynamic allocation.
- **Actions**: Small input helpers (`DigitalAction`, `PressReleaseAction`) for `pressed` / `justPressed` / confirm-style handling.
- **IRenderTarget**: Minimal interface for render targets (`width()`, `height()`, `blit565(...)`) to decouple flushing from concrete display drivers.
- **TileFlusher**: Tile-based dirty-rect flusher. Takes `DirtyRects`, an `IRenderTarget`, and a tile render callback to repaint only modified regions in bounded tiles.
- **Sprites**: Software sprite layer with fixed slots (sprites + missiles), transparent key, and simple horizontal scaling modes; intended to be composed over a background buffer.
- **DirtyRects**: Simple registry of rectangles to refresh, with clip/merge helpers to reduce overdraw.
- **Collision**: Collision helpers, including circle-rectangle intersection.
- **Color565**: RGB565 helpers (`Color565::rgb(...)`, `Color565::lighten(...)`, `Color565::darken(...)`, `Color565::bswap(...)`).
- **FastILI9341**: Display driver for ILI9341 (blitting, backlight control, rotation).
- **Platform bus adapters**: Keep hardware/platform-specific `IDisplayBus` implementations in separate libraries such as `SGF_ESP32` or `SGF_ArduinoQ`, then include them explicitly from the sketch.
- **RectFlashAnim**: Utility for animating flashing rectangles, built on `DirtyRects`.
- **Font5x7**: Fixed 5x7 bitmap font routines (width calculation, pixel sampling, drawing).
- **Renderer + built-in scroll helper**: `Renderer` owns the 1D scroll helper internally and stitches together optional hardware scroll, background redraw, sprites, and dirty-rect tile flushing. The active on-screen axis depends on rotation (portrait: vertical, landscape: horizontal).

## Typical use
- Derive your game class from `Game`, override the three lifecycle hooks, and hold your state there.
- For rendering, adapt your display to `IRenderTarget` (or use a thin adapter) and use `TileFlusher` with a game-provided region renderer to redraw dirty areas efficiently.
- Leverage `DirtyRects` to mark updates, `Collision` for basic geometry tests, `Color565` for colors, and pair a display driver such as `FastILI9341` with a platform-specific bus adapter.

## Example: Game + Scene
Below is a minimal example showing a game host with a title scene and a play scene. The title scene starts the game on `FIRE`, while the play scene moves a rectangle and redraws only dirty regions.

```cpp
#include <Arduino.h>
#include "SGF_ArduinoQ.h"
#include "SGF/Actions.h"
#include "SGF/Color565.h"
#include "SGF/DirtyRects.h"
#include "SGF/Game.h"
#include "SGF/Scene.h"

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_LED D6

const SPIArduinoQDisplayBus::Config DISPLAY_BUS_CONFIG = {
  DEVICE_DT_GET(DT_NODELABEL(spi2)),
  TFT_CS,
  TFT_DC,
  TFT_RST,
  TFT_LED
};

SPIArduinoQDisplayBus displayBus(DISPLAY_BUS_CONFIG);
FastILI9341 gfx(displayBus);

class MiniGame;

class TitleScene : public Scene {
public:
  explicit TitleScene(MiniGame& game) : game(game) {}
  void onEnter() override;
  void onPhysics(float delta) override;
  void onProcess(float delta) override;

private:
  MiniGame& game;
};

class PlayScene : public Scene {
public:
  explicit PlayScene(MiniGame& game) : game(game) {}
  void onEnter() override;
  void onPhysics(float delta) override;
  void onProcess(float delta) override;

private:
  MiniGame& game;
};

class MiniGame : public Game {
public:
  MiniGame(FastILI9341& gfx, uint8_t firePin)
    : Game(10000u, 30000u),
      gfx(gfx),
      pinFire(firePin),
      titleScene(*this),
      playScene(*this) {}

  void setup() { start(); }

private:
  FastILI9341& gfx;
  uint8_t pinFire = 0;
  DigitalAction fireAction;
  PressReleaseAction fireConfirm;
  DirtyRects dirty;
  SceneSwitcher sceneSwitcher;
  TitleScene titleScene;
  PlayScene playScene;

  int boxX = 20;
  int boxY = 120;
  int boxVX = 100;  // px/s

  friend class TitleScene;
  friend class PlayScene;

  void fillRect(int x0, int y0, int w, int h, uint16_t color565) {
    if (w <= 0 || h <= 0) {
      return;
    }
    static constexpr int TILE_W = 16;
    static constexpr int TILE_H = 16;
    static uint16_t buf[TILE_W * TILE_H];
    for (int i = 0; i < TILE_W * TILE_H; ++i) {
      buf[i] = color565;
    }

    for (int ty = 0; ty < h; ty += TILE_H) {
      int hh = min(TILE_H, h - ty);
      for (int tx = 0; tx < w; tx += TILE_W) {
        int ww = min(TILE_W, w - tx);
        gfx.blit565(x0 + tx, y0 + ty, ww, hh, buf);
      }
    }
  }

  void onSetup() override {
    pinMode(pinFire, INPUT_PULLUP);
    fireAction.reset(digitalRead(pinFire) == LOW);

    gfx.begin(24000000u);
    gfx.screenRotation(FastILI9341::ScreenRotation::Landscape);
    gfx.fillScreen565(Color565::rgb(0, 0, 0));
    sceneSwitcher.setInitial(titleScene);
    resetClock();
  }

  void onPhysics(float delta) override {
    fireAction.update(digitalRead(pinFire) == LOW);
    sceneSwitcher.onPhysics(delta);
  }

  void onProcess(float delta) override {
    sceneSwitcher.onProcess(delta);
  }
};

void TitleScene::onEnter() {
  game.fireConfirm.reset();
  game.gfx.fillScreen565(Color565::rgb(4, 8, 20));
}

void TitleScene::onPhysics(float delta) {
  (void)delta;
  if (game.fireConfirm.update(game.fireAction)) {
    game.gfx.fillScreen565(Color565::rgb(0, 0, 0));
    game.sceneSwitcher.switchTo(game.playScene);
    game.resetClock();
  }
}

void TitleScene::onProcess(float delta) {
  (void)delta;
}

void PlayScene::onEnter() {
  game.boxX = 20;
  game.boxY = 120;
  game.gfx.fillScreen565(Color565::rgb(0, 0, 0));
}

void PlayScene::onPhysics(float delta) {
  int oldX = game.boxX;
  game.boxX += (int)(game.boxVX * delta);
  if (game.boxX < 0) {
    game.boxX = 0;
    game.boxVX = -game.boxVX;
  }
  if (game.boxX > game.gfx.width() - 16) {
    game.boxX = game.gfx.width() - 16;
    game.boxVX = -game.boxVX;
  }

  game.dirty.add(oldX, game.boxY, oldX + 15, game.boxY + 15);
  game.dirty.add(game.boxX, game.boxY, game.boxX + 15, game.boxY + 15);
}

void PlayScene::onProcess(float delta) {
  (void)delta;
  game.dirty.clip(game.gfx.width(), game.gfx.height());
  game.dirty.mergeAll();
  for (int i = 0; i < game.dirty.count(); ++i) {
    const Rect& r = game.dirty[i];
    game.fillRect(r.x0, r.y0, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1, Color565::rgb(0, 0, 0));
  }
  game.dirty.clear();
  game.fillRect(game.boxX, game.boxY, 16, 16, Color565::rgb(64, 200, 255));
}
```

Notes:
- The host `MiniGame` owns shared state (display, input, scene switcher).
- Scenes are regular classes with composition (`MiniGame& game`), not subclasses of the game.
- Keep scene transitions in `onPhysics(...)`, and rendering in `onProcess(...)`.

## Example: Hardware scrolling with sprites (RiverRaid-style)
```cpp
#include "SGF_ArduinoQ.h"
#include "SGF/IRenderTarget.h"
#include "SGF/Renderer.h"
#include "SGF/Sprites.h"
#include "SGF/DirtyRects.h"

SPIArduinoQDisplayBus::Config displayBusConfig = {
  DEVICE_DT_GET(DT_NODELABEL(spi2)),
  TFT_CS,
  TFT_DC,
  TFT_RST,
  TFT_LED
};
SPIArduinoQDisplayBus displayBus(displayBusConfig);
FastILI9341 display(displayBus);
SpriteLayer sprites;
DirtyRects dirty;
Renderer renderer(display, sprites, dirty, 16, 16);

uint16_t stripBuf[320 * 16];
uint16_t regionBuf[16 * 16];

void setup() {
  // Initialize your display driver and rotation before configuring scroll.
  renderer.configureFullScreenScroll();

  renderer.setBackgroundRenderer(
    [&](int x0,int y0,int w,int h,int32_t wx,int32_t wy,uint16_t* buf){
      drawBackground(wx, wy, w, h, buf);   // active scroll axis is already mapped into wx/wy
    });
}

void loopFrame(int d) { // d>0 moves forward on the active scroll axis
  renderer.scroll(d, stripBuf, 16);       // hardware scroll + sprite ghost cleanup
  updateSprites(sprites);                 // move sprites; Renderer auto-tracks sprite bounds
  renderer.flush(regionBuf);              // redraw only dirty tiles (background + sprites)
}
```

Key points:
- `scroll` uses the controller hardware scroll path when the target supports it; otherwise `Renderer` falls back to a full invalidate.
- Rotation defines the visible axis: portrait behaves like vertical scrolling, landscape behaves like horizontal scrolling.
- `setStripRenderer(...)` is optional. Without it, `Renderer` renders the exposed strip via `BackgroundFn`.
- If used, `StripFn` receives a `StripDesc` (axis + `w/h` + world origin) so the callback does not need direct access to a separate scroller object.
- `Renderer::scroll(...)` marks sprite ghost regions caused by hardware scroll.
- Sprite movement dirty rects are auto-tracked across `flush()` calls (`markSpriteMovement(...)` remains optional).
- `Renderer` combines background render, sprite overlay, and tile-based dirty flushing to minimize SPI traffic.
