
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <random>
#include <iomanip>
#include <functional>
#include <thread>
#include <vector>
#include <numeric>
#include <sstream> 

int width = 2400;
int height = 1350;
int particleCount = 0;
sf::RenderWindow window(sf::VideoMode({ 2400, 1350 }), "AWA", sf::Style::Default);
sf::Vector2f mousePos;
sf::Vector2f lastmousePos;
/*
box2d

↑x
|
|
•——→—————————|
|     y                                    |
|                                           |
|                                           |
|                                           |
|————————————|

sfml
      x
•——→—————————|
|                                           |
|                                           |
↓y                                      |
|                                           |
|————————————|
*/

int main() {
    window.setFramerateLimit(120);
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

    }

    return 0;
}
