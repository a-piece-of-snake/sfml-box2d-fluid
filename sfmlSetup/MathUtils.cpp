#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <optional> // Assuming std::optional is used

namespace MathUtils
{
    float b2Vec2Length(const b2Vec2& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    b2Vec2 b2Vec2Normalized(const b2Vec2& v) {
        float len = b2Vec2Length(v);
        return (len > 0.0001f) ? b2Vec2{ v.x / len, v.y / len } : b2Vec2_zero;
    }
}