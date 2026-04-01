#include "FontRenderer.h"

#include "IFillRect.h"
#include "IFont.h"
#include "IScreen.h"

namespace {

int alignedTextX(int anchorX, int textWidth, FontRenderer::AlignX align) {
    switch (align) {
        case FontRenderer::AlignX::Center:
            return anchorX - (textWidth / 2);
        case FontRenderer::AlignX::Right:
            return anchorX - textWidth + 1;
        case FontRenderer::AlignX::Left:
        default:
            return anchorX;
    }
}

}  // namespace

int FontRenderer::textWidth(const IFont& font, const char* text, int scale) {
    if (text == nullptr || scale <= 0) {
        return 0;
    }

    int length = 0;
    for (const char* p = text; *p != '\0'; ++p) {
        length++;
    }
    if (length == 0) {
        return 0;
    }

    return length * font.glyphAdvance() * scale - scale;
}

void FontRenderer::drawText(const IFont& font,
                            IFillRect& fillRect,
                            int x,
                            int y,
                            const char* text,
                            int scale,
                            uint16_t color565) {
    if (text == nullptr || scale <= 0) {
        return;
    }

    int cx = x;
    for (const char* p = text; *p != '\0'; ++p, cx += font.glyphAdvance() * scale) {
        for (int row = 0; row < font.glyphHeight(); ++row) {
            uint8_t rowBits = font.glyphRowBits(*p, row);
            int runStart = -1;
            for (int col = 0; col < font.glyphWidth(); ++col) {
                bool pixelSet = (rowBits & (1u << (font.glyphWidth() - 1 - col))) != 0;
                if (pixelSet) {
                    if (runStart < 0) {
                        runStart = col;
                    }
                    continue;
                }
                if (runStart >= 0) {
                    int runWidth = col - runStart;
                    fillRect.fillRect565(cx + runStart * scale,
                                         y + row * scale,
                                         runWidth * scale,
                                         scale,
                                         color565);
                    runStart = -1;
                }
            }
            if (runStart >= 0) {
                int runWidth = font.glyphWidth() - runStart;
                fillRect.fillRect565(cx + runStart * scale,
                                     y + row * scale,
                                     runWidth * scale,
                                     scale,
                                     color565);
            }
        }
    }
}

void FontRenderer::drawText(const IFont& font,
                            IScreen& screen,
                            int x,
                            int y,
                            const char* text,
                            int scale,
                            uint16_t color565) {
    drawText(font, (IFillRect&)(screen), x, y, text, scale, color565);
}

void FontRenderer::drawTextBlock(const IFont& font,
                                 IFillRect& fillRect,
                                 int anchorX,
                                 int y,
                                 const char* text,
                                 int scale,
                                 uint16_t color565,
                                 AlignX align) {
    int x = alignedTextX(anchorX, textWidth(font, text, scale), align);
    drawText(font, fillRect, x, y, text, scale, color565);
}

void FontRenderer::drawTextBlock(const IFont& font,
                                 IScreen& screen,
                                 int anchorX,
                                 int y,
                                 const char* text,
                                 int scale,
                                 uint16_t color565,
                                 AlignX align) {
    drawTextBlock(font, (IFillRect&)(screen), anchorX, y, text, scale, color565,
                  align);
}

void FontRenderer::drawTextCentered(const IFont& font,
                                    IFillRect& fillRect,
                                    int centerX,
                                    int y,
                                    const char* text,
                                    int scale,
                                    uint16_t color565) {
    drawTextBlock(font, fillRect, centerX, y, text, scale, color565, AlignX::Center);
}

void FontRenderer::drawTextCentered(const IFont& font,
                                    IScreen& screen,
                                    int centerX,
                                    int y,
                                    const char* text,
                                    int scale,
                                    uint16_t color565) {
    drawTextBlock(font, screen, centerX, y, text, scale, color565, AlignX::Center);
}
