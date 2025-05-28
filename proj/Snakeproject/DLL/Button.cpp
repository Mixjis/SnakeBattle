#include "Button.h"

// Konstruktor
Button::Button(float x, float y, float width, float height,
    const sf::Font& font, const sf::String& text,
    const sf::Color& idleColor,
    const sf::Color& hoverColor,
    const sf::Color& activeColor)
    : idleColor(idleColor), hoverColor(hoverColor), activeColor(activeColor),
    isHovered(false), isPressed(false)
{
    // Konfiguracja kształtu przycisku
    this->shape.setPosition(sf::Vector2f(x, y));
    this->shape.setSize(sf::Vector2f(width, height));
    this->shape.setFillColor(idleColor);

    // Konfiguracja tekstu przycisku
    this->font = font;
    this->text.setFont(this->font);
    this->text.setString(text);
    this->text.setFillColor(sf::Color::White);
    this->text.setCharacterSize(30);

    // Wyśrodkowanie tekstu na przycisku
    this->text.setPosition(
        this->shape.getPosition().x + (this->shape.getGlobalBounds().width / 2.f) - this->text.getGlobalBounds().width / 2.f,
        this->shape.getPosition().y + (this->shape.getGlobalBounds().height / 2.f) - this->text.getGlobalBounds().height / 2.f - 15.f
    );
}

// Ustawienie funkcji callback dla kliknięcia
void Button::setOnClick(std::function<void()> callback) {
    this->onClick = callback;
};

// Aktualizacja stanu przycisku (sprawdzanie czy mysz nad przyciskiem i czy kliknięty)
void Button::update(const sf::Vector2f& mousePosition) {
    // Stan domyślny
    isHovered = false;
    isPressed = false;

    // Sprawdzenie czy mysz nad przyciskiem
    if (this->shape.getGlobalBounds().contains(mousePosition)) {
        isHovered = true;

        // Sprawdzenie czy przycisk wciśnięty
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            isPressed = true;
        }
    }

    // Aktualizacja koloru przycisku w zależności od stanu
    if (isPressed) {
        this->shape.setFillColor(activeColor);
    }
    else if (isHovered) {
        this->shape.setFillColor(hoverColor);
    }
    else {
        this->shape.setFillColor(idleColor);
    }
};

// Sprawdzenie czy przycisk został kliknięty
bool Button::isClicked(const sf::Vector2f& mousePosition) {
    if (this->shape.getGlobalBounds().contains(mousePosition)) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            // Wywołanie funkcji callback jeśli została ustawiona
            if (onClick) {
                onClick();
            }
            return true;
        }
    }
    return false;
};

// Funkcja zwracająca, czy kliknięcie zostało zarejestrowane i rozpoczęto akcję
bool Button::checkClick(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y)
            );

            if (this->shape.getGlobalBounds().contains(mousePos) && isHovered) {
                // Wywołanie funkcji callback jeśli została ustawiona
                if (onClick) {
                    onClick();
                }
                return true;
            }
        }
    }

    return false;
};

// Rysowanie przycisku
void Button::render(sf::RenderTarget& target) {
    target.draw(this->shape);
    target.draw(this->text);
};

// Zmiana tekstu przycisku
void Button::setText(const std::string& newText) {
    this->text.setString(newText);

    // Aktualizacja pozycji tekstu (wyśrodkowanie)
    this->text.setPosition(
        this->shape.getPosition().x + (this->shape.getGlobalBounds().width / 2.f) - this->text.getGlobalBounds().width / 2.f,
        this->shape.getPosition().y + (this->shape.getGlobalBounds().height / 2.f) - this->text.getGlobalBounds().height / 2.f - 5.f
    );
};

// Gettery do sprawdzania stanu
bool Button::getIsHovered() const {
    return this->isHovered;
};

bool Button::getIsPressed() const {
    return this->isPressed;
};