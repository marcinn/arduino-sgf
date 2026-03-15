# Simple Games Factory (SGF)

SGF is a lightweight C++ support library for small embedded games. It provides timing, rendering, and utility building blocks without imposing a specific engine architecture. All headers are included with the `SGF/` prefix (e.g., `#include "SGF/TileFlusher.h"`).

## Components
- **Game**: Base loop with an internal frame clock. Exposes `start()`, `loop()`, `switchScene(...)`, and `resetActions()`. Derive from it and implement `onSetup()`, `onPhysics(float delta)`, and `onProcess(float delta)` to integrate your game logic and rendering.
- **Scene** / **SceneSwitcher**: Lightweight scene interface and dispatcher for title/gameplay/game-over style flows without dynamic allocation. `Game` owns one `SceneSwitcher` internally and delegates `onAction`, `onInput`, `onPhysics`, and `onProcess` to the active scene.
- **Actions**: Input state helpers built around `ActionState`, `ActionBinding`, and `InputEvent`. `Game` updates bound inputs, emits `pressed` / `justPressed` / `justReleased`, and `resetActions()` can resync action state to the current hardware snapshot without emitting edge events.
- **IRenderTarget**: Minimal interface for render targets (`size()`, `blit565(...)`) to decouple flushing from concrete display drivers.
- **TileFlusher**: Tile-based dirty-rect flusher. Takes `DirtyRects`, an `IRenderTarget`, and a tile render callback to repaint only modified regions in bounded tiles.
- **Sprites**: `Sprite`, `Missile`, and `SpriteLayer` provide fixed-slot software sprites over a background buffer. Gameplay code binds renderer sprite handles; the renderer owns sprite storage.
- **SpriteCharacter / SpriteRigidBody**: Small helpers that bind a renderer sprite handle to a `Character` or `RigidBody` and keep sprite position in sync with object position.
- **DirtyRects**: Simple registry of rectangles to refresh, with clip/merge helpers to reduce overdraw.
- **Collision**: Low-level geometry helpers using `Vector2i` inputs (`aabbHit`, `circleRectHit`, `raycastToRect`, etc.).
- **StaticBody / BodiesCollider / CollisionSystem**: `StaticBody` is a non-dynamic collidable with position, anchor, and shape. `BodiesCollider` resolves body-vs-body stages for configured bodies. `CollisionSystem` runs a fixed pipeline over statically provided bodies and colliders without dynamic allocation.
- **RigidBody / Physics**: `RigidBody` stores position, velocity, mass, forces, and `linearDamp`. `Physics` integrates motion, applies gravity, and can reflect velocity with `bounce(...)`; collision detection / clamping stays outside `Physics`.
- **ICollidable / ICollider / Tile colliders**: `ICollidable` provides collision position, shape, and body type. `ICollider` exposes collision queries plus `resolve(RigidBody&)`, and can participate in a `CollisionSystem` pipeline. `AreaCollider` handles world bounds with per-edge response. `CharTileCollider` is the byte-per-tile variant and `BitTileCollider` is the bit-per-tile variant for tightly packed tile maps.
- **CollisionDebug**: Debug overlay for `CollisionSystem`. Render it into the normal render pass through an `IFillRect` target so it participates in the same redraw pipeline as the rest of the frame.
- **Color565**: RGB565 helpers (`Color565::rgb(...)`, `Color565::lighten(...)`, `Color565::darken(...)`, `Color565::bswap(...)`).
- **FastILI9341**: Display driver for ILI9341 (blitting, backlight control, rotation).
- **Platform bus adapters**: Keep hardware/platform-specific `IDisplayBus` implementations in separate libraries such as `SGF_ESP32` or `SGF_ArduinoQ`, then include them explicitly from the sketch.
- **RectFlashAnim**: Utility for animating flashing rectangles, built on `DirtyRects`.
- **IFont / Font5x7 / FontRenderer**: `IFont` describes glyph metrics/rows, `Font5x7` is the built-in 5x7 bitmap font, and `FontRenderer` draws fonts to `IScreen` / `IFillRect`.
- **TextBlock**: Small text primitive that owns its content/style, marks `DirtyRects` on changes, and renders through `IFont` + `FontRenderer`.
- **Renderer2D + built-in scroll helper**: `Renderer2D` owns the 1D scroll helper internally and stitches together optional hardware scroll, background redraw, sprites, and dirty-rect tile flushing. The active on-screen axis depends on rotation (portrait: vertical, landscape: horizontal).

