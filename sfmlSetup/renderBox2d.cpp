#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "renderBox2d.h"
#include "GameObjects.h"
#include "MathUtils.h"
namespace renderB2
{
	namespace DefaultColors {
		sf::Color ClearFill = sf::Color{ 1, 14, 22 };
		sf::Color BackGroundOutline = sf::Color{ 0, 98, 167 };
		sf::Color WarningOutline = sf::Color{ 136, 0, 27 };
		sf::Color WarningText = sf::Color{ 222, 33, 40 };
		sf::Color TextBox = sf::Color{ 2, 66, 110, 133 };
		sf::Color B2BodyFill = sf::Color{ 255, 255, 255, 85 };
	}
	sf::Font DefaultFont("Assets\\Fonts\\NanoDyongSong.ttf");

	sf::Font getDefaultFont() {
		DefaultFont.setSmooth(false);
		return DefaultFont;
	}

	sf::Font& getDefaultFontAddress() {
		static sf::Font font;
		font.setSmooth(false);
		static bool loaded = false;
		if (!loaded) {
			if (!font.openFromFile("Assets\\Fonts\\NanoDyongSong.ttf")) {
				std::cerr << "Failed to load font" << std::endl;
			}
			loaded = true;
		}
		return font;
	}
	void renderTextInShape(sf::RenderWindow* window, sf::Shape& shape, sf::Text& text) {
		sf::FloatRect shapeBounds = shape.getGlobalBounds();
		sf::FloatRect textBounds = text.getLocalBounds();
		float textX = shapeBounds.getCenter().x - (textBounds.size.x) / 2.0f;
		float textY = shapeBounds.getCenter().y - (textBounds.size.y) / 2.0f;
		text.setPosition({ textX, textY });
		window->draw(shape);
		window->draw(text);
	}

	sf::ConvexShape getRectangleMinusCorners(float Width, float Length, float Subtract) {
		sf::ConvexShape convex;
		convex.setPointCount(8);
		convex.setPoint(0, { 0.f, Subtract });
		convex.setPoint(1, { 0.f, Length - Subtract });
		convex.setPoint(2, { Subtract, Length });
		convex.setPoint(3, { Width - Subtract, Length });
		convex.setPoint(4, { Width, Length - Subtract });
		convex.setPoint(5, { Width, Subtract });
		convex.setPoint(6, { Width - Subtract, 0.f });
		convex.setPoint(7, { Subtract, 0.f });
		return convex;
	}
	void setRenderSettings(sf::Shape* shape, renderB2::RenderSettings rendersettings){
		shape->setFillColor(rendersettings.FillColor);
		shape->setOutlineThickness(rendersettings.OutlineThickness);
		shape->setOutlineColor(rendersettings.OutlineColor);
		if (rendersettings.Texture.has_value()) {
			const auto textureRef = rendersettings.Texture.value();
			shape->setTexture(&textureRef);
		}
		if (rendersettings.TextureRect.has_value()) {
			const auto& textureRectRef = rendersettings.TextureRect.value();
			shape->setTextureRect(textureRectRef);
		}
		return;
	}
	void SFMLrenderShadow(sf::RenderWindow* window, sf::Shape* shape, float Thickness, int repeat, sf::Color Color) {
		float shapeoutlinethickness = shape->getOutlineThickness();
		sf::Color shapeoutlinecolor = shape->getOutlineColor();
		float thickness = Thickness / repeat;
		sf::Color color = Color;
		color.a /= repeat;
		sf::Shape* shadow = shape;
		shadow->setOutlineColor(color);
		for (int i = 1; i <= repeat; i++) {
			shadow->setOutlineThickness(thickness * i + shape->getOutlineThickness());
			window->draw(*shadow);
		}
		shape->setOutlineColor(shapeoutlinecolor);
		shape->setOutlineThickness(shapeoutlinethickness);
		return;
	}
	void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::ConvexShape shape;
		shape.setPointCount(rendersettings.verticecount);
		for (int i = 0; i < rendersettings.verticecount; i++) {
			b2Vec2 localVertex = polygon.vertices[i];
			float screenX = localVertex.x + screensettings.width / 2.0f;
			float screenY = screensettings.height / 2.0f - localVertex.y;
			shape.setPoint(i, sf::Vector2f(screenY, screenX));
		}
		/*
		sf::Vector2f minPoint(FLT_MAX, FLT_MAX), maxPoint(-FLT_MAX, -FLT_MAX);
		for (size_t i = 0; i < shape.getPointCount(); i++) {
			sf::Vector2f point = shape.getPoint(i);
			minPoint.x = std::min(minPoint.x, point.x);
			minPoint.y = std::min(minPoint.y, point.y);
			maxPoint.x = std::max(maxPoint.x, point.x);
			maxPoint.y = std::max(maxPoint.y, point.y);
		}
		sf::Vector2f center = (minPoint + maxPoint) / 2.f;
		shape.setOrigin(center);
		*/
		shape.setOrigin({ shape.getLocalBounds().getCenter().x, shape.getLocalBounds().getCenter().y});
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		float angle = atan2(transform.q.s, transform.q.c) * 180.0f / B2_PI;

		shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		shape.setRotation(sf::degrees(angle));
		setRenderSettings(&shape, rendersettings);
		window->draw(shape);
		return;
	}

	void renderb2circle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::CircleShape shape(circle.radius, rendersettings.verticecount);
		shape.setOrigin({ circle.radius, circle.radius });
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		float angle = atan2(transform.q.s, transform.q.c) * -180.0f / B2_PI;

		shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		shape.setRotation(sf::degrees(angle));
		setRenderSettings(&shape, rendersettings);
		window->draw(shape);
		return;
	}

	void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::CircleShape shape(circle.radius, rendersettings.verticecount);
		shape.setOrigin({ circle.radius, circle.radius });
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		shape.setRotation(sf::degrees(90));
		setRenderSettings(&shape, rendersettings);
		window->draw(shape);
		return;
	}
}