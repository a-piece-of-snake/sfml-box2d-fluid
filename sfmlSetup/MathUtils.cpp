#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

namespace MathUtils
{
    float Q_rsqrt(float number) {//Quake III Arena
        long i;
        float x2, y;
        const float threehalfs = 1.5F;

        x2 = number * 0.5F;
        y = number;
        i = *(long*)&y;             // evil floating point bit level hacking
        i = 0x5f3759df - (i >> 1);  // what the f***?
        y = *(float*)&i;
        y = y * (threehalfs - (x2 * y * y));
        y = y * (threehalfs - (x2 * y * y)); 
        return y;
    }

    float Q_sqrt(float x) {
        return x * Q_rsqrt(x);
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