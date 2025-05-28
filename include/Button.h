#pragma once
#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional>
#include <string>
#include "API.h"

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Font font;
    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color activeColor;

    bool isHovered;
    bool isPressed;

    std::function<void()> onClick;

public:
    // Konstruktor
    Button(float x, float y, float width, float height,
        const sf::Font& font, const sf::String& text,
        const sf::Color& idleColor = sf::Color(100, 100, 100),
        const sf::Color& hoverColor = sf::Color(150, 150, 150),
        const sf::Color& activeColor = sf::Color(50, 50, 50));

    // Ustawienie funkcji callback dla kliknięcia
    void setOnClick(std::function<void()> callback);

    // Aktualizacja stanu przycisku (sprawdzanie czy mysz nad przyciskiem i czy kliknięty)
    void update(const sf::Vector2f& mousePosition);

    // Sprawdzenie czy przycisk został kliknięty
    bool isClicked(const sf::Vector2f& mousePosition);

    // Funkcja zwracająca, czy kliknięcie zostało zarejestrowane i rozpoczęto akcję
    bool checkClick(const sf::Event& event, const sf::RenderWindow& window);

    // Rysowanie przycisku
    void render(sf::RenderTarget& target);

    // Zmiana tekstu przycisku
    void setText(const std::string& newText);

    // Gettery do sprawdzania stanu
    bool getIsHovered() const;
    bool getIsPressed() const;
};

#endif // BUTTON_H