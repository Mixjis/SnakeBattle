#pragma once

#ifndef GAEBASE_H
#define GAMEBASE_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include "GameCommon.h"
#include "API.h"

class EXPORT_API GameBase {
protected:
    sf::RenderWindow& window;
    sf::Font font;
    GameState gameState;
    sf::Clock clock;
    sf::Int32 lastUpdateTime;
    sf::Text scoreText;
    sf::Text statusText;
	sf::Music gameMusic;

    sf::Sprite bgSprite;
	sf::Texture backgroundImage;

    sf::String snakeNickname;
    sf::Color snakeColor;

    // Funkcja do rysowania węża
    void drawSnake(const Snake& snake);

    // rysowanie jedzenia
    void drawFood(const FoodItem& food);

    // rysowanie HUD
    void drawHUD(int localPlayerId);

    // Inicjalizacja fontów i tekstu
    void initText();

public:
    // Konstruktory
    GameBase();
    GameBase(sf::RenderWindow& existingWindow);

    virtual ~GameBase() {}

    virtual void run(sf::String nickname, sf::Color color) = 0;
};

#endif // GAEBASE_H