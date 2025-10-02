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
#include <SFML/Audio.hpp>

#include <SFML/OpenGL.hpp>

#include "GameObjects.h"

#include "imgui/imgui_internal.h"

namespace ImGui {
    //https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554
    inline void ToggleButton(const char* str_id, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float height = ImGui::GetFrameHeight();
        float width = height * 1.55f;
        float radius = height * 0.50f;

        ImGui::InvisibleButton(str_id, ImVec2(width, height));
        if (ImGui::IsItemClicked())
            *v = !*v;

        float t = *v ? 1.0f : 0.0f;

        ImGuiContext& g = *GImGui;
        float ANIM_SPEED = 0.08f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
        {
            float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
            t = *v ? (t_anim) : (1.0f - t_anim);
        }

        ImU32 col_bg;
        if (ImGui::IsItemHovered())
            col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
        else
            col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
        draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    }
}

namespace renderB2
{
    namespace DefaultColors {
        extern sf::Color ClearFill;
        extern sf::Color BackGroundOutline;
        extern sf::Color WarningOutline;
        extern sf::Color WarningText;
        extern sf::Color TextBox;
        extern sf::Color Button;
        extern sf::Color LightBlue;
        extern sf::Color B2BodyFill;
        extern sf::Color B2BodyFill2;
    }

    extern sf::Font DefaultFont;
    extern sf::Texture blurTexture;
    extern sf::Shader blurShader;
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

    void init(sf::RenderWindow* window);

