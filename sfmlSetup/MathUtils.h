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
    float b2Vec2Length(const b2Vec2& v);

    b2Vec2 b2Vec2Normalized(const b2Vec2& v);
}