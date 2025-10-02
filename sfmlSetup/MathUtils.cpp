#include "MathUtils.h"

namespace MathUtils
{

    float cross(const b2Vec2& a, const b2Vec2& b, const b2Vec2& c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }

    bool pointInTriangle(const b2Vec2& a, const b2Vec2& b, const b2Vec2& c, const b2Vec2& p) {
        float cross1 = cross(a, b, p);
        float cross2 = cross(b, c, p);
        float cross3 = cross(c, a, p);

        return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0) ||
            (cross1 <= 0 && cross2 <= 0 && cross3 <= 0);
    }

    bool triangleContainsPoint(const std::vector<b2Vec2>& vertices, int a, int b, int c, const std::vector<int>& indices) {
        for (int index : indices) {
            if (index == a || index == b || index == c) continue;

            if (pointInTriangle(vertices[a], vertices[b], vertices[c], vertices[index])) {
                return true;
            }
        }
        return false;
    }

    bool isEar(const std::vector<b2Vec2>& vertices, int prev, int current, int next, const std::vector<int>& indices) {
        if (cross(vertices[prev], vertices[current], vertices[next]) < 0) {
            return false;
        }

        return !triangleContainsPoint(vertices, prev, current, next, indices);
    }

    std::vector<std::vector<int>> triangulate(const std::vector<b2Vec2>& vertices) {
        std::vector<std::vector<int>> triangles;
        if (vertices.size() < 3) return triangles;

        std::vector<int> indices;
        for (size_t i = 0; i < vertices.size(); ++i) {
            indices.push_back(static_cast<int>(i));
        }

        int n = static_cast<int>(indices.size());
        int current = 0;

        while (n > 3) {
            int prev = (current - 1 + n) % n;
            int next = (current + 1) % n;

            if (isEar(vertices, indices[prev], indices[current], indices[next], indices)) {
                std::vector<int> triangle = { indices[prev], indices[current], indices[next] };
                triangles.push_back(triangle);

                indices.erase(indices.begin() + current);
                n--;

                current = std::max(0, current - 1);
            }
            else {
                current = (current + 1) % n;
            }
        }

        if (n == 3) {
            std::vector<int> triangle = { indices[0], indices[1], indices[2] };
            triangles.push_back(triangle);
        }

        return triangles;
    }

    sf::Color encodeFloatToRGBA(float value) {
        union {
            float f;
            uint32_t i;
        } u;
        u.f = value;

        std::uint8_t r = (u.i >> 24) & 0xFF;
        std::uint8_t g = (u.i >> 16) & 0xFF;
        std::uint8_t b = (u.i >> 8) & 0xFF;
        //std::uint8_t a = u.i & 0xFF;

        //return sf::Color(r, g, b, a);
        return sf::Color(r, g, b, 255); 

    }

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

    float b2Length(b2Vec2 v)
    {
        return Q_sqrt(v.x * v.x + v.y * v.y);
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