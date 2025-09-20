#include "renderBox2d.h"

namespace renderB2
{
	namespace DefaultColors {
		sf::Color ClearFill = sf::Color{ 1, 14, 22 };
		sf::Color BackGroundOutline = sf::Color{ 0, 98, 167 };
		sf::Color WarningOutline = sf::Color{ 136, 0, 27 };
		sf::Color WarningText = sf::Color{ 222, 33, 40 };
		sf::Color TextBox = sf::Color{ 2, 66, 110, 133 };
		sf::Color Button = sf::Color{ 30, 55, 58, 133 };
		sf::Color LightBlue = sf::Color{ 6, 156, 195, 133 };
		sf::Color B2BodyFill = sf::Color{ 255, 255, 255, 85 };
		sf::Color B2BodyFill2 = sf::Color{ 255, 255, 255, 30 };
	}
	sf::Font DefaultFont("Assets\\Fonts\\CangJiGaoDeGuoMiaoHei_CJgaodeguomh_2.ttf");
	sf::Texture Smoke("Assets\\Textures\\smoke.png");
	sf::Texture Soul("Assets\\Textures\\Soul.png");
	sf::Texture MetaBall("Assets\\Textures\\metaball256.png");
	sf::Texture Circle("Assets\\Textures\\circle.png");
	sf::Shader metaBallShader;
	sf::Texture blurTexture;
	sf::Shader blurShader;

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

