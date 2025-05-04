#pragma once
#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
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
    void SFMLrenderShadow(sf::RenderWindow* window, sf::Shape* shape, float Thickness, int repeat, sf::Color Color);
    void SFMLrenderShadow2(sf::RenderWindow* window, sf::Drawable* drawable, const sf::Transform& baseTransform, float thickness, int repeat, sf::Color shadowColor);
    void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void renderSoul(sf::RenderWindow* window, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void renderb2circle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);

    class MyUIObject {
    public:
        MyUIObject* parent = nullptr;
        std::vector<MyUIObject*> children;

        sf::Vector2f localPosition;
        float localRotation;
        sf::Vector2f localScale;

        sf::Drawable* shape = nullptr;

        MyUIObject(sf::Drawable* shp, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : localPosition(pos), localRotation(rotation), localScale(scale), shape(shp)
        {
        }

        void addChild(MyUIObject* child) {
            if (child) {
                child->parent = this;
                children.push_back(child);
            }
        }

        sf::Transform getGlobalTransform() const {
            sf::Transform transform;
            if (parent)
                transform = parent->getGlobalTransform();
            transform.translate(localPosition);
            transform.rotate(sf::degrees(localRotation));
            transform.scale(localScale);
            return transform;
        }

        virtual void draw(sf::RenderWindow* window) {
            if (shape)
                window->draw(*shape, getGlobalTransform());
            for (MyUIObject* child : children)
                if (child)
                    child->draw(window);
        }

        void drawShadow(sf::RenderWindow* window, float thickness, int repeat = 0, sf::Color shadowColor = sf::Color(0, 0, 0, 128))
        {
            if (shape) {
                SFMLrenderShadow2(window, shape, getGlobalTransform(), thickness, repeat, shadowColor);
            }
        }
    };
    class UIButton : public MyUIObject {
    public:
        std::function<void()> onHover;
        std::function<void()> onClick;
        std::function<void()> onHold;
        std::function<void()> onRelease;
        bool wasClicked;
        sf::SoundBuffer ClickSoundBuffer;
        sf::SoundBuffer ReleaseSoundBuffer;
        sf::Sound ClickSound;
        sf::Sound ReleaseSound;

        UIButton(sf::Drawable* shp, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : MyUIObject(shp, pos, rotation, scale), wasClicked(false),
            ClickSound(ClickSoundBuffer), ReleaseSound(ReleaseSoundBuffer)
        {
            if (!ClickSoundBuffer.loadFromFile("Assets\\Sounds\\mouseclick.ogg")) {
                std::cerr << "Failed to load ClickSoundBuffer!" << std::endl;
            }
            if (!ReleaseSoundBuffer.loadFromFile("Assets\\Sounds\\mouserelease.ogg")) {
                std::cerr << "Failed to load ReleaseSoundBuffer!" << std::endl;
            }

            ClickSound.setBuffer(ClickSoundBuffer);
            ReleaseSound.setBuffer(ReleaseSoundBuffer);
        }

        virtual sf::FloatRect getGlobalBounds() const {
            if (auto s = dynamic_cast<const sf::Shape*>(shape))
                return getGlobalTransform().transformRect(s->getLocalBounds());
            return sf::FloatRect();
        }

        virtual void tick( sf::RenderWindow& window) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            sf::FloatRect bounds = getGlobalBounds();
            bool hover = bounds.contains(mouseWorldPos);

            if (hover) {
                if (onHover)
                    onHover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    if (!wasClicked) {
                        if(onClick)
							onClick();
                    }
                    else {
                        if (onHold)
                            onHold();
                    }
                    wasClicked = true;
                }
                else {
                    if (wasClicked) {
                        if (onRelease) {
                            onRelease();
                        }
                    }
                    wasClicked = false;
                }
            }
		}
    };
    class UISlider : public MyUIObject {
    public:
        std::function<void()> onHover;
        std::function<void()> onClick;
        std::function<void()> onHold;
        std::function<void()> onRelease;
        std::function<void()> onValueChange;
        bool wasClicked;
        sf::SoundBuffer ClickSoundBuffer;
        sf::SoundBuffer ReleaseSoundBuffer;
        sf::Sound ClickSound;
        sf::Sound ReleaseSound;
        float Max;
        float Min;
		float Value;
		float LastTickValue;
        sf::Drawable* button = nullptr; 
        UISlider(sf::Drawable* shp, sf::Drawable* butto, float min, float max, float value, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : MyUIObject(shp, pos, rotation, scale), button(butto), Max(max), Min(min), Value(value), LastTickValue(value), wasClicked(false),
            ClickSound(ClickSoundBuffer), ReleaseSound(ReleaseSoundBuffer)
        {
            if (!ClickSoundBuffer.loadFromFile("Assets\\Sounds\\mouseclick.ogg")) {
                std::cerr << "Failed to load ClickSoundBuffer!" << std::endl;
            }
            if (!ReleaseSoundBuffer.loadFromFile("Assets\\Sounds\\mouserelease.ogg")) {
                std::cerr << "Failed to load ReleaseSoundBuffer!" << std::endl;
            }

            ClickSound.setBuffer(ClickSoundBuffer);
            ReleaseSound.setBuffer(ReleaseSoundBuffer);
        }

        virtual sf::FloatRect getGlobalBounds() const {
            if (auto s = dynamic_cast<const sf::Shape*>(shape))
                return getGlobalTransform().transformRect(s->getLocalBounds());
            return sf::FloatRect();
        }
        virtual sf::FloatRect getButtonGlobalBounds() const {
            if (auto s = dynamic_cast<const sf::Shape*>(button))
                return getGlobalTransform().transformRect(s->getLocalBounds());
            return sf::FloatRect();
        }
        virtual void draw(sf::RenderWindow* window) {
            if (shape)
                window->draw(*shape, getGlobalTransform());
            if (button) {
                sf::FloatRect bounds = getGlobalBounds();
                sf::FloatRect Bbounds = getButtonGlobalBounds();
                float normalizedValue = (Value - Min) / (Max - Min);
                float buttonX = normalizedValue * bounds.size.x - Bbounds.size.x / 2;
                float buttonY = 0;

                sf::Transform transform = getGlobalTransform();
                transform.translate({ buttonX, buttonY });

                window->draw(*button, transform);
            }
            for (MyUIObject* child : children)
                if (child)
                    child->draw(window);
        }

        virtual void tick(sf::RenderWindow& window) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            sf::FloatRect bounds = getGlobalBounds();
            bool hover = bounds.contains(mouseWorldPos);

            if (hover) {
                if (onHover)
                    onHover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
					Value = std::clamp((mouseWorldPos.x - bounds.position.x) / bounds.size.x * (Max - Min) + Min, Min, Max);
                    if (!wasClicked) {
                        if (onClick)
                            onClick();
                    }
                    else {
                        if (onHold)
                            onHold();
                    }
                    wasClicked = true;
                    if (onValueChange && LastTickValue != Value) {
                        onValueChange();
                    }
                }
                else {
                    if (wasClicked) {
                        if (onRelease) {
                            onRelease();
                        }
                    }
                    wasClicked = false;
                }
            }
			LastTickValue = Value;
        }
    };

}