## Typical use
- Derive your game class from `Game`, override the three lifecycle hooks, and hold your state there.
- Bind hardware inputs with `ActionBinding` and `configureActions(...)`; `Game` will update the bound inputs and action states each frame.
- If you use scenes, let `Game` own scene dispatch and enter/switch scenes through `Game::switchScene(...)`.
- For rendering, adapt your display to `IRenderTarget` (or use a thin adapter) and use `TileFlusher` with a game-provided region renderer to redraw dirty areas efficiently.
- Render text through `FontRenderer`; display drivers stay display-only and do not provide `drawText(...)` helpers.
- Leverage `DirtyRects` to mark updates, `Collision` for basic geometry tests, `Color565` for colors, and pair a display driver such as `FastILI9341` with a platform-specific bus adapter.

## Physics Notes
- `Physics::integrate(...)` updates velocity and position from accumulated forces, gravity, mass, and `RigidBody::linearDamp`.
- `Physics::bounce(...)` only reflects velocity along a supplied collision normal; it does not clamp position or detect collisions.
- `Physics::resolveBodies(...)` is the simple rigid-vs-rigid resolver; it detects body collision internally and applies an impulse using both masses and a caller-supplied restitution.
- `AreaCollider::resolve(...)` is one simple way to clamp a body to world bounds and then invoke `Physics::bounce(...)`.
- `AreaCollider` supports separate left/right/top/bottom restitution and an optional `calculateResponse` callback to override the final edge response.
- `CharTileCollider` and `BitTileCollider` are collider hooks for tile-based worlds; both take a tile buffer, tile-map size, tile size in screen space, and raster bounds, then expose the same `resolve(RigidBody&)` entrypoint through `ICollider`.
- `CharTileCollider` decides solidity through `checkTileCollisionFn(uint8_t tile)`.
- `getCollision(...)` returns a `ColliderCollision` with corrected position, collision normal, tile coordinates, tile index, and buffer/bit offsets where applicable.
- `BodiesCollider` is the body-vs-body/body-vs-static stage for `CollisionSystem`; rigid-vs-rigid uses `Physics::resolveBodies(...)`, rigid-vs-static uses the same pipeline but only the rigid body is moved and bounced.

Minimal collision pipeline example:

```cpp
RigidBody playerBody;
RigidBody enemyBody;
StaticBody wallBody;
BodiesCollider bodiesCollider;
AreaCollider boundsCollider;
ICollidable* bodies[] = {&playerBody, &enemyBody, &wallBody};
ICollider* colliders[] = {&bodiesCollider, &boundsCollider};
CollisionSystem collisionSystem;

void setupCollisions() {
  playerBody.setCollisionShape(CollisionShape::rect(Vector2i{16, 16}));
  enemyBody.setCollisionShape(CollisionShape::circle(8));
  wallBody.setCollisionShape(CollisionShape::rect(Vector2i{32, 32}));
  wallBody.setPosition(Vector2f{80.0f, 40.0f});
  bodiesCollider.setRestitution(0.7f);
  collisionSystem.configureBodies(bodies, sizeof(bodies) / sizeof(bodies[0]));
  collisionSystem.configureColliders(colliders, sizeof(colliders) / sizeof(colliders[0]));
}

void tickCollisions() {
  collisionSystem.update(delta);
}
```

Debug overlay example inside a render pass:

```cpp
void renderRegion(BufferFillRect& fillRect) {
  CollisionDebug::drawSystem(collisionSystem, fillRect,
                             Color565::rgb(255, 0, 0),
                             Color565::rgb(0, 255, 0));
}
```

Do not draw collision debug directly to the screen outside the normal render pipeline, or the overlay will smear against stale background data.

## Text Rendering
Text rendering is split into three layers:
- `IFont`: font description (glyph metrics + row bits)
- `Font5x7`: built-in bitmap font implementing `IFont`
- `FontRenderer`: draws an `IFont` to `IScreen` or any `IFillRect`

Draw directly to a screen:

```cpp
#include "SGF/Font5x7.h"
#include "SGF/FontRenderer.h"

FontRenderer::drawText(FONT_5X7, gfx, 10, 20, "HELLO", 2, Color565::rgb(255, 255, 255));
FontRenderer::drawTextCentered(FONT_5X7, gfx, gfx.size().x / 2, 40, "READY", 2,
                               Color565::rgb(255, 255, 0));
```

