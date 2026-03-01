#pragma once

class DigitalAction {
   public:
    void update(bool isPressedNow);
    void reset(bool isPressedNow = false);

    bool pressed() const;
    bool justPressed() const;
    bool justReleased() const;

   private:
    bool pressedFlag = false;
    bool justPressedFlag = false;
    bool justReleasedFlag = false;
};
