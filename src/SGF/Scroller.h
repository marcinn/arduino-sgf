#pragma once

#include <stdint.h>
#include <functional>

#include "FastILI9341.h"

// Prosty helper do hardware'owego vertical scrolla ILI9341.
// Zarządza offsetem, wywołuje VSCRDEF/VSCRSADD i prosi użytkownika o
// dorysowanie odkrytego paska (renderStrip).
class HardwareScroller {
public:
  using RenderStripFn = std::function<void(int32_t worldY, int h, uint16_t* buf)>;
  using BlitStripFn   = std::function<void(int physY, int h, uint16_t* buf)>;

  explicit HardwareScroller(FastILI9341& gfx) : gfx(gfx) {}

  // topFixed + scrollHeight + bottomFixed musi równać się wysokości ekranu w bieżącej orientacji
  void configure(uint16_t topFixed, uint16_t scrollHeight, uint16_t bottomFixed);

  // Ustawia offset pamięci (0..scrollHeight-1) i koryguje VSCRSADD.
  void resetOffset(uint16_t yOff = 0);

  // Scroll o delta pikseli (delta>0 przesuwa obraz w górę, nowy pasek pojawia się na dole).
  // buf musi mieć co najmniej width()*maxStripLines elementów.
  // maxStripLines: ile linii mieści bufor; większe przesunięcia zostaną pocięte na kawałki.
  // Jeśli blitStrip nie jest podany, zostanie użyty domyślny blit na pełną szerokość.
  void scroll(int delta,
              uint16_t* buf,
              int maxStripLines,
              const RenderStripFn& renderStrip,
              const BlitStripFn& blitStrip = {});

  uint16_t offset() const { return offset_; }
  int32_t worldTop() const { return worldTop_; }
  uint16_t topFixed() const { return topFixed_; }
  uint16_t scrollHeight() const { return scrollH_; }

private:
  FastILI9341& gfx;
  uint16_t topFixed_ = 0;
  uint16_t scrollH_ = 0;
  uint16_t bottomFixed_ = 0;
  uint16_t offset_ = 0;     // aktualny VSCRSADD (mod scrollH_)
  int32_t worldTop_ = 0;    // logiczny Y linii widocznej na szczycie strefy scrolla
};