Draw into a region buffer:

```cpp
#include "SGF/BufferFillRect.h"
#include "SGF/Font5x7.h"
#include "SGF/FontRenderer.h"

uint16_t regionBuf[64 * 16];
BufferFillRect fillRect(0, 0, 64, 16, regionBuf);
FontRenderer::drawText(FONT_5X7, fillRect, 0, 0, "HUD", 2, Color565::rgb(255, 255, 255));
```

## Audio
SGF audio is split into a few small pieces:
- `SynthEngine`: synthesized voices (`Sine`, `Triangle`, `Square`, `Saw`, `Noise`)
- `SamplePlayer`: PCM sample playback
- `AudioMixer`: mixes multiple audio sources into one output
- `SongPlayer` / `PatternTrack`: note scheduling
- platform output such as `ESP32DacAudioOutput`

### Minimal audio init
For ESP32, the smallest working setup is one mixer, one or more sources, and one output:

```cpp
#include "SGF_ESP32.h"
#include "SGF/AudioMixer.h"
#include "SGF/SamplePlayer.h"
#include "SGF/Synth.h"

SGFAudio::SynthEngine synth(11025u);
SGFAudio::SamplePlayer samples(11025u);
SGFAudio::AudioMixer mixer(11025u);
SGFAudio::ESP32DacAudioOutput audioOut(mixer, 25u);

void setupAudio() {
  mixer.addSource(synth);
  mixer.addSource(samples);
  audioOut.begin();
}
```

If a project only needs synth or only needs samples, you can add just one source to the mixer.

### Triggering a synth instrument on an event
Use `SynthEngine::noteOn(...)` for a note and `playSfx(...)` for a short synth effect:

```cpp
#include "SGF/AudioTypes.h"
#include "SGF/Synth.h"

SGFAudio::SynthEngine synth(11025u);

constexpr SGFAudio::Instrument kPickupInstrument{
  .waveform = SGFAudio::Waveform::Sine,
  .ampEnv = {4u, 24u, 180u, 40u},
  .volume = 140u,
};

void playPickup() {
  synth.noteOn(0, kPickupInstrument, 523.25f, 255u);
}
```

Short multi-step synth FX can use `Sfx`:

```cpp
constexpr SGFAudio::SfxStep kFireSteps[] = {
  {.durationMs = 40u, .semitoneOffset = 0, .volume = 255u, .gate = true, .retrigger = true},
  {.durationMs = 30u, .semitoneOffset = -7, .volume = 180u, .gate = true, .retrigger = false},
};

constexpr SGFAudio::Sfx kFireSfx{
  .instrument = &kPickupInstrument,
  .steps = kFireSteps,
  .stepCount = 2u,
};

void playFire() {
  synth.playSfx(kFireSfx, 440.0f, 255u);
}
```

### Triggering a sample instrument on an event
`SamplePlayer` uses `AudioSample` plus a lightweight `SampleInstrument` wrapper:

```cpp
#include "SGF/AudioTypes.h"
#include "SGF/SamplePlayer.h"

SGFAudio::SamplePlayer samples(11025u);

constexpr int8_t kClickPcm[] = {0, 48, 96, 64, 24, 0, -18, -8, 0};

constexpr SGFAudio::AudioSample kClickSample{
  .pcm = kClickPcm,
  .length = sizeof(kClickPcm) / sizeof(kClickPcm[0]),
  .sampleRate = 11025u,
  .rootHz = 1.0f,
  .loop = false,
  .loopStart = 0u,
  .loopEnd = 0u,
};

constexpr SGFAudio::SampleInstrument kClickInstrument{
  .sample = &kClickSample,
  .volume = 255u,
  .oneShot = true,
};

void playClick() {
  samples.playOneShot(kClickInstrument, 255u);
}
```

For pitched playback through patterns or songs, bind the sample through `makeProgramRef(...)`:

```cpp
constexpr SGFAudio::PatternStep kLeadSteps[] = {
  {.hz = 261.63f, .length = 1u, .velocity = 255u},
  {.hz = 329.63f, .length = 1u, .velocity = 255u},
  {.hz = 392.00f, .length = 1u, .velocity = 255u},
};

constexpr SGFAudio::Pattern kLeadPattern{
  .steps = kLeadSteps,
  .stepCount = 3u,
  .unitMs = 180u,
  .loop = true,
};

SGFAudio::PatternTrack sampleTrack(
  samples,
  0,
  SGFAudio::makeProgramRef(kClickInstrument),
  kLeadPattern
);
```

