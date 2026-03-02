#include "TextBlock.h"

#include "Font5x7.h"
#include "FontRenderer.h"

#include <string.h>

TextBlock::TextBlock(DirtyRects& dirty) : dirty_(dirty), font_(&FONT_5X7) {}

void TextBlock::setText(const char* text) {
    const char* nextText = text ? text : "";
    if (strncmp(text_, nextText, MAX_TEXT_LEN) == 0 && strlen(text_) == strlen(nextText)) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    strncpy(text_, nextText, MAX_TEXT_LEN);
    text_[MAX_TEXT_LEN] = '\0';
    markDirtyChange(oldBounds);
}

void TextBlock::setPosition(int x, int y) {
    if (posX_ == x && posY_ == y) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    posX_ = x;
    posY_ = y;
    markDirtyChange(oldBounds);
}

void TextBlock::setScale(int scale) {
    int nextScale = scale > 0 ? scale : 1;
    if (scale_ == nextScale) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    scale_ = nextScale;
    markDirtyChange(oldBounds);
}

void TextBlock::setColor(uint16_t color565) {
    if (color565_ == color565) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    color565_ = color565;
    markDirtyChange(oldBounds);
}

void TextBlock::setAlignX(AlignX align) {
    if (alignX_ == align) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    alignX_ = align;
    markDirtyChange(oldBounds);
}

void TextBlock::setFont(const IFont* font) {
    const IFont* nextFont = font ? font : &FONT_5X7;
    if (font_ == nextFont) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    font_ = nextFont;
    markDirtyChange(oldBounds);
}

void TextBlock::setVisible(bool visible) {
    if (visible_ == visible) {
        return;
    }

    Rect oldBounds{};
    bounds(&oldBounds);
    visible_ = visible;
    markDirtyChange(oldBounds);
}

int TextBlock::width() const {
    return hasVisibleText() ? FontRenderer::textWidth(*font_, text_, scale_) : 0;
}

int TextBlock::height() const { return hasVisibleText() ? font_->glyphHeight() * scale_ : 0; }

void TextBlock::bounds(Rect* out) const {
    if (!out) {
        return;
    }

    if (!hasVisibleText()) {
        *out = Rect{0, 0, -1, -1};
        return;
    }

    int textW = width();
    int textH = height();
    int x0 = drawX();
    int y0 = posY_;
    *out = Rect{
        (int16_t)x0,
        (int16_t)y0,
        (int16_t)(x0 + textW - 1),
        (int16_t)(y0 + textH - 1),
    };
}

void TextBlock::render(int x0, int y0, int w, int h, uint16_t* buf) const {
    if (!hasVisibleText() || !buf || w <= 0 || h <= 0) {
        return;
    }

    BufferFillRect fillRect(x0, y0, w, h, buf);
    FontRenderer::drawText(*font_, fillRect, drawX(), posY_, text_, scale_, color565_);
}

bool TextBlock::hasVisibleText() const {
    return visible_ && text_[0] != '\0' && font_ != nullptr && scale_ > 0 &&
           font_->glyphHeight() > 0;
}

int TextBlock::drawX() const {
    int textW = hasVisibleText() ? FontRenderer::textWidth(*font_, text_, scale_) : 0;
    switch (alignX_) {
        case AlignX::Center:
            return posX_ - (textW / 2);
        case AlignX::Right:
            return posX_ - textW + 1;
        case AlignX::Left:
        default:
            return posX_;
    }
}

void TextBlock::markDirtyChange(const Rect& oldBounds) {
    if (oldBounds.x1 >= oldBounds.x0 && oldBounds.y1 >= oldBounds.y0) {
        dirty_.add(oldBounds.x0, oldBounds.y0, oldBounds.x1, oldBounds.y1);
    }

    Rect newBounds{};
    bounds(&newBounds);
    if (newBounds.x1 >= newBounds.x0 && newBounds.y1 >= newBounds.y0) {
        dirty_.add(newBounds.x0, newBounds.y0, newBounds.x1, newBounds.y1);
    }
}
