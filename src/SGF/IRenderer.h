#pragma once

class IRenderer {
   public:
    virtual ~IRenderer() = default;
    virtual void render() = 0;
};
