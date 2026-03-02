#pragma once

#include <stdint.h>

#include <array>

#include "Vector2.h"

// Simple software sprites layer; meant to be composed over a background buffer.
// Clients fill sprite/missile slots and call renderRegion(...) after the background
// for a region is written into the buffer.
class SpriteLayer {
   public:
    enum class Scale {
        Normal,
        DoubleX,
        DoubleY,
        Double,
        QuadX,
        QuadY,
        Quad,
    };

    struct Sprite {
        bool active = false;
        Vector2i position{};
        int w = 0;
        int h = 0;
        const uint16_t* pixels565 = nullptr;
        uint16_t transparent = 0;  // pixels matching this value are skipped
        Scale scale = Scale::Normal;
        Vector2f anchor{};  // 0.0=left/top, 1.0=right/bottom (can be outside range)

        void setAnchor(const Vector2f& newAnchor) { anchor = newAnchor; }

        void setScale(Scale newScale) {
            if (scale == newScale) return;
            scale = newScale;
            redraw();
        }

        void setBitmap(const uint16_t* pixels, int width, int height) {
            bool changed = (pixels565 != pixels) || (w != width) || (h != height);
            pixels565 = pixels;
            w = width;
            h = height;
            if (changed) {
                redraw();
            }
        }

        void setBitmap(const uint16_t* pixels, int width, int height, uint16_t transparentColor) {
            bool changed = (transparent != transparentColor);
            setBitmap(pixels, width, height);
            transparent = transparentColor;
            if (changed) {
                redraw();
            }
        }

        void setTransparent(uint16_t transparentColor) {
            if (transparent == transparentColor) return;
            transparent = transparentColor;
            redraw();
        }

        void setActive(bool enabled) {
            if (active == enabled) return;
            active = enabled;
            redraw();
        }

        void setPosition(const Vector2i& newPosition) { position = newPosition; }

        void translate(const Vector2i& delta) { position += delta; }

        // Schedules redraw of the current bounds on the next Renderer2D::flush().
        // Use this when visual content changes without changing sprite bounds.
        void redraw() { ++redrawRevision_; }

        uint32_t redrawRevision() const { return redrawRevision_; }

       private:
        uint32_t redrawRevision_ = 1;
    };

    struct Missile {
        bool active = false;
        int x = 0;
        int y = 0;
        int w = 1;  // typically 1-2 px
        int h = 0;
        uint16_t color = 0xFFFF;
        Scale scale = Scale::Normal;
    };

    static constexpr int kMaxSprites = 8;
    static constexpr int kMaxMissiles = 4;

    SpriteLayer();

    void clearSprites();
    void clearMissiles();
    void clearAll();

    Sprite& sprite(int index);
    Missile& missile(int index);
    static void spriteBounds(const Sprite& s, int* x0, int* y0, int* x1, int* y1);
    static void spriteBounds(const Sprite& s, int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1);
    static void spriteBoundsPadded(const Sprite& s, int pad, int* x0, int* y0, int* x1, int* y1);

    void renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const;

   private:
    std::array<Sprite, kMaxSprites> sprites_{};
    std::array<Missile, kMaxMissiles> missiles_{};
};
