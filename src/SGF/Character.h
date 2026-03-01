#pragma once

#include "SGF/Vector2.h"

class Character {
public:
  using Position = Vector2;

  virtual ~Character();

  Vector2 getPosition() const;
  void setPosition(const Position& pos);
  void setX(int newX);
  void setY(int newY);
  void setPosition(int newX, int newY);

protected:
  int positionX() const;
  int positionY() const;

private:
  virtual void didSetPosition();

  int posX = 0;
  int posY = 0;
};
