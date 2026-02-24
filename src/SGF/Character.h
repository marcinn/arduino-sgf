#pragma once

#include "SGF/Vector2.h"

class Character {
public:
  using Position = Vector2;

  virtual ~Character() = default;

  Vector2 getPosition() const {
    return Position{posX, posY};
  }

  void setPosition(const Position& pos) {
    setPosition(pos.x, pos.y);
  }

  void setX(int newX) {
    setPosition(newX, posY);
  }

  void setY(int newY) {
    setPosition(posX, newY);
  }

  void setPosition(int newX, int newY) {
    posX = newX;
    posY = newY;
    didSetPosition();
  }

protected:
  int positionX() const {
    return posX;
  }

  int positionY() const {
    return posY;
  }

private:
  virtual void didSetPosition() {}

  int posX = 0;
  int posY = 0;
};
