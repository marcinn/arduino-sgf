#pragma once

#include "Vector2.h"

class CharacterBody {
   public:
    using Position = Vector2i;

    virtual ~CharacterBody();

    Vector2i getPosition() const;
    void setPosition(const Position& pos);
    void setPosition(int newX, int newY);

   private:

    int posX = 0;
    int posY = 0;
};