### Notes
- `AudioMixer` is the normal top-level source passed to the output backend.
- `ESP32DacAudioOutput` takes any `IAudioSource`, not just synth.
- For a full song, build lanes with `Song`, `SongLane`, `SongClip`, and `SongPlayer`.
- For projects with generated PCM assets, keep the generated lookup layer outside gameplay code and feed `AudioSample` pointers into `SampleInstrument`.

## SGF CLI
SGF ships with a small command-line helper at `tools/sgf`. It creates a project skeleton and wraps `arduino-cli` for build, upload, flash, monitor, and cleanup.

### Requirements
- `arduino-cli` must be available either in `PATH`, in a common system path such as `/usr/bin/arduino-cli` or `/usr/local/bin/arduino-cli`, or under an Arduino IDE directory passed explicitly as `arduino_ide=/path/to/arduino-ide`.
- Hardware preset definitions are resolved through the `sgf-hardware-presets` Arduino library.
- Project-local library overrides may be placed in `./libraries`, for example `./libraries/SGF` and `./libraries/sgf-hardware-presets`.

### Project layout
An SGF project is expected to have a single sketch entrypoint (`*.ino`) in the project root plus optional local source files. The entrypoint may define SGF metadata as comment lines:

```cpp
// sgf.name: MyGame
// sgf.boards: unoq, esp32
// sgf.default_board: unoq
// sgf.port.unoq: /dev/ttyACM0
// sgf.port.esp32: /dev/ttyUSB0

#include "SGFHardwarePresets.h"
```

Supported metadata keys:
- `sgf.name`: logical sketch name used for staging/build paths.
- `sgf.boards`: comma-separated board list enabled for this project.
- `sgf.default_board`: default board when `board=...` is not passed.
- `sgf.port.<board>`: default serial port for a given board.
- `sgf.fqbn.<board>`: overrides the default FQBN for a given board.
- `sgf.options.<board>`: overrides board options passed to `arduino-cli`.
- `sgf.preset.<board>`: overrides the `SGF_HW_PRESET` define for a given board.
- `sgf.extra_flags`: appended to build extra flags.
- `sgf.monitor_config`: default config passed to `arduino-cli monitor`.

### Creating a project
Create a new project skeleton:

```bash
/path/to/SGF/tools/sgf init MyGame
cd MyGame
```

This creates:
- `MyGame.ino`
- `.gitignore`

If you want to use local library checkouts instead of globally installed Arduino libraries, create:

```text
MyGame/
  libraries/
    SGF
    sgf-hardware-presets
```

These may be copies or symlinks.

### Building and flashing
Typical commands:

```bash
sgf build board=esp32
sgf flash board=esp32 port=/dev/ttyUSB0
sgf monitor board=esp32 port=/dev/ttyUSB0
sgf gentextures
sgf gensamples
sgf info
sgf clean board=esp32
```

If a project has `textures/` and/or `samples/`, `sgf build` runs the built-in asset generators automatically before compile. The defaults match the Wolf project:
- `textures/` -> `TexturesGenerated.h/.cpp`
- `samples/` + `samples.txt` -> `SamplesGenerated.h/.cpp`

You can also run the generators directly:

```bash
sgf gentextures
sgf gentextures --src alt-textures --out MyTexturesGenerated
sgf gensamples
sgf gensamples --src alt-samples --out MySamplesGenerated
```

Exported `ENABLE_*` variables are forwarded as preprocessor defines. `PHYSICS_TARGET_FPS`,
`RENDER_TARGET_FPS`, `SPI_FREQ`, and `DMA_BUS` are also forwarded. Example:

```bash
ENABLE_CTRL4B=1 sgf flash board=esp32 port=/dev/ttyUSB0
```

Useful flags during bring-up and profiling:
- `SPI_FREQ=60` sets SPI frequency to 60 MHz (the helper also accepts raw Hz values).
- `RENDER_TARGET_FPS=0` disables the render cap.
- `DMA_BUS=1` enables the alternate DMA bus define used by supported ESP32 ST7789 presets.
- `ENABLE_PROFILER=1` enables SGF `Profiler` + `SerialMonitor` output in the game loop.
- `ENABLE_FPS=1` enables the on-screen FPS overlay rendered by `Renderer2D`.

