#ifndef KLASA_H
#define KLASA_H

#include <SFML/Graphics.hpp>
#include "API.h"
class EXPORT_API SFMLWindow {
public:
    SFMLWindow(int width, int height, const std::string& title);
    void run();

private:
    sf::RenderWindow window;
    sf::RectangleShape rect;
};

#endif // TESTCLASS_H
