#pragma once

#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <optional> 

#include <box2d/box2d.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

namespace MathUtils
{
    sf::Color encodeFloatToRGBA(float value);

    float Q_sqrt(float x);

    float b2Length(b2Vec2 v);

    sf::Vector2f getSFpos(float x, float y);

    sf::Vector2f getViewOffset(const sf::View& view);

    b2Vec2 toB2Position(float screenX, float screenY, sf::View view);
}