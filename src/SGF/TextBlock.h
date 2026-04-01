#pragma once

#include <stddef.h>
#include <stdint.h>

#include "BufferFillRect.h"
#include "DirtyRects.h"
#include "FontRenderer.h"
#include "IFont.h"

class TextBlock {
   public:
    using AlignX = FontRenderer::AlignX;

    static constexpr size_t MAX_TEXT_LEN = 63u;

    explicit TextBlock(DirtyRects& dirty);

    void setText(const char* text);
    const char* text() const { return text_; }

    void setPosition(int x, int y);
    void setScale(int scale);
    void setColor(uint16_t color565);
    void setAlignX(AlignX align);
    void setFont(const IFont* font);
    void setVisible(bool visible);

    int x() const { return posX_; }
    int y() const { return posY_; }
    int scale() const { return scale_; }
    uint16_t color() const { return color565_; }
    AlignX alignX() const { return alignX_; }
    const IFont* font() const { return font_; }
    bool visible() const { return visible_; }

    int width() const;
    int height() const;
    void bounds(Rect* out) const;
    void render(int x0, int y0, int w, int h, uint16_t* buf) const;

   private:
    DirtyRects& dirty_;
    const IFont* font_ = nullptr;
    AlignX alignX_ = AlignX::Left;
    int posX_ = 0;
    int posY_ = 0;
    int scale_ = 1;
    uint16_t color565_ = 0xFFFFu;
    bool visible_ = true;
    char text_[MAX_TEXT_LEN + 1] = {};

    bool hasVisibleText() const;
    int drawX() const;
    void markDirtyChange(const Rect& oldBounds);
};
