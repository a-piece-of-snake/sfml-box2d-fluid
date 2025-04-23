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
	sf::Font DefaultFont("Assets\\Fonts\\CangJiGaoDeGuoMiaoHei_CJgaodeguomh_2.ttf");
	sf::Texture Smoke("Assets\\Textures\\smoke.png");

	sf::Font getDefaultFont() {
		DefaultFont.setSmooth(false);
		return DefaultFont;
	}

	sf::Font& getDefaultFontAddress() {
		static sf::Font font;
		font.setSmooth(false);
		static bool loaded = false;
		if (!loaded) {
			if (!font.openFromFile("Assets\\Fonts\\CangJiGaoDeGuoMiaoHei_CJgaodeguomh_2.ttf")) {
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
		if (repeat <= 0)
		{
			repeat = static_cast<int>(Thickness);
			if (repeat < 1)
				repeat = 1;
		}
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
	void SFMLrenderShadow2(sf::RenderWindow* window, sf::Drawable* drawable, const sf::Transform& baseTransform, float Thickness, int repeat, sf::Color shadowColor) {
		if (!drawable)
			return;
		if (repeat <= 0)
		{
			repeat = static_cast<int>(Thickness);
			if (repeat < 1)
				repeat = 1;
		}

		if (auto shape = dynamic_cast<sf::Shape*>(drawable))
		{
			float shapeoutlinethickness = shape->getOutlineThickness();
			sf::Color shapeoutlinecolor = shape->getOutlineColor();
			sf::Color shapefillcolor = shape->getFillColor();
			float thickness = Thickness / repeat;
			sf::Color color = shadowColor;
			color.a /= repeat;
			sf::Shape* shadow = shape;
			shadow->setOutlineColor(color);
			shadow->setFillColor(sf::Color::Transparent);
			for (int i = 1; i <= repeat; i++) {
				shadow->setOutlineThickness(thickness * i + shape->getOutlineThickness());
				window->draw(*shadow, baseTransform);
			}
			shape->setFillColor(shapefillcolor);
			shape->setOutlineColor(shapeoutlinecolor);
			shape->setOutlineThickness(shapeoutlinethickness);
		}
		else if (auto text = dynamic_cast<sf::Text*>(drawable)) {
			float textoutlinethickness = text->getOutlineThickness();
			sf::Color textoutlinecolor = text->getOutlineColor();
			sf::Color textfillcolor = text->getFillColor();
			float thickness = Thickness / repeat;
			sf::Color color = shadowColor;
			color.a /= repeat;
			sf::Text* shadow = text;
			shadow->setOutlineColor(color);
			shadow->setFillColor(sf::Color::Transparent);
			for (int i = 1; i <= repeat; i++) {
				shadow->setOutlineThickness(thickness * i + text->getOutlineThickness());
				window->draw(*shadow, baseTransform);
			}
			text->setFillColor(textfillcolor);
			text->setOutlineColor(textoutlinecolor);
			text->setOutlineThickness(textoutlinethickness);
		}
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
	/*
	//smoke
	void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings) {
		sf::CircleShape shape(0.f, 10);
		shape.setTexture(&Smoke);
		for (auto& p : group.Particles) {
			float radius = p.shape.radius * (group.Config.Impact + 2.f);
			shape.setRadius(radius);
			shape.setOrigin({ radius, radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		return;
	}
	*/
	void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings) {
		sf::CircleShape shape(0.f, 3);
		shape.setRotation(sf::degrees(90));
		setRenderSettings(&shape, rendersettings);
		for (auto& p : group.Particles) {
			shape.setRadius(p.shape.radius);
			shape.setOrigin({ p.shape.radius, p.shape.radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		return;
	}
	/*
	  // water
	void rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup group, renderB2::ScreenSettings screensettings) {
		sf::CircleShape shape(0.f, 10);
		shape.setRotation(sf::degrees(90));
		setRenderSettings(&shape, rendersettings);
		shape.setOutlineThickness(0.f);
		for (auto& p : group.Particles) {
			if (MathUtils::b2Vec2Length(p.LinearVelocity) > 20.f) {
				shape.setFillColor(sf::Color{ 255, 255, 255, 127 });
			}
			else {
				shape.setFillColor(sf::Color{ 63, 72, 204 });
			}
			float radius = (float)std::min(std::max(p.neighborCount, 4), 5) / 5.f * (p.shape.radius * group.Config.Impact + 4.f);
			if (p.neighborCount == 1) {
				radius = (p.shape.radius * group.Config.Impact + 4.f) * 0.5f;
			}
			shape.setRadius(radius);
			shape.setOrigin({ radius, radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		for (auto& p : group.Particles) {
			if (MathUtils::b2Vec2Length(p.LinearVelocity) > 20.f) {
				shape.setFillColor(sf::Color{ 255, 255, 255, 127 });
			}
			else {
				shape.setFillColor(sf::Color{ 0, 168, 243 });
			}
			float radius = (float)std::min(std::max(p.neighborCount, 4), 5) / 5.f * (p.shape.radius * group.Config.Impact + 1.f);
			if (p.neighborCount == 1) {
				radius = (p.shape.radius * group.Config.Impact + 1.f) * 0.5f;
			}
			shape.setRadius(radius);
			shape.setOrigin({ radius, radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		return;
	}*/
	/*
	void renderB2::rendersimpleparticle(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings) {
		const float neighborFactor = 2.0f;
		const float attractAlpha = 0.f;

		sf::CircleShape shape;
		// 预先设置基本渲染属性
		setRenderSettings(&shape, rendersettings);
		shape.setRotation(sf::degrees(90));

		// 遍历每个粒子
		for (auto& p : group.Particles) {
			// 获取粒子原始屏幕位置
			sf::Vector2f basePos = MathUtils::getSFpos(p.pos.x, p.pos.y);
			sf::Vector2f attractPos = basePos;
			int neighborCount = 0;
			sf::Vector2f neighborSum(0.f, 0.f);
			float influenceRadius = p.shape.radius * group.Config.Impact * neighborFactor;
			for (auto& other : group.Particles) {
				if (&other == &p)
					continue;
				sf::Vector2f otherPos = MathUtils::getSFpos(other.pos.x, other.pos.y);
				float dx = otherPos.x - basePos.x;
				float dy = otherPos.y - basePos.y;
				float dist = std::sqrt(dx * dx + dy * dy);
				if (dist < influenceRadius) {
					neighborSum += otherPos;
					neighborCount++;
				}
			}
			if (neighborCount > 0) {
				sf::Vector2f avgPos = neighborSum / static_cast<float>(neighborCount);
				attractPos = basePos + (attractAlpha * (avgPos - basePos));
			}
			sf::Color fillColor = rendersettings.FillColor;
			if (neighborCount >= 3) {
				fillColor = sf::Color(255, 100, 100, 200);
			}
			else if (neighborCount > 0) {
				fillColor = sf::Color(100, 200, 255, 200);
			}
			shape.setFillColor(fillColor);

			float drawRadius = p.shape.radius * 1.25f;
			shape.setRadius(drawRadius);
			shape.setOrigin({ drawRadius, drawRadius });
			shape.setPosition(attractPos);
			window->draw(shape);
			if (neighborCount > 0) {
				sf::Vertex line[] = {
					sf::Vertex(basePos, sf::Color::White),
					sf::Vertex(attractPos, sf::Color::White)
				};
				window->draw(line, 2, sf::PrimitiveType::LineStrip);
			}
		}*/
}