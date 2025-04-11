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
    sf::Vector2f getSFpos(float x, float y) {
        return sf::Vector2f{ y, -x };
    }

    sf::Vector2f getViewOffset(const sf::View& view) {
        sf::Vector2f center = view.getCenter();
        sf::Vector2f size = view.getSize();
        sf::Vector2f offset = center - size / 2.0f;
        return offset;
    }

    b2Vec2 toB2Position(float screenX, float screenY, sf::View view) {
        //view.rotate(sf::degrees(-90));
        //sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(screenX, screenY), view);
        sf::Vector2f worldPos = {
           -(screenY + getViewOffset(view).y),
           screenX + getViewOffset(view).x
        };
        return b2Vec2{ worldPos.x, worldPos.y };
    }

}