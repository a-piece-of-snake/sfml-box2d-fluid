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
			const auto& textureRef = rendersettings.Texture.value();
			shape->setTexture(&textureRef);
		}
		if (rendersettings.TextureRect.has_value()) {
			const auto& textureRectRef = rendersettings.TextureRect.value();
			shape->setTextureRect(textureRectRef);
		}
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
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		float angle = atan2(transform.q.s, transform.q.c) * -180.0f / B2_PI;

		shape.setPosition({ screensettings.height / 2.0f - pos.y + screensettings.camX, screensettings.width / 2.0f - pos.x + screensettings.camY});
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

		shape.setPosition({ screensettings.height / 2.0f - pos.y + screensettings.camX, screensettings.width / 2.0f - pos.x + screensettings.camY });
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
		shape.setPosition({ screensettings.height / 2.0f - pos.y + screensettings.camX, screensettings.width / 2.0f - pos.x + screensettings.camY });
		shape.setRotation(sf::degrees(90));
		setRenderSettings(&shape, rendersettings);
		window->draw(shape);
		return;
	}
}