`PHYSICS_TARGET_FPS` and `RENDER_TARGET_FPS` default to `60`. They cap the `Game` physics tick rate and render rate independently. Set either to `0` to disable that limit.

If `arduino-cli` is bundled inside Arduino IDE and not available in `PATH`, pass the IDE root:

```bash
sgf build board=esp32 arduino_ide=/opt/arduino-ide
sgf flash board=esp32 port=/dev/ttyUSB0 arduino_ide=/opt/arduino-ide
```

Supported built-in board aliases are currently:
- `unoq`
- `esp32`

## Example: Game + Scene
Below is a minimal example showing a game host with a title scene and a play scene. The title scene starts the game on `FIRE`, while the play scene moves a rectangle and redraws only dirty regions.

```cpp
#include <Arduino.h>
#include "SGF_ArduinoQ.h"
#include "SGF/ActionBinding.h"
#include "SGF/ActionState.h"
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
  void onInput(const InputEvent& event) override;
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
      fireInput(firePin, true),
      actionBindings{{fireInput, fireAction}},
      titleScene(*this),
      playScene(*this) {}

  void setup() { start(); }

private:
  FastILI9341& gfx;
  DebouncedInputPin fireInput;
  ActionState fireAction;
  ActionBinding actionBindings[1];
  DirtyRects dirty;
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
    configureActions(actionBindings, 1);

    gfx.begin(24000000u);
    gfx.screenRotation(ScreenRotation::Landscape);
    gfx.fillScreen565(Color565::rgb(0, 0, 0));
    switchScene(titleScene);
  }

  void onPhysics(float delta) override {
    (void)delta;
  }

  void onProcess(float delta) override {
    (void)delta;
  }
};

void TitleScene::onEnter() {
  game.resetActions();
  game.gfx.fillScreen565(Color565::rgb(4, 8, 20));
}

void TitleScene::onInput(const InputEvent& event) {
  if (!event.isActionJustPressed(game.fireAction)) {
    return;
  }

  game.gfx.fillScreen565(Color565::rgb(0, 0, 0));
  game.switchScene(game.playScene);
}

void TitleScene::onPhysics(float delta) {
  (void)delta;
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
  if (game.boxX > game.gfx.size().x - 16) {
    game.boxX = game.gfx.size().x - 16;
    game.boxVX = -game.boxVX;
  }

  game.dirty.add(oldX, game.boxY, oldX + 15, game.boxY + 15);
  game.dirty.add(game.boxX, game.boxY, game.boxX + 15, game.boxY + 15);
}

void PlayScene::onProcess(float delta) {
  (void)delta;
  game.dirty.clip(game.gfx.size().x, game.gfx.size().y);
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
- `Game` owns frame timing and scene transitions; use `switchScene(...)` instead of touching the switcher directly from outside engine code.
- Use `resetActions()` on scene entry when you want to ignore inherited button edges from the previous scene.

## Example: Hardware scrolling with sprites (RiverRaid-style)
```cpp
#include "SGF_ArduinoQ.h"
#include "SGF/IRenderTarget.h"
#include "SGF/Renderer.h"
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
DirtyRects dirty;
Renderer2D renderer(display, dirty, 16, 16);

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
  updateSprites(sprites);                 // move sprites; Renderer2D auto-tracks sprite bounds
  renderer.flush(regionBuf);              // redraw only dirty tiles (background + sprites)
}
```

Key points:
- `scroll` uses the controller hardware scroll path when the target supports it; otherwise `Renderer2D` falls back to a full invalidate.
- Rotation defines the visible axis: portrait behaves like vertical scrolling, landscape behaves like horizontal scrolling.
- `setStripRenderer(...)` is optional. Without it, `Renderer2D` renders the exposed strip via `BackgroundFn`.
- If used, `StripFn` receives a `StripDesc` (axis + `w/h` + world origin) so the callback does not need direct access to a separate scroller object.
- `Renderer2D::scroll(...)` marks sprite ghost regions caused by hardware scroll.
- Sprite movement dirty rects are auto-tracked across `flush()` calls (`markSpriteMovement(...)` remains optional).
- `Renderer2D` combines background render, sprite overlay, and tile-based dirty flushing to minimize SPI traffic.