    void setRenderSettings(sf::Shape* shape, renderB2::RenderSettings rendersettings);
    void SFMLrenderShadow(sf::RenderWindow* window, sf::Shape* shape, float Thickness, int repeat, sf::Color Color);
    void SFMLrenderShadow2(sf::RenderWindow* window, sf::Drawable* drawable, const sf::Transform& baseTransform, float thickness, int repeat, sf::Color shadowColor);
    void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void renderb2Polygons(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, std::vector<std::pair<b2BodyId, b2Polygon>> polygons, renderB2::ScreenSettings screensettings);
    void renderSoul(sf::RenderWindow* window, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void renderb2circle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings);
    void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);
    void rendersandparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);
    void rendergasparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);
    void renderparticletest(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);
    void renderwater(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);
    void renderwatershader(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings);

    class MyUIObject {
    public:
        MyUIObject* parent = nullptr;
        std::vector<MyUIObject*> children;

        sf::Vector2f localPosition;
        float localRotation;
        sf::Vector2f localScale;
        bool lineBreak = true;
        float lineSpacing = 10.f;

        sf::Drawable* shape = nullptr;

        MyUIObject(sf::Drawable* shp, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : localPosition(pos), localRotation(rotation), localScale(scale), shape(shp)
        {
        }

        void addChild(MyUIObject* child) {
            if (child) {
                child->parent = this;
                if (child->lineBreak) {
                    child->localPosition.y += nowLineSpacing;
                    nowLineSpacing += child->lineSpacing + child->getBound().size.y;
                }
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
        sf::FloatRect  getBound() const {
            sf::FloatRect bound;
            if (auto shape2 = dynamic_cast<sf::Shape*>(shape))
                bound = shape2->getLocalBounds();
            else if (auto shape2 = dynamic_cast<sf::Text*>(shape))
                bound = shape2->getLocalBounds();
            /*
			sf::FloatRect childBound;
            for (MyUIObject* child : children)
                if (child) {
					childBound = child->getBound();
					bound.size.x = std::max(bound.size.x, childBound.size.x);
					bound.size.y = std::max(bound.size.y, childBound.size.y);
                }
            */
            return bound;
        }

        virtual void draw(sf::RenderWindow* window, bool blur = false, bool root = true) {
            if (shape) {
                if (blur && !root) {
                    blurTexture.update(*window);
                    blurShader.setUniform("screenTexture", blurTexture);
                    sf::RenderStates states;
                    states.transform = getGlobalTransform();
                    states.shader = &blurShader;
                    window->draw(*shape, states);
                }
                window->draw(*shape, getGlobalTransform());
            }
            for (MyUIObject* child : children)
                if (child)
                    child->draw(window, blur, false);
        }

        void drawShadow(sf::RenderWindow* window, float thickness, int repeat = 0, sf::Color shadowColor = sf::Color(0, 0, 0, 128))
        {
            if (shape) {
                SFMLrenderShadow2(window, shape, getGlobalTransform(), thickness, repeat, shadowColor);
            }
        }
     private:
         float nowLineSpacing = 0.f;
    };
    class UICheckBox : public MyUIObject {
    public:
        std::function<void()> onHover;
        std::function<void()> onHoverRelease;
        std::function<void()> onClick;
        std::function<void()> onHold;
        std::function<void()> onRelease;
        bool wasClicked;
        bool wasHover;
		bool isChecked;
        sf::SoundBuffer ClickSoundBuffer;
        sf::SoundBuffer ReleaseSoundBuffer;
        sf::Sound ClickSound;
        sf::Sound ReleaseSound;

        UICheckBox(sf::Drawable* shp, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f), bool defaultValue = false)
            : MyUIObject(shp, pos, rotation, scale), wasClicked(false), wasHover(false), isChecked(defaultValue),
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
            if (!checkedTexture.loadFromFile("Assets\\Textures\\checkbox_checked.png")) {
                std::cerr << "Failed to load checkedTexture!" << std::endl;
            }
            if (!uncheckedTexture.loadFromFile("Assets\\Textures\\checkbox_unchecked.png")) {
                std::cerr << "Failed to load uncheckedTexture!" << std::endl;
            }
            checkedShape.setSize({ 1.f, 1.f });
            checkedShape.setTexture(&checkedTexture);

            uncheckedShape.setSize({ 1.f, 1.f });
            uncheckedShape.setTexture(&uncheckedTexture);

        }

        virtual sf::FloatRect getGlobalBounds() const {
            if (auto s = dynamic_cast<const sf::Shape*>(shape))
                return getGlobalTransform().transformRect(s->getLocalBounds());
            return sf::FloatRect();
        }

        virtual void draw(sf::RenderWindow* window, bool blur = false, bool root = true) override {
            if (shape) {
                if (blur && !root) {
                    blurTexture.update(*window);
                    blurShader.setUniform("screenTexture", blurTexture);
                    sf::RenderStates states;
                    states.transform = getGlobalTransform();
                    states.shader = &blurShader;
                    window->draw(*shape, states);
                }
                window->draw(*shape, getGlobalTransform());
            }

            checkedShape.setSize(getGlobalBounds().size);
            uncheckedShape.setSize(getGlobalBounds().size);
            sf::RectangleShape& box = isChecked ? checkedShape : uncheckedShape;
            sf::Transform boxtransform = getGlobalTransform();
            boxtransform.translate({ (getGlobalBounds().size.x - box.getGlobalBounds().size.x) / 2, (getGlobalBounds().size.y - box.getGlobalBounds().size.y) / 2 });
            window->draw(box, boxtransform);

            for (MyUIObject* child : children)
                if (child)
                    child->draw(window);
        }



        virtual void tick( sf::RenderWindow& window) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            sf::FloatRect bounds = getGlobalBounds();
            bool hover = bounds.contains(mouseWorldPos);

            if (hover) {
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();
                window.setMouseCursor(cursor);
                wasHover = true;
                if (onHover)
                    onHover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    if (!wasClicked) {
						isChecked = !isChecked;
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
            else if (wasHover) {
                wasHover = false;
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
                window.setMouseCursor(cursor);
                if (onHoverRelease) {
                    onHoverRelease();
                }
            }
		}
    private:
        sf::Texture checkedTexture;
        sf::Texture uncheckedTexture;
        sf::RectangleShape checkedShape;
        sf::RectangleShape uncheckedShape;
    };
    class UIButton : public MyUIObject {
    public:
        std::function<void()> onHover;
        std::function<void()> onHoverRelease;
        std::function<void()> onClick;
        std::function<void()> onHold;
        std::function<void()> onRelease;
        bool wasClicked;
        bool wasHover;
        sf::SoundBuffer ClickSoundBuffer;
        sf::SoundBuffer ReleaseSoundBuffer;
        sf::Sound ClickSound;
        sf::Sound ReleaseSound;

        UIButton(sf::Drawable* shp, const sf::Vector2f& pos = sf::Vector2f(0.f, 0.f),
            float rotation = 0.f, const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f))
            : MyUIObject(shp, pos, rotation, scale), wasClicked(false), wasHover(false),
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

        virtual void tick(sf::RenderWindow& window) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            sf::FloatRect bounds = getGlobalBounds();
            bool hover = bounds.contains(mouseWorldPos);

            if (hover) {
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();
                window.setMouseCursor(cursor);
                wasHover = true;
                if (onHover)
                    onHover();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    if (!wasClicked) {
                        if (onClick)
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
            else if (wasHover) {
                wasHover = false;
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
                window.setMouseCursor(cursor);
                if (onHoverRelease) {
                    onHoverRelease();
                }
            }
        }
    };
    class UISlider : public MyUIObject {
    public:
        std::function<void()> onHover;
        std::function<void()> onHoverRelease;
        std::function<void()> onClick;
        std::function<void()> onHold;
        std::function<void()> onRelease;
        std::function<void()> onValueChange;
        bool wasClicked;
        bool wasHover;
        bool dragging;
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
            : MyUIObject(shp, pos, rotation, scale), button(butto), Max(max), Min(min), Value(value), LastTickValue(value), wasClicked(false), wasHover(false), dragging(false),
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
            if (auto s = dynamic_cast<const sf::Shape*>(button)) {
                sf::Transform transform = getGlobalTransform();
                sf::FloatRect bounds = getGlobalBounds();
                sf::FloatRect bbounds = transform.transformRect(s->getLocalBounds());
                float normalizedValue = (Value - Min) / (Max - Min);
                float buttonX = normalizedValue * bounds.size.x - bbounds.size.x / 2;
                float buttonY = 0;
                transform.translate({ buttonX, buttonY });
                return transform.transformRect(s->getLocalBounds());
            }
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
            sf::FloatRect bbounds = getButtonGlobalBounds();
            bool hover = bbounds.contains(mouseWorldPos) || bounds.contains(mouseWorldPos);
            bool leftPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

            if (hover && !wasHover) {
                wasHover = true;
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand).value();
                window.setMouseCursor(cursor);
                if (onHover) onHover();
            }

            if (!hover && wasHover && !dragging) {
                wasHover = false;
                const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
                window.setMouseCursor(cursor);
                if (onHoverRelease) onHoverRelease();
            }

            if (leftPressed) {
                if (!dragging && hover) {
                    dragging = true;
                    if (onClick) onClick();
                }

                if (dragging) {
                    Value = std::clamp((mouseWorldPos.x - bounds.position.x) / bounds.size.x * (Max - Min) + Min, Min, Max);
                    if (onHold) onHold();
                    if (onValueChange && LastTickValue != Value) {
                        onValueChange();
                    }
                }

                wasClicked = true;
            }
            else {
                if (dragging) {
                    dragging = false;
                    if (onRelease) onRelease();
                }
                wasClicked = false;
            }

            LastTickValue = Value;
        }

    };

}