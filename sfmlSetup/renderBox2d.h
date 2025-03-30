#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "GameObjects.h"
#include <optional> 

namespace renderB2
{
    namespace DefaultColors {
        extern sf::Color ClearFill;
        extern sf::Color BackGroundOutline;
        extern sf::Color WarningOutline;
        extern sf::Color WarningText;
        extern sf::Color TextBox;
        extern sf::Color B2BodyFill;
    }
    extern sf::Font DefaultFont;
    sf::Font getDefaultFont();
    sf::Font& getDefaultFontAddress();
    void renderTextInShape(sf::RenderWindow* window, sf::Shape& shape, sf::Text& text);
    sf::ConvexShape getRectangleMinusCorners(float Width, float Length, float Subtract);
    struct RenderSettings
    {
        std::optional<sf::Texture> Texture;
        std::optional<sf::IntRect> TextureRect;
        int verticecount;
        int OutlineThickness;
        sf::Color OutlineColor;
        sf::Color FillColor;
    };
    struct ScreenSettings
    {
        int width;
        int height;
        int camX;
        int camY;
    };
    void setRenderSettings(sf::Shape* shape, renderB2::RenderSettings rendersettings);
    void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void renderb2circle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
}