	void init(sf::RenderWindow* window) {
		blurTexture.resize(window->getSize());
		if (!blurShader.loadFromFile("Assets\\Shaders\\blur.frag", sf::Shader::Type::Fragment)) {
			std::cerr << "Failed to load shaders" << std::endl;
		}
		if (!metaBallShader.loadFromFile("Assets\\Shaders\\metaball.frag", sf::Shader::Type::Fragment)) {
			std::cerr << "Failed to load shader" << std::endl;
		}
		metaBallShader.setUniform("iResolution", sf::Vector2f(window->getSize()));
		metaBallShader.setUniform("blurRadius", 3);

		blurShader.setUniform("iResolution", sf::Vector2f(window->getSize()));
		blurShader.setUniform("blurRadius", 5);
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
	void setRenderSettings(sf::Shape* shape, renderB2::RenderSettings rendersettings) {
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
		return;
	}
	/*
	void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::ConvexShape shape;
		shape.setPointCount(rendersettings.verticecount);
		for (int i = 0; i < rendersettings.verticecount; i++) {
			b2Vec2 localVertex = polygon.vertices[i];
			float screenX = localVertex.x + screensettings.width / 2.0f;
			float screenY = screensettings.height / 2.0f - localVertex.y;
			shape.setPoint(i, sf::Vector2f(screenY, screenX));
		}
		shape.setOrigin({ shape.getLocalBounds().getCenter().x, shape.getLocalBounds().getCenter().y });
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		float angle = atan2(transform.q.s, transform.q.c) * 180.0f / B2_PI;

		shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		shape.setRotation(sf::degrees(angle));
		setRenderSettings(&shape, rendersettings);

		blurTexture.update(*window);
		blurShader.setUniform("screenTexture", blurTexture);

		window->draw(shape, &blurShader);
		window->draw(shape);
		return;
	}
	*/
	void renderb2Polygon(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, b2Polygon polygon, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::ConvexShape shape;
		shape.setPointCount(rendersettings.verticecount);
		for (int i = 0; i < rendersettings.verticecount; i++) {
			b2Vec2 localVertex = polygon.vertices[i];
			float screenX = localVertex.x + screensettings.width / 2.0f;
			float screenY = screensettings.height / 2.0f - localVertex.y;
			shape.setPoint(i, sf::Vector2f(screenY, screenX));
		}
		shape.setOrigin({ shape.getLocalBounds().getCenter().x, shape.getLocalBounds().getCenter().y });
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
		float angle = atan2(transform.q.s, transform.q.c) * 180.0f / B2_PI;

		shape.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		shape.setRotation(sf::degrees(angle));
		setRenderSettings(&shape, rendersettings);
		window->draw(shape);
		return;
	}

	void renderSoul(sf::RenderWindow* window, b2Circle circle, b2BodyId bodyId, renderB2::ScreenSettings screensettings) {
		sf::Sprite sprite(Soul);
		sprite.setOrigin({ circle.radius, circle.radius });
		b2Transform transform = b2Body_GetTransform(bodyId);
		b2Vec2 pos = transform.p;
		float angle = atan2(transform.q.s, transform.q.c) * -180.0f / B2_PI;

		sprite.setPosition(MathUtils::getSFpos(pos.x, pos.y));
		window->draw(sprite);
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
	void rendersimpleparticle(sf::RenderWindow* window,
		renderB2::RenderSettings rendersettings,
		GameObjects::ParticleGroup& group,
		renderB2::ScreenSettings screensettings)
	{
		//sf::VertexArray point(sf::PrimitiveType::Points, group.Particles.size() * 9);
		sf::VertexArray point(sf::PrimitiveType::Triangles, group.Particles.size() * 3);
		int count = 0;
		const sf::Vector2f offset[3] = { { 3, 0 },   { -3, 3 },   { -3, -3 } };
		for (const auto& p : group.Particles)
		{
			const sf::Vector2f center = MathUtils::getSFpos(p.pos.x, p.pos.y);
			/*
			for (int i = -1; i <= 1; ++i) {
				for (int j = -1; j <= 1; ++j) {
					point[count].color = rendersettings.FillColor;
					point[count].position = { center.x + i, center.y + j };
					count++;
				}
			}
			*/
			for (int i = 0; i < 3; ++i) {
				point[count].color = p.color;
				point[count].position = center + offset[i];
				count++;
			}
		}
		window->draw(point);
		return;
	}
	void rendersandparticle(sf::RenderWindow* window,
		renderB2::RenderSettings rendersettings,
		GameObjects::ParticleGroup& group,
		renderB2::ScreenSettings screensettings)
	{
		//sf::VertexArray point(sf::PrimitiveType::Points, group.Particles.size() * 9);
		sf::VertexArray point(sf::PrimitiveType::Triangles, group.Particles.size() * 6);
		int count = 0;
		const sf::Vector2f offset[3] = { { 0, 0 },   { 1, 0 },   { 0, 1 } };
		const sf::Vector2f offset2[3] = { { 1, 1 },   { 1, 0 },   { 0, 1 } };
		for (const auto& p : group.Particles)
		{
			const sf::Vector2f center = MathUtils::getSFpos(p.pos.x, p.pos.y);
			/*
			for (int i = -1; i <= 1; ++i) {
				for (int j = -1; j <= 1; ++j) {
					point[count].color = rendersettings.FillColor;
					point[count].position = { center.x + i, center.y + j };
					count++;
				}
			}
			*/
			for (int i = 0; i < 3; ++i) {
				point[count].color = sf::Color::Yellow;
				point[count].position = center + sf::Vector2f{ (offset[i].x - 0.5f) * p.shape.radius * 5.25f, (offset[i].y - 0.5f) * p.shape.radius * 5.25f };
				point[count].texCoords = sf::Vector2f{ offset[i].x * 128, offset[i].y * 128 };
				count++;
			}
			for (int i = 0; i < 3; ++i) {
				point[count].color = sf::Color::Yellow;
				point[count].position = center + sf::Vector2f{ (offset2[i].x - 0.5f) * p.shape.radius * 5.25f, (offset2[i].y - 0.5f) * p.shape.radius * 5.25f };
				point[count].texCoords = sf::Vector2f{ offset2[i].x * 128, offset2[i].y * 128 };
				count++;
			}
		}
		sf::RenderStates states;
		states.texture = &Circle;
		window->draw(point, states);
		return;
	}
	void renderparticletest(sf::RenderWindow* window, renderB2::RenderSettings rendersettings, GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings) {
		//sf::CircleShape shape(1,8);

		for (auto& p : group.Particles) {
			sf::Vector2f basePos = MathUtils::getSFpos(p.pos.x, p.pos.y);
			sf::Vector2f velocity = MathUtils::getSFpos(p.LinearVelocity.x / 2, p.LinearVelocity.y / 2);
			//sf::Vector2f velocity = MathUtils::getSFpos(p.nextTickLinearImpulse.x / 10, p.nextTickLinearImpulse.y / 10);
		   //sf::Color fillColor = p.color;

		   //float drawRadius = p.shape.radius;
		   //shape.setRadius(drawRadius);
		   //shape.setOrigin({ drawRadius, drawRadius });
		   //shape.setPosition(basePos);
		   //window->draw(shape);
			sf::Vertex line[2] = {
				sf::Vertex{ basePos, sf::Color::Green },
				sf::Vertex{ sf::Vector2f(basePos.x + velocity.x, basePos.y + velocity.y), sf::Color::Green }
			};
			window->draw(line, 2, sf::PrimitiveType::LineStrip);
		}
		return;
	}
	void renderwater(sf::RenderWindow* window,
		renderB2::RenderSettings rendersettings,
		GameObjects::ParticleGroup& group,
		renderB2::ScreenSettings screensettings)
	{
		sf::CircleShape shape(0.f, 10);
		shape.setRotation(sf::degrees(90));
		shape.setOutlineThickness(0.f);
		for (auto& p : group.Particles) {
			shape.setFillColor(sf::Color{ 63, 72, 204 });
			float radius = (float)std::min(std::max(p.neighborCount, 4), 5) / 5.f * (p.shape.radius * group.Config.Impact + 3.f);
			if (p.neighborCount == 1) {
				radius = (p.shape.radius * group.Config.Impact + 3.f) * 0.5f;
			}
			shape.setRadius(radius);
			shape.setOrigin({ radius, radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		for (auto& p : group.Particles) {
				shape.setFillColor(p.color);
			float radius = (float)std::min(std::max(p.neighborCount, 4), 5) / 5.f * (p.shape.radius * group.Config.Impact / 1.5f);
			if (p.neighborCount == 1) {
				radius = (p.shape.radius * group.Config.Impact / 1.5f) * 0.5f;
			}
			shape.setRadius(radius);
			shape.setOrigin({ radius, radius });
			shape.setPosition(MathUtils::getSFpos(p.pos.x, p.pos.y));
			window->draw(shape);
		}
		return;
	}
	
	void renderwatershader(sf::RenderWindow* window, renderB2::RenderSettings rendersettings,
		GameObjects::ParticleGroup& group, renderB2::ScreenSettings screensettings)
	{
		sf::RenderTexture renderTexture(window->getSize());
		renderTexture.clear();

		sf::VertexArray vertices(sf::PrimitiveType::Triangles);
		vertices.resize(group.Particles.size() * 6);

		int idx = 0;
		for (auto& p : group.Particles) {
			float radius = p.shape.radius * 4.25f;
			sf::Vector2f pos = MathUtils::getSFpos(p.pos.x, p.pos.y) - window->mapPixelToCoords({ 0,0 });

			sf::Vector2f offsets[4] = {
				{-radius, -radius}, 
				{ radius, -radius}, 
				{ radius,  radius},
				{-radius,  radius} 
			};

			sf::Vector2f texCoords[4] = {
				{0.f,   0.f},  
				{256.f, 0.f},   
				{256.f, 256.f},  
				{0.f,   256.f}    
			};

			int tri[6] = { 0,1,2, 0,2,3 };
			for (int t = 0; t < 6; t++) {
				int v = tri[t];
				sf::Vertex& vertex = vertices[idx++];
				vertex.position = pos + offsets[v];
				vertex.texCoords = texCoords[v];
				vertex.color = sf::Color::White;//p.color;
			}
		}

		sf::RenderStates states;
		states.texture = &MetaBall;

		renderTexture.draw(vertices, states);
		renderTexture.display();

		sf::RectangleShape rectangle(sf::Vector2f(window->getSize()));
		rectangle.setPosition(window->mapPixelToCoords({ 0, 0 }));
		rectangle.setTexture(&renderTexture.getTexture());

		blurTexture.update(*window);
		metaBallShader.setUniform("screenTexture", blurTexture);

		//states.blendMode = sf::BlendAdd;
		states.shader = &metaBallShader;
		window->draw(rectangle, states);
	}


}