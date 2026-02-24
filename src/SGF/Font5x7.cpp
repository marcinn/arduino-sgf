#include "Font5x7.h"

namespace Font5x7 {

static const uint8_t* glyph(char ch) {
  static const uint8_t GLYPH_SPACE[7] = {0,0,0,0,0,0,0};
  static const uint8_t GLYPH_BALL[7]  = {0x00,0x0E,0x1F,0x1F,0x1F,0x0E,0x00};
  static const uint8_t GLYPH_0[7] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E};
  static const uint8_t GLYPH_1[7] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E};
  static const uint8_t GLYPH_2[7] = {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F};
  static const uint8_t GLYPH_3[7] = {0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E};
  static const uint8_t GLYPH_4[7] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02};
  static const uint8_t GLYPH_5[7] = {0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E};
  static const uint8_t GLYPH_6[7] = {0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E};
  static const uint8_t GLYPH_7[7] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08};
  static const uint8_t GLYPH_8[7] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E};
  static const uint8_t GLYPH_9[7] = {0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E};
  static const uint8_t GLYPH_A[7] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11};
  static const uint8_t GLYPH_B[7] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E};
  static const uint8_t GLYPH_C[7] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
  static const uint8_t GLYPH_D[7] = {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E};
  static const uint8_t GLYPH_E[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F};
  static const uint8_t GLYPH_F[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10};
  static const uint8_t GLYPH_G[7] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F};
  static const uint8_t GLYPH_H[7] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11};
  static const uint8_t GLYPH_I[7] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E};
  static const uint8_t GLYPH_J[7] = {0x01,0x01,0x01,0x01,0x11,0x11,0x0E};
  static const uint8_t GLYPH_K[7] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11};
  static const uint8_t GLYPH_L[7] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
  static const uint8_t GLYPH_M[7] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11};
  static const uint8_t GLYPH_N[7] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11};
  static const uint8_t GLYPH_O[7] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E};
  static const uint8_t GLYPH_P[7] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10};
  static const uint8_t GLYPH_Q[7] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D};
  static const uint8_t GLYPH_R[7] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11};
  static const uint8_t GLYPH_S[7] = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E};
  static const uint8_t GLYPH_T[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04};
  static const uint8_t GLYPH_U[7] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
  static const uint8_t GLYPH_V[7] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04};
  static const uint8_t GLYPH_W[7] = {0x11,0x11,0x11,0x15,0x15,0x1B,0x11};
  static const uint8_t GLYPH_X[7] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11};
  static const uint8_t GLYPH_Y[7] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04};
  static const uint8_t GLYPH_Z[7] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F};

  switch (ch) {
    case '0': return GLYPH_0;
    case '1': return GLYPH_1;
    case '2': return GLYPH_2;
    case '3': return GLYPH_3;
    case '4': return GLYPH_4;
    case '5': return GLYPH_5;
    case '6': return GLYPH_6;
    case '7': return GLYPH_7;
    case '8': return GLYPH_8;
    case '9': return GLYPH_9;
    case 'A': return GLYPH_A;
    case 'B': return GLYPH_B;
    case 'C': return GLYPH_C;
    case 'D': return GLYPH_D;
    case 'E': return GLYPH_E;
    case 'F': return GLYPH_F;
    case 'G': return GLYPH_G;
    case 'H': return GLYPH_H;
    case 'I': return GLYPH_I;
    case 'J': return GLYPH_J;
    case 'K': return GLYPH_K;
    case 'L': return GLYPH_L;
    case 'M': return GLYPH_M;
    case 'N': return GLYPH_N;
    case 'O': return GLYPH_O;
    case 'P': return GLYPH_P;
    case 'Q': return GLYPH_Q;
    case 'R': return GLYPH_R;
    case 'S': return GLYPH_S;
    case 'T': return GLYPH_T;
    case 'U': return GLYPH_U;
    case 'V': return GLYPH_V;
    case 'W': return GLYPH_W;
    case 'X': return GLYPH_X;
    case 'Y': return GLYPH_Y;
    case 'Z': return GLYPH_Z;
    case ' ': return GLYPH_SPACE;
    case '*': return GLYPH_BALL;
    default:  return GLYPH_SPACE;
  }
}

int textWidth(const char* s, int scale) {
  int n = 0;
  for (const char* p = s; *p; ++p) n++;
  if (n == 0) return 0;
  return n * (5 * scale + scale) - scale;
}

bool textPixel(const char* s, int scale, int x, int y) {
  if (!s || scale <= 0 || x < 0 || y < 0) return false;

  const int charW = 5 * scale;
  const int advance = charW + scale;
  const int row = y / scale;
  if (row < 0 || row >= 7) return false;

  const int charIndex = x / advance;
  const int localX = x % advance;
  if (localX >= charW) return false;  // inter-char gap

  const char* p = s;
  for (int i = 0; i < charIndex && *p; i++) ++p;
  if (*p == '\0') return false;

  const int col = localX / scale;
  const uint8_t bits = glyph(*p)[row];
  return (bits & (1u << (4 - col))) != 0;
}

void drawText(int x, int y, const char* s, int scale, uint16_t color565, FillRectFn fillRect) {
  if (!s || !fillRect || scale <= 0) return;

  const int advance = 5 * scale + scale;
  int cx = x;
  for (const char* p = s; *p; ++p, cx += advance) {
    const uint8_t* g = glyph(*p);
    for (int row = 0; row < 7; row++) {
      uint8_t bits = g[row];
      for (int col = 0; col < 5; col++) {
        if (bits & (1u << (4 - col))) {
          fillRect(cx + col * scale, y + row * scale, scale, scale, color565);
        }
      }
    }
  }
}

void drawCenteredText(int screenW, int y, const char* s, int scale, uint16_t color565, FillRectFn fillRect) {
  int w = textWidth(s, scale);
  int x = (screenW - w) / 2;
  drawText(x, y, s, scale, color565, fillRect);
}

}  // namespace Font5x7
